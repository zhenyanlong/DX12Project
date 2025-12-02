#pragma once
#include "Core.h"
#include <dxgi.h>
#include <iostream>
#include "PSOManager.h"
#include <fstream>
#include <sstream>




class Pipeline
{
public:
	ID3DBlob* vertexShader;
	ID3DBlob* pixelShader;

	std::string vertexShaderStr;
	std::string pixelShaderStr;

	ID3D12RootSignature* rootSignature;

	PSOManager psos;


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

	void init();
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

	
};

