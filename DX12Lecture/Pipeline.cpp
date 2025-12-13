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
	float wavelength;  // 替换原frequency，存储波长
	float phaseOffset; // 相位偏移（HLSL中改为动态计算，这里预留可省略，或保留）
	float steepness;
	float padding[2];  // 保证32字节对齐（与HLSL一致）
};

// 水面常量缓冲区（与HLSL严格对齐）
struct alignas(16) WaterBuffer
{
	GerstnerWave waves[16]; // 对应HLSL的16个波
	int waveCount;          // 4字节
	float wavelengthMin;    // 4字节
	float wavelengthMax;    // 4字节
	float steepnessMin;     // 4字节
	float steepnessMax;     // 4字节
	float baseDirection[2];   // 8字节

	float randomDirection;  // 4字节
	float time;             // 4字节
	float scale;            // 4字节
	float waveHeightGain;	// 4字节
	int seed;               // 4字节
	float padding[2];       // 填充8字节，保证总大小为16的倍数
};
void Pipelines::updateWaveBuffer(const std::string& pipeName, Pipelines* pipes)
{
	World* myWorld = World::Get();
	// 静态变量：累计总时间、随机种子（只初始化一次）
	static float totalTime = 0.0f;
	static int seed = 12345; // 随机数种子（可自定义）
	
	float deltaTime = myWorld->GetDeltatime(); // 每帧时间增量（需在World中实现）
	totalTime += deltaTime;

	WaterBuffer waterData = {}; // 初始化所有成员为0

	// ===== 核心：配置自动波参数（可根据需求调整）=====
	waterData.waveCount = 16;          // 使用8个波叠加（效果更自然，可调整）
	waterData.wavelengthMin = 2.0f;   // 最小波长
	waterData.wavelengthMax = 10.0f;  // 最大波长
	waterData.steepnessMin = 0.1f;    // 最小陡度
	waterData.steepnessMax = 5.0f;    // 最大陡度
	waterData.baseDirection[0] = 1.0f; // 基础方向（如沿x+z方向）
	waterData.baseDirection[1] = 0.5f;
	waterData.randomDirection = 0.8f; // 方向随机度（0=完全沿基础方向，1=完全随机）
	waterData.time = totalTime;       // 累计时间
	waterData.scale = 0.001f;           // 水面缩放系数
	waterData.waveHeightGain = 5.f;		// wave height scale
	waterData.seed = seed;            // 随机数种子

	// ===== 更新常量缓冲区（只需更新配置参数，HLSL端自动计算每个波的参数）=====
	// 注意：waves数组在HLSL中是预留空间，C++端无需赋值，可传空或默认值
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
