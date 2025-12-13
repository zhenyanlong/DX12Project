#include "Pipeline.h"
#include "VertexLayoutCache.h"
#include "Mesh.h"
#include "World.h"

void Pipeline::init(std::string vsPath, std::string psPath)
{
	vertexShaderStr = loadstr(vsPath);
	pixelShaderStr = loadstr(psPath);

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

void Pipelines::loadPipeline(Core& core, std::string pipeName, Pipeline& pipe)
{
	auto findIt = pipelines.find(pipeName);
	if (findIt != pipelines.end())
	{
		return;
	}


}

void Pipelines::loadPipeline(Core& core, std::string pipeName, PSOManager& psos, std::string vsfilename, std::string psfilename, const D3D12_INPUT_LAYOUT_DESC& inputDesc)
{
	auto findIt = pipelines.find(pipeName);
	if (findIt != pipelines.end())
	{
		return;
	}			
	// init pipe 
	Pipeline pipe;
	// init shaders
	pipe.init(vsfilename, psfilename);
	// init buffers
	pipe.vsConstantBuffers = ConstantBuffer::reflect(&core, pipe.vertexShader);
	pipe.psConstantBuffers = ConstantBuffer::reflect(&core, pipe.pixelShader, &pipe.textureBindPoints);
	// init psos
	psos.createPSO(&core, pipeName, pipe.vertexShader, pipe.pixelShader, inputDesc);
	pipe.psoName = pipeName;

	pipelines.insert({ pipeName, pipe });


}

void Pipelines::loadPipeline(Core& core, std::string pipeName, PSOManager* const psos, std::string vsfilename, std::string psfilename, const D3D12_INPUT_LAYOUT_DESC& inputDesc)
{
	auto findIt = pipelines.find(pipeName);
	if (findIt != pipelines.end())
	{
		return;
	}
	// init pipe 
	Pipeline pipe;
	// init shaders
	pipe.init(vsfilename, psfilename);
	// init buffers
	pipe.vsConstantBuffers = ConstantBuffer::reflect(&core, pipe.vertexShader);
	pipe.psConstantBuffers = ConstantBuffer::reflect(&core, pipe.pixelShader, &pipe.textureBindPoints);
	// init psos
	psos->createPSO(&core, pipeName, pipe.vertexShader, pipe.pixelShader, inputDesc);
	pipe.psoName = pipeName;

	pipelines.insert({ pipeName, pipe });				   
}

void Pipelines::updateBaseStaticBuffer(const std::string& pipeName, Pipelines* pipes, Matrix worldPosMat)
{
	GeneralMatrix* gm = GeneralMatrix::Get();
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"staticMeshBuffer", "W", &worldPosMat);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"staticMeshBuffer", "VP", &gm->viewProjMatrix);
}

void Pipelines::updateInstanceBuffer(const std::string& pipeName, Pipelines* pipes, std::vector<Matrix>* const instanceMatrices)
{
	GeneralMatrix* gm = GeneralMatrix::Get();
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"staticMeshBuffer", "VP", &gm->viewProjMatrix);
	if (instanceMatrices != nullptr)
	{
		Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
			"instanceBuffer", "instanceMatrices", instanceMatrices->data());
	}
}

void Pipelines::updateLightBuffer(const std::string& pipeName, Pipelines* pipes)
{

	Vec3 lightDir = Vec3(0.f, 1.0f, 0.f).normalize();
	float lightIntensity = 3.0f;
	Vec3 lightColor = Vec3(1.0f, 1.0f, 1.0f);
	float roughness = 0.3f;

	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].psConstantBuffers,
		"PSLightBuffer", "lightDir", &lightDir);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].psConstantBuffers,
		"PSLightBuffer", "lightIntensity", &lightIntensity);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].psConstantBuffers,
		"PSLightBuffer", "lightColor", &lightColor);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].psConstantBuffers,
		"PSLightBuffer", "roughness", &roughness);
}
struct GerstnerWave
{
	float directionX; // 对应HLSL的float2（8字节）
	float directionY;
	float amplitude;              // 4字节（累计12）
	float frequency;              // 4字节（累计16，刚好对齐）
	float phase;                  // 4字节
	float steepness;              // 4字节
	// 补充填充：HLSL中结构体默认按16字节对齐，需添加2个float填充（共24→32字节，16的倍数）
	float padding[2];
};

// 水面常量缓冲区（与HLSL的WaterBuffer对齐）
struct alignas(16) WaterBuffer
{
	GerstnerWave waves[4]; // 4个波：4×32=128字节
	float time;            // 4字节
	float scale;           // 4字节
	float padding[2];      // 填充8字节，总大小144字节（16的倍数）
};
void Pipelines::updateWaveBuffer(const std::string& pipeName, Pipelines* pipes)
{
	WaterBuffer waterData;
	waterData.time = 0.0f;
	waterData.scale = 0.001f; // 水面缩放系数（控制波的密度）

	// 波1：主波（大振幅，低频率）
	waterData.waves[0].directionX = 1.0f;
	waterData.waves[0].directionY = 0.5f;
	waterData.waves[0].amplitude = 0.5f* 200;
	waterData.waves[0].frequency = 0.5f ;
	waterData.waves[0].phase = 0.0f;
	waterData.waves[0].steepness = 0.8f ;

	// 波2：次波（小振幅，高频率）
	waterData.waves[1].directionX = -0.3f;
	waterData.waves[1].directionY = 1.0f;
	waterData.waves[1].amplitude = 0.2f * 200;
	waterData.waves[1].frequency = 1.2f ;
	waterData.waves[1].phase = 0.5f ;
	waterData.waves[1].steepness = 0.5f ;

	// 波3：扰动波
	waterData.waves[2].directionX = 0.8f;
	waterData.waves[2].directionY = -0.2f;
	waterData.waves[2].amplitude = 0.1f * 200;
	waterData.waves[2].frequency = 2.0f ;
	waterData.waves[2].phase = 0.2f ;
	waterData.waves[2].steepness = 0.3f ;

	// 波4：扰动波
	waterData.waves[3].directionX = -0.5f;
	waterData.waves[3].directionY = -0.8f;
	waterData.waves[3].amplitude = 0.05f * 200;
	waterData.waves[3].frequency = 2.0f;
	waterData.waves[3].phase = 0.7f;
	waterData.waves[3].steepness = 0.2f;

	// 实时更新时间（每帧增加）	
	World* myMorld = World::Get();
	waterData.time += myMorld->GetCultime(); // deltaTime是每帧的时间间隔

	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"WaterBuffer", "waves", waterData.waves);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"WaterBuffer", "time", &waterData.time);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"WaterBuffer", "scale", &waterData.scale);
}
