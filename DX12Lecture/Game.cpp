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

// ===== 新增：重力与跳跃相关常量 =====
const float GRAVITY = 98.0f;              // 重力加速度（游戏中放大，更有手感，实际9.8）
const float GROUND_THRESHOLD = 0.01f;     // 落地检测的垂直阈值（避免微小抖动）
const float JUMP_FORCE = 30.0f;           // 跳跃力度（可选）
const float GROUND_ADJUST = 0.01f;        // 地面吸附的微小偏移（防止悬浮）
bool IsGravityMode = true;


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
	//Vec3 cameraPos = Vec3(40.0f, 15.0f, 0.0f);  // 相机初始位置（FPS中位置由WASD控制）
	//Vec3 lastCameraPos = cameraPos;
	float cameraYaw = -M_PI / 2;                  // 偏航角（初始朝向Z轴负方向，对应FPS初始向前）
	float cameraPitch = 0.0f;                   // 俯仰角（初始水平）
	bool mouseLocked = LOCK_MOUSE_ON_START;     // 鼠标是否锁定
	POINT windowCenter;                         // 窗口中心坐标（用于鼠标重置）

	// ===== 新增：重力与落地状态变量 =====
	float verticalVelocity = 0.0f; // 垂直速度（Y轴，向上为正）
	bool isGrounded = true;       // 是否落地（站在地面上）
	bool firstframe = true;

	
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
	if (mainCameraController)
	{
		mainCameraController->updatePos(Vec3(40.0f, 15.0f, 0.0f));
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
			Vec3 moveForward;
			cameraForward.x = cos(cameraYaw) * cos(cameraPitch);
			cameraForward.y = sin(cameraPitch);
			//cameraForward.y = 0.f;
			cameraForward.z = sin(cameraYaw) * cos(cameraPitch);
			cameraForward = cameraForward.normalize();

			moveForward.x = cos(cameraYaw); // 无俯仰角影响的水平向前
			moveForward.y = 0.0f;
			moveForward.z = sin(cameraYaw);
			moveForward = moveForward.normalize();
			/*if (IsGravityMode)
			{
				cameraForward.y = 0.f;
			}*/
			// 计算相机右方向（用于WASD横向移动）
			Vec3 cameraRight = Cross(cameraForward, Vec3(0.0f, 1.0f, 0.0f)).normalize();
			Vec3 cameraUp = Vec3(0.0f, 1.0f, 0.0f); // FPS固定上方向为世界Y轴

			Vec3 moveRight = Cross(moveForward, Vec3(0.0f, 1.0f, 0.0f)).normalize();
			// ===== 重构：先计算输入的移动向量（不直接修改位置）=====
			Vec3 desiredMove = Vec3(0, 0, 0);
			Vec3 gravityMove = Vec3(0.f, 0.f, 0.f);
			if (IsGravityMode)
			{
				

				if (win.keys['W']) desiredMove += moveForward * CAMERA_MOVE_SPEED * dt;
				if (win.keys['S']) desiredMove -= moveForward * CAMERA_MOVE_SPEED * dt;
				if (win.keys['A']) desiredMove += moveRight * CAMERA_MOVE_SPEED * dt;
				if (win.keys['D']) desiredMove -= moveRight * CAMERA_MOVE_SPEED * dt;
				
				desiredMove.y = 0.f;
				//desiredMove.normalize();
				//if (win.keys['Q']) desiredMove.y -= CAMERA_MOVE_SPEED * dt;
				//if (win.keys['E']) desiredMove.y += CAMERA_MOVE_SPEED * dt;
				if (win.keys['F']) mainActor->setWorldPos(Vec3(40.f,15.f,0.f));
				// ===== 步骤2：应用重力与跳跃逻辑 =====
			// 1. 重力：仅当不在地面时，垂直速度叠加重力加速度（向下为负）
				//if (!isGrounded)
				//{
				if (!firstframe)
				{
					verticalVelocity -= GRAVITY * dt; // Y轴向上，重力向下，所以减
					
				}
				else
				{
					firstframe = false;
				}
					
				//}

				// 2. 跳跃：仅当落地时，按空格触发（可选）
				if (win.keys[VK_SPACE] && win.keyJustPressed[VK_SPACE] && isGrounded)
				{
					verticalVelocity = JUMP_FORCE; // 给向上的力
					isGrounded = false;            // 离开地面
					//win.keyJustPressed[VK_SPACE] = false; // 防止连续跳跃
				}

				// 3. 合并垂直移动（重力/跳跃）到总移动向量
				gravityMove = Vec3(0, verticalVelocity*dt , 0);
				desiredMove += gravityMove;
			}
			else
			{
				

				if (win.keys['W']) desiredMove += cameraForward * CAMERA_MOVE_SPEED * dt;
				if (win.keys['S']) desiredMove -= cameraForward * CAMERA_MOVE_SPEED * dt;
				if (win.keys['A']) desiredMove += cameraRight * CAMERA_MOVE_SPEED * dt;
				if (win.keys['D']) desiredMove -= cameraRight * CAMERA_MOVE_SPEED * dt;
				if (win.keys['Q']) desiredMove.y -= CAMERA_MOVE_SPEED * dt;
				if (win.keys['E']) desiredMove.y += CAMERA_MOVE_SPEED * dt;
			}
			
			
			
			// ===== 碰撞检测：获取可移动的向量 =====
			auto collidableActors = myWorld->getCollidableActors();
			Vec3 resolvedMove = CollisionResolver::resolveSlidingCollision(mainActor, desiredMove, collidableActors, 0.01f);

			if (IsGravityMode)
			{
				// ===== 步骤4：处理落地逻辑（核心）=====
				//isGrounded = false; // 先默认未落地
				// 判断条件：期望的垂直移动是向下（gravityMove.y < 0），且实际垂直移动远小于期望（被地面阻挡）
				float verticalDesired = gravityMove.y; // 期望的垂直移动（向下为负）
				float verticalResolved = resolvedMove.y; // 实际的垂直移动
				if (verticalDesired < -GROUND_THRESHOLD && abs(verticalResolved - verticalDesired) > GROUND_THRESHOLD)
				{
					// 落地：重置垂直速度，标记为落地
					verticalVelocity = 0.0f;
					isGrounded = true;

					// 地面吸附：轻微上移Actor，确保稳定站在地面（避免悬浮/抖动）
					Vec3 currentPos = mainActor->getWorldPos();
					mainActor->setWorldPos(Vec3(currentPos.x, currentPos.y, currentPos.z));
				}
				// 可选：处理撞到天花板（垂直移动向上，被阻挡）
				else if (verticalDesired > GROUND_THRESHOLD && abs(verticalResolved - verticalDesired) > GROUND_THRESHOLD)
				{
					verticalVelocity = 0.0f; // 重置垂直速度，停止上升
				}
				else
				{
					// ===== 应用移动到 mainActor =====
					Vec3 currentPos = mainActor->getWorldPos();
					mainActor->setWorldPos(currentPos + resolvedMove);
				}
			}
			else
			{
				// ===== 应用移动到 mainActor =====
				Vec3 currentPos = mainActor->getWorldPos();
				mainActor->setWorldPos(currentPos + resolvedMove);
			}
			

			// ===== 从 mainActor 获取相机位置 =====
			Vec3 cameraPos = mainActor->getWorldPos();

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
		// 新增：检测移动状态（WASD是否按下）
		bool isMoving = win.keys['W'] || win.keys['S'] || win.keys['A'] || win.keys['D'];
		FPSActor* fpsActor = dynamic_cast<FPSActor*>(mainActor);
		FPSAnimationStateMachine* fpsAnimation = fpsActor->animStateMachine;
		if (fpsAnimation) {
			fpsAnimation->SetMoving(isMoving); // 设置移动状态（控制Idle/Walk切换）
			// 检测换弹（R键）
			if (win.keys['R'] ) {
				fpsAnimation->TriggerReload();
				//win.keyJustPressed['R'] = false;
			}
			// 检测射击（鼠标左键）
			if (win.keys[VK_LBUTTON] && win.keyJustPressed[VK_LBUTTON]) {
				fpsAnimation->TriggerFire();
				//win.keyJustPressed[VK_LBUTTON] = false;
			}
		}

		fpsActor->Tick(myWorld->GetDeltatime());

		// draw
		myWorld->ExecuteDraw();
		//SkySphere.draw(&core, myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
		

		//animMesh.drawSingle(&core, myWorld->GetPSOManager(), ANIM_PIPE, myWorld->GetPipelines(), &animatedInstance, myWorld->GetDeltatime());
		
		
		core.finishFrame();
		
		
		


		
	


	}
	
	core.flushGraphicsQueue();


	return 0;

}