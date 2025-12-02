#include "Pipeline.h"

void Pipeline::init()
{
	vertexShaderStr = loadstr("VertexShader.hlsl");
	pixelShaderStr = loadstr("PixelShader.hlsl");

	ID3DBlob* status;

	HRESULT hr = D3DCompile(vertexShaderStr.c_str(), strlen(vertexShaderStr.c_str()), NULL,
		NULL, NULL, "VS", "vs_5_0", 0, 0, &vertexShader, &status);

	if (FAILED(hr))
	{
		(char*)status->GetBufferPointer();

	}

	hr = D3DCompile(pixelShaderStr.c_str(), strlen(pixelShaderStr.c_str()), NULL, NULL,
		NULL, "PS", "ps_5_0", 0, 0, &pixelShader, &status);

	if (FAILED(hr))
	{
		(char*)status->GetBufferPointer();

	}

	

}
