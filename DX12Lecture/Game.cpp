#include "Window.h"
#include "Core.h"
#include "Vec3.h"
#include "Mesh.h"
#include "PSOManager.h"
#include "Pipeline.h"
#include "ConstantBuffer.h"
#include "VertexLayoutCache.h"
#include "algorithm"
//#include "GamesEngineeringBase.h"
#define M_PI       3.14159265358979323846   // pi

#define WIDTH 1280
#define HEIGHT 720

class Timer
{
private:
	LARGE_INTEGER freq;   // Frequency of the performance counter
	LARGE_INTEGER start;  // Starting time

public:
	// Constructor that initializes the frequency
	Timer()
	{
		QueryPerformanceFrequency(&freq);
		reset();
	}

	// Resets the timer
	void reset()
	{
		QueryPerformanceCounter(&start);
	}

	// Returns the elapsed time since the last reset in seconds. Note this should only be called once per frame as it resets the timer.
	float dt()
	{
		LARGE_INTEGER cur;
		QueryPerformanceCounter(&cur);
		float value = static_cast<float>(cur.QuadPart - start.QuadPart) / freq.QuadPart;
		reset();
		return value;
	}
};



void draw(Core* core, PSOManager& psos, Mesh& prim) {
	core->beginRenderPass();
	psos.bind(core, "Triangle");
	prim.draw(core);
}

bool updateConstantBuffer(std::vector<ConstantBuffer>& constantBuffers, std::string bufferName, std::string dataName, void* data)
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

void submitToCommandList(Core* core, std::vector<ConstantBuffer>& constantBuffers)
{
	for (int i = 0; i < constantBuffers.size(); i++)
	{
		core->getCommandList()->SetGraphicsRootConstantBufferView(i, constantBuffers[i].getGPUAddress());
		constantBuffers[i].next();
	}
}


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, int nCmdShow)
{
	Window win;
	Core core;

	win.create(WIDTH, HEIGHT, "My Window");

	core.init(win.hwnd, WIDTH, HEIGHT);

	ScreenSpaceTriangle tri;
	// init mesh
	//Mesh mesh;
	//Mesh::CreatePlane(&core, &mesh);
	//Mesh::CreateCube(&core, &mesh);
	//Mesh::CreateSphere(&core, &mesh, 16, 24, 5);
	std::vector<Mesh> meshes;
	Mesh::CreateGEM(&core, meshes, "Resources/acacia_003.gem");

	Pipeline pipe;
	pipe.init();
	PSOManager psos;
	psos.createPSO(&core, "Triangle", pipe.vertexShader, pipe.pixelShader, meshes[0].inputLayoutDesc);

	std::vector<ConstantBuffer> vsBuffers = ConstantBuffer::reflect(&core, pipe.vertexShader);


	ConstantBuffer2 constBufferCPU2;
	constBufferCPU2.time = 0;
	for (int i =0;i<4 ;i++)
	{
		constBufferCPU2.lights[i] = Vec4(0,0,0,0);
	}

	Timer timer;
	float cultime=0;
	float dt;

	while (true)
	{
		//core.resetCommandList();

		win.processMessages();

		core.beginFrame();

		core.getCommandList()->SetGraphicsRootSignature(core.rootSignature);

		dt = timer.dt();
		cultime += dt;
		Vec4 cameraPos(40 * cos(cultime), 15.0f,40 * sin(cultime), 1.0f); 
		Vec4 cameraTarget(0.0f, 1.0f, 0.0f, 1.0f);                  
		Vec4 cameraUp(0.0f, 1.0f, 0.0f, 0.0f);                      

		/*Vec4 from = Vec4(11 * cos(0), 5, 11 * sinf(0), 1.f);
		Matrix w = Matrix::GetProjectionMatrix(90, 0.1, 10);
		Matrix v = Matrix::GetLookAtMatrix(from, Vec4(0, 1, 0,1), Vec4(0, 1, 0,0));*/
		//Matrix w = Matrix::SetPositionMatrix(Vec4(0, 0, 0, 1));
		Matrix worldMatrix = Matrix::SetPositionMatrix(Vec4(0.0f, 0.0f, 0.0f, 1.0f)); 
		Matrix::SetScaling(worldMatrix, Vec3(0.02f, 0.02f, 0.02f));
		Matrix viewMatrix = Matrix::GetLookAtMatrix(cameraPos, cameraTarget, cameraUp); 
		Matrix projMatrix = Matrix::GetProjectionMatrix(45.0f, 1.f, 100.0f); 
		Matrix viewProjMatrix = viewMatrix.mul(projMatrix);

		
		//vsBuffers[0].update("W", &worldMatrix);
		//vsBuffers[0].update("VP", &viewProjMatrix);
		updateConstantBuffer(vsBuffers, "staticMeshBuffer", "W", &worldMatrix);
		updateConstantBuffer(vsBuffers, "staticMeshBuffer", "VP", &viewProjMatrix);
		submitToCommandList(&core, vsBuffers);
		//draw(&core, psos, mesh);
		for (int i=0;i<meshes.size();i++)
		{
			draw(&core, psos, meshes[i]);
		}
		
		//core.getCommandList()->SetGraphicsRootSignature(core.rootSignature);
		/*worldMatrix = Matrix::SetPositionMatrix(Vec4(15.0f, 0.0f, 0.0f, 1.0f));

		updateConstantBuffer(vsBuffers, "staticMeshBuffer", "W", &worldMatrix);
		updateConstantBuffer(vsBuffers, "staticMeshBuffer", "VP", &viewProjMatrix);
		submitToCommandList(&core, vsBuffers);
		draw(&core, psos, mesh);*/
		
		
		core.finishFrame();
		
		
		


		
	


	}
	
	core.flushGraphicsQueue();


	return 0;

}