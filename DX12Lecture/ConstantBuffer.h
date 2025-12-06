#pragma once

#include "Core.h"
#include <map>
#include <iostream>
#include "Vec3.h"
struct ConstantBufferVariable
{
	unsigned int offset;
	unsigned int size;
};

struct alignas(16) ConstantBuffer2	: ConstantBufferVariable
{
	float time;
	float padding[3];
	Vec4 lights[4];
};

class ConstantBuffer
{

	ID3D12Resource* constantBuffer;
	unsigned char* buffer;
	unsigned int cbSizeInBytes;
	unsigned int maxDrawCalls;
	unsigned int offsetIndex;

public:
	std::string name;
	std::map<std::string, ConstantBufferVariable> constantBufferData;

public:
	void init(Core* core, unsigned int sizeInBytes, unsigned int _maxDrawCalls = 1024);

	void update(std::string name, void* data)
	{
		ConstantBufferVariable cbVariable = constantBufferData[name];
		unsigned int offset = offsetIndex * cbSizeInBytes;
		//constantBuffer->Map(0, NULL, (void**)&buffer);
		memcpy(&buffer[offset + cbVariable.offset], data, cbVariable.size);
		//constantBuffer->Unmap(0, NULL);
	}
	D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const
	{
		return (constantBuffer->GetGPUVirtualAddress() + (offsetIndex * cbSizeInBytes));
	}

	void next()
	{
		offsetIndex++;
		if (offsetIndex >= maxDrawCalls)
		{
			offsetIndex = 0;
		}
	}

	static std::vector<ConstantBuffer> reflect(Core* core, ID3DBlob* shader)
	{
		std::vector<ConstantBuffer> buffers;
		ID3D12ShaderReflection* reflection;
		D3DReflect(shader->GetBufferPointer(), shader->GetBufferSize(), IID_PPV_ARGS(&reflection));
		D3D12_SHADER_DESC desc;
		reflection->GetDesc(&desc);
		unsigned int totalSize = 0;
		for (int i = 0; i < desc.ConstantBuffers; i++)
		{
			ConstantBuffer buffer;
			ID3D12ShaderReflectionConstantBuffer* constantBuffer = reflection->GetConstantBufferByIndex(i);
			D3D12_SHADER_BUFFER_DESC cbDesc;
			constantBuffer->GetDesc(&cbDesc);
			buffer.name = cbDesc.Name;
			

			for (int j = 0; j < cbDesc.Variables; j++)
			{
				ID3D12ShaderReflectionVariable* var = constantBuffer->GetVariableByIndex(j);
				D3D12_SHADER_VARIABLE_DESC vDesc;
				var->GetDesc(&vDesc);
				ConstantBufferVariable bufferVariable;
				bufferVariable.offset = vDesc.StartOffset;
				bufferVariable.size = vDesc.Size;
				buffer.constantBufferData.insert({ vDesc.Name, bufferVariable });
				totalSize += bufferVariable.size;

			}
		
			buffer.init(core,totalSize);
			buffers.push_back(buffer);
		}
		
		return buffers;
	}

	void free()
	{
		constantBuffer->Unmap(0, NULL);
		constantBuffer->Release();
	}
};

