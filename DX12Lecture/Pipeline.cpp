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

	Vec3 lightDir = Vec3(0.5f, 1.0f, 0.5f).normalize();
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
	float directionX;
	float directionY;
	float amplitude;
	float wavelength;  
	float phaseOffset; 
	float steepness;
	float padding[2];  
};

// WaterBuffer
struct alignas(16) WaterBuffer
{
	GerstnerWave waves[16]; 
	int waveCount;          
	float wavelengthMin;    
	float wavelengthMax;    
	float steepnessMin;     
	float steepnessMax;     
	float baseDirection[2]; 

	float randomDirection;  
	float time;             
	float scale;            
	float waveHeightGain;	
	int seed;               
	float padding[2];       
};
void Pipelines::updateWaveBuffer(const std::string& pipeName, Pipelines* pipes)
{
	World* myWorld = World::Get();
	
	static float totalTime = 0.0f;
	static int seed = 12345; 
	
	float deltaTime = myWorld->GetDeltatime(); 
	totalTime += deltaTime;

	WaterBuffer waterData = {}; // init data

	
	waterData.waveCount = 16;         
	waterData.wavelengthMin = 2.0f;   
	waterData.wavelengthMax = 10.0f;  
	waterData.steepnessMin = 0.1f;    
	waterData.steepnessMax = 5.0f;    
	waterData.baseDirection[0] = 1.0f;
	waterData.baseDirection[1] = 0.5f;
	waterData.randomDirection = 0.8f; 
	waterData.time = totalTime;       
	waterData.scale = 0.001f;         
	waterData.waveHeightGain = 5.f;	
	waterData.seed = seed;            

	// update constant buffer
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"WaterBuffer", "waveCount", &waterData.waveCount);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"WaterBuffer", "wavelengthMin", &waterData.wavelengthMin);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"WaterBuffer", "wavelengthMax", &waterData.wavelengthMax);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"WaterBuffer", "steepnessMin", &waterData.steepnessMin);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"WaterBuffer", "steepnessMax", &waterData.steepnessMax);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"WaterBuffer", "baseDirection", &waterData.baseDirection);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"WaterBuffer", "randomDirection", &waterData.randomDirection);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"WaterBuffer", "time", &waterData.time);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"WaterBuffer", "scale", &waterData.scale);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"WaterBuffer", "waveHeightGain", &waterData.waveHeightGain);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"WaterBuffer", "seed", &waterData.seed);
}
