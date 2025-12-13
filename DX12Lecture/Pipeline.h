#pragma once
#include "Core.h"
#include <dxgi.h>
#include <iostream>
#include "PSOManager.h"
#include <fstream>
#include <sstream>
#include <map>
#include "ConstantBuffer.h"
#include "TextureManager.h"


class Pipeline
{
public:
	ID3DBlob* vertexShader;
	ID3DBlob* pixelShader;

	std::string vertexShaderStr;
	std::string pixelShaderStr;

	std::vector<ConstantBuffer> psConstantBuffers;
	std::vector<ConstantBuffer> vsConstantBuffers;
	std::map<std::string, int> textureBindPoints;

	ID3D12RootSignature* rootSignature;

	std::string psoName;


public:
	Pipeline()
	{

	}

	std::string loadstr(std::string path)
	{
		std::ifstream file(path);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	void init(std::string vsPath= "VertexShader.hlsl", std::string psPath = "PixelShader.hlsl");
	
	void free()
	{
		pixelShader->Release();
		vertexShader->Release();
		for (auto cb : psConstantBuffers)
		{
			cb.free();
		}
		for (auto cb : vsConstantBuffers)
		{
			cb.free();
		}
	}
	
};


class Pipelines
{

public:
	std::map<std::string, Pipeline> pipelines;

	void loadPipeline(Core& core, std::string pipeName, Pipeline& pipe);

	void loadPipeline(Core& core, std::string pipeName, PSOManager& psos, std::string vsfilename, std::string psfilename, const D3D12_INPUT_LAYOUT_DESC& inputDesc);

	void loadPipeline(Core& core, std::string pipeName, PSOManager* const psos, std::string vsfilename, std::string psfilename, const D3D12_INPUT_LAYOUT_DESC& inputDesc);

	~Pipelines()
	{
		for (auto it = pipelines.begin(); it != pipelines.end(); )
		{
			it->second.free();
			pipelines.erase(it++);
		}
	}

	// update constant buffer
	static bool updateConstantBuffer(std::vector<ConstantBuffer>& constantBuffers, std::string bufferName, std::string dataName, void* data)
	{
		auto findIt = std::find_if(constantBuffers.begin(), constantBuffers.end(),
			[&](const ConstantBuffer& entry)
			{
				return entry.name == bufferName;
			}
		);
		if (findIt == constantBuffers.end())
		{
			return false;
		}
		findIt->update(dataName, data);

		return true;
	}

	// update world matrix and view_projection matrix
	static void updateBaseStaticBuffer(const std::string& pipeName, Pipelines* pipes, Matrix worldPosMat);
	

	// update instance matrix
	static void updateInstanceBuffer(const std::string& pipeName, Pipelines* pipes, std::vector<Matrix>* const instanceMatrices);
	

	// update light info
	static void updateLightBuffer(const std::string& pipeName, Pipelines* pipes);
	
	// update water vertex info
	static void updateWaveBuffer(const std::string& pipeName, Pipelines* pipes);


	static void updateTexture(std::map<std::string, int>* const textureBindPoints, Core* core, std::string name, int heapOffset, int srvRootIndex = 3) 
	{
		UINT bindPoint = textureBindPoints->find(name)->second;
		D3D12_GPU_DESCRIPTOR_HANDLE handle = core->srvHeap.gpuHandle;
		handle.ptr = handle.ptr + (UINT64)(heapOffset) * (UINT64)core->srvHeap.incrementSize;
		core->getCommandList()->SetGraphicsRootDescriptorTable(srvRootIndex, handle);
	}

	static void updateTexture(std::map<std::string, int>* const textureBindPoints, Core* core, const std::map<std::string, int>& textureHeapOffsets, int srvRootIndex = 3)
	{
		for (const auto& pair : textureHeapOffsets)
		{
			std::string texName = pair.first; // "albedo_tex" / "normal_tex"
			int heapOffset = pair.second;

			auto it = textureBindPoints->find(texName);
			if (it == textureBindPoints->end())
			{
				//OutputDebugStringA(("Texture bind point not found: " + texName).c_str());
				continue;
			}

			UINT bindPoint = it->second;
			D3D12_GPU_DESCRIPTOR_HANDLE handle = core->srvHeap.gpuHandle;
			handle.ptr += (UINT64)(heapOffset) * core->srvHeap.incrementSize;

			// 注意：这里的根参数索引是Pipeline中记录的srvTableRootIndex（原修改后的动态索引）
			// 假设Pipeline的srvTableRootIndex是3，且SRV范围是t0-t7，直接设置描述符表即可
			// 若需要单独设置每个SRV，需修改根签名为多个Descriptor Table，这里简化为批量绑定
			core->getCommandList()->SetGraphicsRootDescriptorTable(srvRootIndex, handle);
		}
	}
	static void submitToCommandList(Core* core, std::vector<ConstantBuffer>& constantBuffers, UINT rootIndexOffset = 0)
	{
		for (int i = 0; i < constantBuffers.size(); i++)
		{
			//UINT rootIndex = rootIndexOffset + i;
			UINT rootIndex = ConstantBuffer::RegisterToRootIndex[constantBuffers[i].name];
			core->getCommandList()->SetGraphicsRootConstantBufferView(rootIndex, constantBuffers[i].getGPUAddress());
			constantBuffers[i].next();
		}
	}
private:
	//Pipelines() = default;
	//Pipelines& operator=(const Pipelines& pipe) = delete;
	
	
};

