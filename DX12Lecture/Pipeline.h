#pragma once
#include "Core.h"
#include <dxgi.h>
#include <iostream>
#include "PSOManager.h"
#include <fstream>
#include <sstream>
#include <map>
#include "ConstantBuffer.h"



class Pipeline
{
public:
	ID3DBlob* vertexShader;
	ID3DBlob* pixelShader;

	std::string vertexShaderStr;
	std::string pixelShaderStr;

	std::vector<ConstantBuffer> psConstantBuffers;
	std::vector<ConstantBuffer> vsConstantBuffers;

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
	/*void apply(Core* core)
	{
		for (int i = 0; i < vsConstantBuffers.size(); i++)
		{
			core->getCommandList()->SetGraphicsRootConstantBufferView(i, vsConstantBuffers[i].getGPUAddress());
			vsConstantBuffers[i].next();
		}
		for (int i = 0; i < psConstantBuffers.size(); i++)
		{
			core->getCommandList()->SetGraphicsRootConstantBufferView(i, psConstantBuffers[i].getGPUAddress());
			psConstantBuffers[i].next();
		}
	}*/
	//void apply(Core* core)
	//{
	//	for (int i = 0; i < vsConstantBuffers.size(); i++)
	//	{
	//		core->getCommandList()->SetGraphicsRootConstantBufferView(i, vsConstantBuffers[i].getGPUAddress());
	//		vsConstantBuffers[i].next();
	//	}
	//	for (int i = 0; i < psConstantBuffers.size(); i++)
	//	{
	//		core->getCommandList()->SetGraphicsRootConstantBufferView(i, psConstantBuffers[i].getGPUAddress());
	//		psConstantBuffers[i].next();
	//	}
	//}
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

	~Pipelines()
	{
		for (auto it = pipelines.begin(); it != pipelines.end(); )
		{
			it->second.free();
			pipelines.erase(it++);
		}
	}

	/*Pipelines(const Pipelines& Layer) = delete;
	Pipelines& operator=(const Pipelines& Layer) = delete;*/
	// singleton 
	/*inline static Pipelines& Get()
	{
		static Pipelines instance;
		return instance;
	}*/

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

	static void submitToCommandList(Core* core, std::vector<ConstantBuffer>& constantBuffers)
	{
		for (int i = 0; i < constantBuffers.size(); i++)
		{
			core->getCommandList()->SetGraphicsRootConstantBufferView(i, constantBuffers[i].getGPUAddress());
			constantBuffers[i].next();
		}
	}
private:
	//Pipelines() = default;
	//Pipelines& operator=(const Pipelines& pipe) = delete;
	
	
};

