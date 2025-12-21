#include "ConstantBuffer.h"

void ConstantBuffer::init(Core* core, unsigned int sizeInBytes, unsigned int _maxDrawCalls)
{
	cbSizeInBytes = (sizeInBytes + 255) & ~255;
	unsigned int cbSizeInBytesAligned = cbSizeInBytes * _maxDrawCalls;
	maxDrawCalls = _maxDrawCalls;
	offsetIndex = 0;
	HRESULT hr;
	D3D12_HEAP_PROPERTIES heapprops = {};
	heapprops.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprops.CreationNodeMask = 1;
	heapprops.VisibleNodeMask = 1;
	D3D12_RESOURCE_DESC cbDesc = {};
	cbDesc.Width = cbSizeInBytesAligned;
	cbDesc.Height = 1;
	cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbDesc.DepthOrArraySize = 1;
	cbDesc.MipLevels = 1;
	cbDesc.SampleDesc.Count = 1;
	cbDesc.SampleDesc.Quality = 0;
	cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &cbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		IID_PPV_ARGS(&constantBuffer));
	//constantBuffer->Map(0, NULL, (void**)&buffer);

}


std::map<std::string, UINT> ConstantBuffer::RegisterToRootIndex = {
	{"staticMeshBuffer", 0},	// staticMeshBuffer (b0) ¡ú Index 0
	{"instanceBuffer", 1},		// instanceBuffer (b1) ¡ú Index 1
	{"PSLightBuffer", 2},		// PSLightBuffer (b2) ¡ú Index 2
	{"WaterBuffer", 4},			// WaterBuffer (b3) ¡ú Index 4
	{"AnimMeshBuffer", 5}		// AnimMeshBuffer (b4) ¡ú Index 5 
};