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
// ===== 初始化全局映射表：HLSL寄存器编号 → 根签名索引（核心配置）=====
// 例如：b0→0, b1→1, b2→2, b3→3（与根签名的配置一致）
std::map<std::string, UINT> ConstantBuffer::RegisterToRootIndex = {
	{"staticMeshBuffer", 0},  // staticMeshBuffer (b0) → 根索引0
	{"instanceBuffer", 1},  // instanceBuffer (b1) → 根索引1
	{"PSLightBuffer", 2},  // PSLightBuffer (b2) → 根索引2
	{"WaterBuffer", 4}   // WaterBuffer (b3) → 根索引3
};