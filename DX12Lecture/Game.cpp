#include "Window.h"
#include "Core.h"
#include "Vec3.h"
#include "Mesh.h"
#include "PSOManager.h"
#include "Pipeline.h"
#include "ConstantBuffer.h"
#include "VertexLayoutCache.h"
#include "algorithm"
#include "World.h"
#include "TextureManager.h"
#include "World.h"
//#include "GamesEngineeringBase.h"
#define M_PI       3.14159265358979323846   // pi

#define WIDTH 1280
#define HEIGHT 720

// Camera Parameters
const float CAMERA_MOVE_SPEED = 15.0f;    // 移动速度（单位/秒）
const float MOUSE_SENSITIVITY = 0.15f;    // 鼠标灵敏度（弧度/像素，越小越慢）
const bool LOCK_MOUSE_ON_START = true;    // 启动时自动锁定鼠标





void draw(Core* core, PSOManager& psos, Mesh& prim) {
	core->beginRenderPass();
	//psos.bind(core, "Triangle");
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
	// activate or create single instance
	Window win;
	Core core;
	
	win.create(WIDTH, HEIGHT, "My Window");
	core.init(win.hwnd, WIDTH, HEIGHT);

	World* myWorld = World::Create(core);
	GeneralMatrix* gm = GeneralMatrix::Create();
	TextureManager* textures = TextureManager::Create();

	// load new level with shared ptr
	myWorld->LoadNewLevel(std::make_shared<TestMap>());

	Actor* mainActor = myWorld->GetLevel()->GetActor("FPSActor");
	CameraControllable* mainCameraController = dynamic_cast<CameraControllable*>(mainActor);

	ScreenSpaceTriangle tri;
	// init mesh
	//Mesh mesh;
	//Mesh::CreatePlane(&core, &mesh);
	//Mesh::CreateCube(&core, &mesh);
	//Mesh::CreateSphere(&core, &mesh, 16, 24, 5);
	//StaticMesh staticMesh(&core, "Models/acacia_003.gem");
	//AnimatedModel animMesh(&core, "Models/TRex.gem");
	/*StaticMesh SkySphere;
	SkySphere.CreateFromSphere(&core, 16, 24, 5, "Models/Textures/sky.png");
	SkySphere.SetWorldScaling(Vec3(1000.f, 1000.f, 1000.f));
	SkySphere.SetWorldRotationRadian(Vec3(M_PI, 0.f, 0.f));*/
	//std::vector<Mesh> meshes;
	//Mesh::CreateGEM(&core, staticMesh.meshes, "Resources/acacia_003.gem");
	//init general matrix
	

	Pipeline pipe;
	
	pipe.init();
	//PSOManager psos;

	//psos.createPSO(&core, "Triangle", pipe.vertexShader, pipe.pixelShader, VertexLayoutCache::getStaticLayout());

	std::vector<ConstantBuffer> vsBuffers = ConstantBuffer::reflect(&core, pipe.vertexShader);


	//Pipelines::Get();
	//Pipelines pipes;
	
	//AnimationInstance animatedInstance;
	//animatedInstance.init(&animMesh.animation, 0);

	ConstantBuffer2 constBufferCPU2;
	constBufferCPU2.time = 0;
	for (int i =0;i<4 ;i++)
	{
		constBufferCPU2.lights[i] = Vec4(0,0,0,0);
	}

	

	// ===================== FPS相机核心状态 =====================
	Vec3 cameraPos = Vec3(40.0f, 15.0f, 0.0f);  // 相机初始位置（FPS中位置由WASD控制）
	float cameraYaw = -M_PI / 2;                  // 偏航角（初始朝向Z轴负方向，对应FPS初始向前）
	float cameraPitch = 0.0f;                   // 俯仰角（初始水平）
	bool mouseLocked = LOCK_MOUSE_ON_START;     // 鼠标是否锁定
	POINT windowCenter;                         // 窗口中心坐标（用于鼠标重置）

	// 计算窗口中心（相对于屏幕）
	RECT windowRect;
	GetWindowRect(win.hwnd, &windowRect);
	windowCenter.x = windowRect.left + WIDTH / 2;
	windowCenter.y = windowRect.top + HEIGHT / 2;

	// 初始锁定鼠标
	if (mouseLocked)
	{
		ClipCursor(&windowRect);  // 锁定鼠标在窗口内
		ShowCursor(FALSE);        // 隐藏鼠标指针
		SetCursorPos(windowCenter.x, windowCenter.y); // 重置鼠标到中心
	}
	while (true)
	{
		//Process messages 
		win.processMessages();
		// update time
		myWorld->UpdateTime();
		float dt = myWorld->GetDeltatime();
		// Process mouse control
		GetWindowRect(win.hwnd, &windowRect);
		windowCenter.x = windowRect.left + WIDTH / 2;
		windowCenter.y = windowRect.top + HEIGHT / 2;
		// 按ESC键切换鼠标锁定状态
		if (win.keys[VK_ESCAPE] && win.keyJustPressed[VK_ESCAPE])
		{
			mouseLocked = !mouseLocked;
			ShowCursor(mouseLocked ? FALSE : TRUE);
			if (mouseLocked)
			{
				GetWindowRect(win.hwnd, &windowRect);
				ClipCursor(&windowRect);
				SetCursorPos(windowCenter.x, windowCenter.y);
			}
			else
			{
				ClipCursor(NULL); // 解锁鼠标
			}
			win.keyJustPressed[VK_ESCAPE] = false; // 防止连续触发
		}

		core.beginFrame();

		core.getCommandList()->SetGraphicsRootSignature(core.rootSignature);
		core.getCommandList()->SetDescriptorHeaps(1, &core.srvHeap.heap);

		
		// begin play
		
		// ===================== 1. 鼠标控制视角（FPS核心） =====================
		if (mouseLocked ) // 窗口激活时才响应
		{
			POINT currentMousePos;
			GetCursorPos(&currentMousePos);

			// 计算鼠标相对于窗口中心的偏移（delta）
			int mouseDeltaX = currentMousePos.x - windowCenter.x;
			int mouseDeltaY = currentMousePos.y - windowCenter.y;

			// 重置鼠标到窗口中心（关键：避免指针移出窗口）
			SetCursorPos(windowCenter.x, windowCenter.y);

			// 更新偏航角（Yaw）和俯仰角（Pitch）
			cameraYaw -= mouseDeltaX * MOUSE_SENSITIVITY * 0.0174533f; // 转弧度
			cameraPitch -= mouseDeltaY * MOUSE_SENSITIVITY * 0.0174533f;

			// 限制俯仰角（-89° ~ +89°），避免视角翻转
			cameraPitch = clamp(cameraPitch, (float) - M_PI / 2 + 0.01f, (float)M_PI / 2 - 0.01f);

			// 根据欧拉角计算相机朝向（FPS中相机上方向固定为世界Y轴）
			Vec3 cameraForward;
			cameraForward.x = cos(cameraYaw) * cos(cameraPitch);
			cameraForward.y = sin(cameraPitch);
			cameraForward.z = sin(cameraYaw) * cos(cameraPitch);
			cameraForward = cameraForward.normalize();

			// 计算相机右方向（用于WASD横向移动）
			Vec3 cameraRight = Cross(cameraForward, Vec3(0.0f, 1.0f, 0.0f)).normalize();
			Vec3 cameraUp = Vec3(0.0f, 1.0f, 0.0f); // FPS固定上方向为世界Y轴

			// ===================== 2. WASD+QE控制相机移动 =====================
			// W/S：沿相机朝向前后移动
			if (win.keys['W']) cameraPos += cameraForward * CAMERA_MOVE_SPEED * dt;
			if (win.keys['S']) cameraPos -= cameraForward * CAMERA_MOVE_SPEED * dt;
			// A/D：沿相机右方向左右平移
			if (win.keys['A']) cameraPos += cameraRight * CAMERA_MOVE_SPEED * dt;
			if (win.keys['D']) cameraPos -= cameraRight * CAMERA_MOVE_SPEED * dt;
			// Q/E：垂直上下移动（世界Y轴方向）
			if (win.keys['Q']) cameraPos.y -= CAMERA_MOVE_SPEED * dt;
			if (win.keys['E']) cameraPos.y += CAMERA_MOVE_SPEED * dt;

			// ===================== 3. 更新观察矩阵 =====================
			Matrix p = Matrix::perspective(0.01f, 10000.0f, (float)WIDTH / HEIGHT, 45.0f);
			Matrix v = Matrix::lookAt(cameraPos, cameraPos + cameraForward, cameraUp);
			gm->viewProjMatrix = v * p;

			mainCameraController->updatePos(cameraPos);

			// 步骤1：补偿模型Yaw角的90度偏移（根据模型正朝向调整+/-M_PI/2，这里用+M_PI/2示例）
			float modelYaw = cameraYaw + M_PI / 2.0f;
			// 步骤2：生成绕世界Y轴的Yaw旋转四元数
			Quaternion qYaw = Quaternion::fromYRotation(-modelYaw);
			// 步骤3：生成绕模型局部X轴的Pitch旋转四元数
			Quaternion qPitch = Quaternion::fromXRotation(cameraPitch);
			// 步骤4：组合旋转四元数（顺序：先Yaw后Pitch，四元数乘法是qPitch * qYaw，因为右乘）
			Quaternion qTotal = qYaw*qPitch ;
			// 步骤5：将四元数转为旋转矩阵（模型的最终旋转矩阵）
			Matrix modelRotMatrix = qTotal.toMatrix();

			mainCameraController->updateRotation(modelRotMatrix);
		}
		else
		{
			// 未锁定鼠标时，保持初始视角
			/*Matrix p = Matrix::perspective(0.01f, 10000.0f, (float)WIDTH / HEIGHT, 45.0f);
			Matrix v = Matrix::lookAt(cameraPos, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
			gm->viewProjMatrix = v * p;*/
		}
		
		// draw
		myWorld->ExecuteDraw();
		//SkySphere.draw(&core, myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
		

		//animMesh.drawSingle(&core, myWorld->GetPSOManager(), ANIM_PIPE, myWorld->GetPipelines(), &animatedInstance, myWorld->GetDeltatime());
		
		
		core.finishFrame();
		
		
		


		
	


	}
	
	core.flushGraphicsQueue();


	return 0;

}