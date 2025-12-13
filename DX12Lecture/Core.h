#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <vector>
#include "DescriptorHeap.h"
#define NOMINMAX
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3dcompiler.lib")

class Barrier {
public:
	static void add(ID3D12Resource* res, D3D12_RESOURCE_STATES first, D3D12_RESOURCE_STATES second,
		ID3D12GraphicsCommandList4* commandList) {
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Transition.pResource = res;
		rb.Transition.StateBefore = first;
		rb.Transition.StateAfter = second;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &rb);
	}
};

class GPUFence {
public:
	ID3D12Fence* fence;
	HANDLE eventHandle;
	UINT64 value = 0;
	void create(ID3D12Device5* device) {
		device->CreateFence(value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		eventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	void signal(ID3D12CommandQueue* queue) {
		queue->Signal(fence, ++value);
	}
	void wait() {
		if (fence->GetCompletedValue() < value) {
			fence->SetEventOnCompletion(value, eventHandle);
			WaitForSingleObject(eventHandle, INFINITE);
		}
	}
	~GPUFence() {
		CloseHandle(eventHandle);
		fence->Release();
	}
};
class DescriptorHeap
{
public:
	ID3D12DescriptorHeap* heap;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	unsigned int incrementSize;
	int used;

	void init(ID3D12Device5* device, int num)
	{
		D3D12_DESCRIPTOR_HEAP_DESC uavcbvHeapDesc = {};
		uavcbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		uavcbvHeapDesc.NumDescriptors = num;
		uavcbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		device->CreateDescriptorHeap(&uavcbvHeapDesc, IID_PPV_ARGS(&heap));
		cpuHandle = heap->GetCPUDescriptorHandleForHeapStart();
		gpuHandle = heap->GetGPUDescriptorHandleForHeapStart();
		incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		used = 0;
	}
	
	
	D3D12_CPU_DESCRIPTOR_HANDLE getNextCPUHandle()
	{
		if (used > 0)
		{
			cpuHandle.ptr += incrementSize;
		}
		used++;
		return cpuHandle;
	}


};

class Core
{
public:
	IDXGIAdapter1* adapter;

	ID3D12Device5* device;
	ID3D12CommandQueue* graphicsQueue;
	ID3D12CommandQueue* copyQueue;
	ID3D12CommandQueue* computeQueue;
	IDXGISwapChain3* swapchain;
	ID3D12CommandAllocator* graphicsCommandAllocator[2];
	ID3D12GraphicsCommandList4* graphicsCommandList[2];

	ID3D12DescriptorHeap* backbufferHeap;
	ID3D12Resource** backbuffers;

	GPUFence graphicsQueueFence[2];

	// init depth buffer
	ID3D12DescriptorHeap* dsvHeap;
	ID3D12Resource* dsv;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	ID3D12RootSignature* rootSignature;

	DescriptorHeap srvHeap;

	UINT srvTableRootIndex = 0;
	~Core()
	{
		rootSignature->Release();
		graphicsCommandList[0]->Release();
		graphicsCommandAllocator[0]->Release();
		graphicsCommandList[1]->Release();
		graphicsCommandAllocator[1]->Release();
		swapchain->Release();
		computeQueue->Release();
		copyQueue->Release();
		graphicsQueue->Release();
		device->Release();

	}

	inline void init(HWND hwnd, int _width, int _height)
	{
		ID3D12Debug1* debug;
		D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
		debug->EnableDebugLayer();
		debug->Release();

		IDXGIAdapter1* adapterf;
		std::vector<IDXGIAdapter1*> adapters;
		IDXGIFactory6* factory = NULL;
		CreateDXGIFactory(__uuidof(IDXGIFactory6), (void**)&factory);
		int i = 0;
		while (factory->EnumAdapters1(i, &adapterf) != DXGI_ERROR_NOT_FOUND)
		{
			adapters.push_back(adapterf);
			i++;
		}

		long long maxVideoMemory = 0;
		int useAdapterIndex = 0;
		for (int i = 0; i < adapters.size(); i++)
		{
			DXGI_ADAPTER_DESC desc;
			adapters[i]->GetDesc(&desc);
			if (desc.DedicatedVideoMemory > maxVideoMemory)
			{
				maxVideoMemory = desc.DedicatedVideoMemory;
				useAdapterIndex = i;
			}
		}
		adapter = adapters[useAdapterIndex];
		//factory->Release();

		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
		D3D12_COMMAND_QUEUE_DESC graphicsQueueDesc = {};
		graphicsQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		device->CreateCommandQueue(&graphicsQueueDesc, IID_PPV_ARGS(&graphicsQueue));
		D3D12_COMMAND_QUEUE_DESC copyQueueDesc = {};
		copyQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		device->CreateCommandQueue(&copyQueueDesc, IID_PPV_ARGS(&copyQueue));
		D3D12_COMMAND_QUEUE_DESC computeQueueDesc = {};
		computeQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		device->CreateCommandQueue(&computeQueueDesc, IID_PPV_ARGS(&computeQueue));

		// swapchain
		DXGI_SWAP_CHAIN_DESC1 scDesc = {};
		scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scDesc.Width = _width;
		scDesc.Height = _height;
		scDesc.SampleDesc.Count = 1; // MSAA here
		scDesc.SampleDesc.Quality = 0;
		scDesc.BufferCount = 2;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		IDXGISwapChain1* swapChain1;
		factory->CreateSwapChainForHwnd(graphicsQueue, hwnd, &scDesc, NULL, NULL, &swapChain1);
		swapChain1->QueryInterface(&swapchain);
		swapChain1->Release();

		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&graphicsCommandAllocator[0]));
		device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE,
			IID_PPV_ARGS(&graphicsCommandList[0]));
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&graphicsCommandAllocator[1]));
		device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE,
			IID_PPV_ARGS(&graphicsCommandList[1]));

		// init heap and resource
		D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc = {};
		renderTargetViewHeapDesc.NumDescriptors = scDesc.BufferCount;
		renderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		device->CreateDescriptorHeap(&renderTargetViewHeapDesc, IID_PPV_ARGS(&backbufferHeap));

		backbuffers = new ID3D12Resource * [scDesc.BufferCount];

		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = backbufferHeap->GetCPUDescriptorHandleForHeapStart();
		unsigned int renderTargetViewDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		for (unsigned int i = 0; i < 2; i++)
		{
			swapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffers[i]));
			device->CreateRenderTargetView(backbuffers[i], nullptr, renderTargetViewHandle);
			renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;

		}

		// Fence
		graphicsQueueFence[0].create(device);
		graphicsQueueFence[1].create(device);
		
		// depth buffer
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		memset(&dsvHeapDesc, 0, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
		dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
		
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
		D3D12_CLEAR_VALUE depthClearValue = {};
		depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthClearValue.DepthStencil.Depth = 1.0f;
		depthClearValue.DepthStencil.Stencil = 0;

		D3D12_HEAP_PROPERTIES heapprops = {};
		heapprops.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapprops.CreationNodeMask = 1;
		heapprops.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.Width = _width;
		dsvDesc.Height = _height;
		dsvDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		dsvDesc.DepthOrArraySize = 1;
		dsvDesc.MipLevels = 1;
		dsvDesc.SampleDesc.Count = 1;
		dsvDesc.SampleDesc.Quality = 0;
		dsvDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		dsvDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &dsvDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&dsv));
		device->CreateDepthStencilView(dsv, &depthStencilDesc, dsvHandle);

		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = (float)_width;
		viewport.Height = (float)_height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = _width;
		scissorRect.bottom = _height;

		//root signature 
		std::vector<D3D12_ROOT_PARAMETER> parameters;
		
		// VS CBV : b0 Index 0
		D3D12_ROOT_PARAMETER rootParameterCBVS;
		rootParameterCBVS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameterCBVS.Descriptor.ShaderRegister = 0; // Register(b0)
		rootParameterCBVS.Descriptor.RegisterSpace = 0;
		rootParameterCBVS.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		parameters.push_back(rootParameterCBVS);
		// VS CBV : b1 Index 1
		D3D12_ROOT_PARAMETER rootParameterCBVS_b1;
		rootParameterCBVS_b1.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameterCBVS_b1.Descriptor.ShaderRegister = 1; // Register(b1)
		rootParameterCBVS_b1.Descriptor.RegisterSpace = 0;
		rootParameterCBVS_b1.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		parameters.push_back(rootParameterCBVS_b1);
		// PS CBV : b2 Index 2
		D3D12_ROOT_PARAMETER rootParameterCBPS;
		rootParameterCBPS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameterCBPS.Descriptor.ShaderRegister = 2; // Register(b6)
		rootParameterCBPS.Descriptor.RegisterSpace = 0;
		rootParameterCBPS.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		parameters.push_back(rootParameterCBPS);
		

		// PS SRV : s0 
		D3D12_DESCRIPTOR_RANGE srvRange = {};
		srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange.NumDescriptors = 8; // number of SRVs (t0¨Ct7)
		srvRange.BaseShaderRegister = 0; // starting at t0
		srvRange.RegisterSpace = 0;
		srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		D3D12_ROOT_PARAMETER rootParameterTex;
		rootParameterTex.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameterTex.DescriptorTable.NumDescriptorRanges = 1;
		rootParameterTex.DescriptorTable.pDescriptorRanges = &srvRange;
		rootParameterTex.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		parameters.push_back(rootParameterTex);
		srvTableRootIndex = parameters.size() - 1;

		// PS CBV : b3 Index 3
		D3D12_ROOT_PARAMETER rootParameterCBVS_b3;
		rootParameterCBVS_b3.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameterCBVS_b3.Descriptor.ShaderRegister = 3; // Register(b3)
		rootParameterCBVS_b3.Descriptor.RegisterSpace = 0;
		rootParameterCBVS_b3.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		parameters.push_back(rootParameterCBVS_b3);

		// sampler
		D3D12_STATIC_SAMPLER_DESC staticSampler = {};
		staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSampler.MipLODBias = 0;
		staticSampler.MaxAnisotropy = 1;
		staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		staticSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		staticSampler.MinLOD = 0.0f;
		staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
		staticSampler.ShaderRegister = 0;
		staticSampler.RegisterSpace = 0;
		staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		//staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.NumParameters = parameters.size();
		desc.pParameters = &parameters[0];
		desc.NumStaticSamplers = 1;
		desc.pStaticSamplers = &staticSampler;

		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		ID3DBlob* serialized;
		ID3DBlob* error;
		D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);
		device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized-> GetBufferSize(), IID_PPV_ARGS(&rootSignature));
		serialized->Release();

		// descriptor heap
		srvHeap.init(device, 16384);
	}

	// command list
	void resetCommandList()
	{
		unsigned int frameIndex = swapchain->GetCurrentBackBufferIndex();
		graphicsCommandAllocator[frameIndex]->Reset();
		graphicsCommandList[frameIndex]->Reset(graphicsCommandAllocator[frameIndex], NULL);
	}

	ID3D12GraphicsCommandList4* getCommandList()
	{
		unsigned int frameIndex = swapchain->GetCurrentBackBufferIndex();
		return graphicsCommandList[frameIndex];
	}

	void runCommandList()
	{
		getCommandList()->Close();
		ID3D12CommandList* lists[] = { getCommandList() };
		graphicsQueue->ExecuteCommandLists(1, lists);
	}

	// flush
	void flushGraphicsQueue() {
		graphicsQueueFence[0].signal(graphicsQueue);
		graphicsQueueFence[0].wait();
	}

	void beginFrame()
	{
		unsigned int frameIndex = swapchain->GetCurrentBackBufferIndex();
		graphicsQueueFence[frameIndex].wait();
		resetCommandList();

		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = backbufferHeap
			-> GetCPUDescriptorHandleForHeapStart();
		unsigned int renderTargetViewDescriptorSize = device
			-> GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		renderTargetViewHandle.ptr += frameIndex * renderTargetViewDescriptorSize;

		Barrier::add(backbuffers[frameIndex], D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET, getCommandList());
		getCommandList()->OMSetRenderTargets(1, &renderTargetViewHandle, FALSE, &dsvHandle);
		float color[4];
		color[0] = 0.f;
		color[1] = .8f;
		color[2] = .8f;
		color[3] = 1.0;
		getCommandList()->ClearRenderTargetView(renderTargetViewHandle, color, 0, NULL);
		getCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, NULL);

	}

	void finishFrame()
	{
		unsigned int frameIndex = swapchain->GetCurrentBackBufferIndex();
		Barrier::add(backbuffers[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT, getCommandList());
		runCommandList();
		graphicsQueueFence[frameIndex].signal(graphicsQueue);
		swapchain->Present(1, 0);
	}

	void beginRenderPass()
	{
		getCommandList()->RSSetViewports(1, &viewport);
		getCommandList()->RSSetScissorRects(1, &scissorRect);
		getCommandList()->SetGraphicsRootSignature(rootSignature);
	}


	void uploadResource(ID3D12Resource* dstResource, const void* data, unsigned int size,
		D3D12_RESOURCE_STATES targetState, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* texFootprint = NULL);

	int frameIndex()
	{
		return swapchain->GetCurrentBackBufferIndex();
	}

};




