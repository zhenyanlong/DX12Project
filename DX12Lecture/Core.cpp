#include "Core.h"


extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

void Core::uploadResource(ID3D12Resource* dstResource, const void* data, unsigned int size, D3D12_RESOURCE_STATES targetState, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* texFootprint)
{
	ID3D12Resource* uploadBuffer;
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = size;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&uploadBuffer));

	void* mappeddata = NULL;
	uploadBuffer->Map(0, NULL, &mappeddata);
	memcpy(mappeddata, data, size);
	uploadBuffer->Unmap(0, NULL);

	resetCommandList();

	if (texFootprint != NULL)
	{
		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = uploadBuffer;
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint = *texFootprint;
		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = dstResource;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;
		getCommandList()->CopyTextureRegion(&dst, 0, 0, 0, &src, NULL);
	}
	else
	{
		getCommandList()->CopyBufferRegion(dstResource, 0, uploadBuffer, 0, size);
	}

	Barrier::add(dstResource, D3D12_RESOURCE_STATE_COPY_DEST, targetState, getCommandList());
	runCommandList();
	flushGraphicsQueue();
	uploadBuffer->Release();

}
