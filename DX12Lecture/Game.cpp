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
const float CAMERA_MOVE_SPEED = 15.0f;    
const float MOUSE_SENSITIVITY = 0.15f;    
const bool LOCK_MOUSE_ON_START = true;    

// Gravity Parameters
const float GRAVITY = 98.0f;              
const float GROUND_THRESHOLD = 0.01f;     
const float JUMP_FORCE = 30.0f;           
const float GROUND_ADJUST = 0.01f;        
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

// Level_path
const std::string LEVEL1_PATH = "level1.lvl";
const std::string LEVEL2_PATH = "level2.lvl"; // no exist
int currentLevel = 1; // current level index

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

	// Initial load TestMap and save it as Level 1
	auto initialLevel = std::make_shared<TestMap>();
	initialLevel->SaveLevel(LEVEL1_PATH); 
	myWorld->LoadNewLevel(initialLevel);

	Actor* mainActor = myWorld->GetLevel()->GetActor("FPSActor");
	CameraControllable* mainCameraController = dynamic_cast<CameraControllable*>(mainActor);

	ScreenSpaceTriangle tri;
	

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

	


	//Vec3 cameraPos = Vec3(40.0f, 15.0f, 0.0f);  
	//Vec3 lastCameraPos = cameraPos;
	float cameraYaw = -M_PI / 2;                  // Yaw
	float cameraPitch = 0.0f;                   // pitch
	bool mouseLocked = LOCK_MOUSE_ON_START;     
	POINT windowCenter;                         // window center position


	float verticalVelocity = 0.0f; 
	bool isGrounded = false;       
	bool firstframe = true;

	
	// calculate window center
	RECT windowRect;
	GetWindowRect(win.hwnd, &windowRect);
	windowCenter.x = windowRect.left + WIDTH / 2;
	windowCenter.y = windowRect.top + HEIGHT / 2;

	// lock mouse 
	if (mouseLocked)
	{
		ClipCursor(&windowRect);  
		ShowCursor(FALSE);        
		SetCursorPos(windowCenter.x, windowCenter.y); 
	}
	if (mainCameraController)
	{
		mainCameraController->updatePos(myWorld->GetLevel()->GetSpawnPoint());
	}
	
	
	while (true)
	{
		//Process messages 
		win.processMessages();

		// level switch
		
		if (win.keys['1'] && win.keyJustPressed['1']) {
			auto level1 = std::make_shared<TestMap>();
			if (level1->LoadLevel(LEVEL1_PATH)) {
				myWorld->LoadNewLevel(level1);
				
				mainActor = myWorld->GetLevel()->GetActor("FPSActor");
				mainCameraController = dynamic_cast<CameraControllable*>(mainActor);
				if (mainCameraController)
				{
					mainCameraController->updatePos(myWorld->GetLevel()->GetSpawnPoint());
				}
			}
			currentLevel = 1;
			
		}

		// switch to level 2 (haven't achieved)
		if (win.keys['2'] && win.keyJustPressed['2']) {
			auto level2 = std::make_shared<TestMap>();
			if (!level2->LoadLevel(LEVEL2_PATH)) {
				
				level2->SetSpawnPoint(Vec3(60.0f, 15.0f, 0.0f)); 
				level2->SaveLevel(LEVEL2_PATH);
			}
			myWorld->LoadNewLevel(level2);
			
			mainActor = myWorld->GetLevel()->GetActor("FPSActor");
			mainCameraController = dynamic_cast<CameraControllable*>(mainActor);
			if (mainCameraController)
			{
				mainCameraController->updatePos(myWorld->GetLevel()->GetSpawnPoint());
			}
			currentLevel = 2;
			
		}

		// update time
		myWorld->UpdateTime();
		float dt = myWorld->GetDeltatime();

		// garbage Collection 
		if (!firstframe)
		{
			myWorld->garbageCollection();
		}
		static bool testspawn = true;
		if (myWorld->GetLevel() != nullptr && testspawn)
		{
			Actor* bullet = new BulletActor(Vec3(0.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f), 0.f);
			myWorld->addActor("bullet", bullet);
			testspawn = false;
		}
		// Process mouse control
		GetWindowRect(win.hwnd, &windowRect);
		windowCenter.x = windowRect.left + WIDTH / 2;
		windowCenter.y = windowRect.top + HEIGHT / 2;
		// The ESC key toggles the mouse lock status
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
				ClipCursor(NULL); 
			}
			win.keyJustPressed[VK_ESCAPE] = false; 
		}



		Vec3 cameraForward;
		Vec3 cameraLeft;
		// begin play
		myWorld->ExecuteBeginPlays();
		// perspective control
		if (mouseLocked ) 
		{
			POINT currentMousePos;
			GetCursorPos(&currentMousePos);

			
			int mouseDeltaX = currentMousePos.x - windowCenter.x;
			int mouseDeltaY = currentMousePos.y - windowCenter.y;

			
			SetCursorPos(windowCenter.x, windowCenter.y);

			
			cameraYaw -= mouseDeltaX * MOUSE_SENSITIVITY * 0.0174533f; 
			cameraPitch -= mouseDeltaY * MOUSE_SENSITIVITY * 0.0174533f;

			
			cameraPitch = clamp(cameraPitch, (float) - M_PI / 2 + 0.01f, (float)M_PI / 2 - 0.01f);

			
			
			Vec3 moveForward;
			cameraForward.x = cos(cameraYaw) * cos(cameraPitch);
			cameraForward.y = sin(cameraPitch);
			
			cameraForward.z = sin(cameraYaw) * cos(cameraPitch);
			cameraForward = cameraForward.normalize();

			moveForward.x = cos(cameraYaw); 
			moveForward.y = 0.0f;
			moveForward.z = sin(cameraYaw);
			moveForward = moveForward.normalize();
			/*if (IsGravityMode)
			{
				cameraForward.y = 0.f;
			}*/
			// Calculate the left direction of the camera
			cameraLeft = Cross(cameraForward, Vec3(0.0f, 1.0f, 0.0f)).normalize();
			Vec3 cameraUp = Vec3(0.0f, 1.0f, 0.0f); 

			Vec3 moveRight = Cross(moveForward, Vec3(0.0f, 1.0f, 0.0f)).normalize();
			
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
				
				// add verticalVelocity for gravity
				if (!firstframe&& !isGrounded)
				{
					
					verticalVelocity -= GRAVITY * dt; 
					
				}
				else
				{
					firstframe = false;
				}
					
				//}

				// jump
				if (win.keys[VK_SPACE] && win.keyJustPressed[VK_SPACE] && isGrounded)
				{
					verticalVelocity = JUMP_FORCE; 
					isGrounded = false;           
					
				}

				// merge gravity move
				gravityMove = Vec3(0, verticalVelocity*dt , 0);
				desiredMove += gravityMove;
			}
			else
			{
				

				if (win.keys['W']) desiredMove += cameraForward * CAMERA_MOVE_SPEED * dt;
				if (win.keys['S']) desiredMove -= cameraForward * CAMERA_MOVE_SPEED * dt;
				if (win.keys['A']) desiredMove += cameraLeft * CAMERA_MOVE_SPEED * dt;
				if (win.keys['D']) desiredMove -= cameraLeft * CAMERA_MOVE_SPEED * dt;
				if (win.keys['Q']) desiredMove.y -= CAMERA_MOVE_SPEED * dt;
				if (win.keys['E']) desiredMove.y += CAMERA_MOVE_SPEED * dt;
			}
			
			
			
			// slide collision check
			auto collidableActors = myWorld->getCollidableActors();
			Vec3 resolvedMove = CollisionResolver::resolveSlidingCollision(mainActor, desiredMove, collidableActors, 0.01f);

			if (IsGravityMode)
			{
				// check if grounded
				float verticalDesired = gravityMove.y; 
				float verticalResolved = resolvedMove.y; 
				if (verticalDesired < -GROUND_THRESHOLD && abs(verticalResolved - verticalDesired) > GROUND_THRESHOLD)
				{
					
					verticalVelocity = 0.0f;
					isGrounded = true;

					
					Vec3 currentPos = mainActor->getWorldPos();
					mainActor->setWorldPos(Vec3(currentPos.x, currentPos.y, currentPos.z));
				}
				
				//Vec3 currentPos = mainActor->getWorldPos();
				//Vec3 newPos = currentPos + resolvedMove;
				//mainActor->setWorldPos(newPos); 

				
				//float verticalDesired = gravityMove.y; 
				//float verticalResolved = resolvedMove.y; 
				//isGrounded = false; 

				
				//bool isBlockedByGround = (verticalDesired < -GROUND_THRESHOLD) && (abs(verticalResolved - verticalDesired) > GROUND_THRESHOLD);
				
				//bool isRayHitGround = false;
				//if (mainActor)
				//{
			
				//	Vec3 rayOrigin = mainActor->getWorldPos() + Vec3(0, GROUND_THRESHOLD, 0); 
				//	Vec3 rayDir = Vec3(0, -10, 0); 
				//	Ray groundRay(rayOrigin, rayDir);
				//	float t; 
				//	AABB actorAABB = mainActor->getWorldAABB();

				
				//	for (auto* collActor : myWorld->getCollidableActors())
				//	{
				//		if (!collActor || collActor == mainActor || collActor->getActorType() != ActorType::Static)
				//			continue;

				//		if (collActor->getCollisionShapeType() == CollisionShapeType::AABB)
				//		{
				//			CollisionResult rayResult = CollisionDetector::checkRayAABB(groundRay, collActor->getWorldAABB(), t);
				//			if (rayResult.isColliding && t <= (GROUND_THRESHOLD * 2))
				//			{
				//				isRayHitGround = true;
				//				isGrounded = true;
				//				break;
				//			}
				//		}
				//		else if (collActor->getCollisionShapeType() == CollisionShapeType::Sphere)
				//		{
				//			CollisionResult rayResult = CollisionDetector::checkRaySphere(groundRay, collActor->getWorldSphere(), t);
				//			if (rayResult.isColliding && t <= (GROUND_THRESHOLD * 2))
				//			{
				//				isRayHitGround = true;
				//				isGrounded = true;
				//				break;
				//			}
				//		}
				//	}
				//}

				
				//if (isBlockedByGround || isRayHitGround)
				//{
				//	isGrounded = true;
				//	verticalVelocity = 0.0f; 

			
				//	if (isRayHitGround)
				//	{
				//		Vec3 pos = mainActor->getWorldPos();
				//		mainActor->setWorldPos(Vec3(pos.x, pos.y, pos.z));
				//	}
				//	else
				//	{
				//		Vec3 pos = mainActor->getWorldPos();
				//		mainActor->setWorldPos(Vec3(pos.x, pos.y, pos.z));
				//	}
				//}
				else if (verticalDesired > GROUND_THRESHOLD && abs(verticalResolved - verticalDesired) > GROUND_THRESHOLD)
				{
					verticalVelocity = 0.0f; 
				}
				else
				{
					
					Vec3 currentPos = mainActor->getWorldPos();
					mainActor->setWorldPos(currentPos + resolvedMove);
				}
			}
			else
			{
				
				Vec3 currentPos = mainActor->getWorldPos();
				mainActor->setWorldPos(currentPos + resolvedMove);
			}
			

			
			Vec3 cameraPos = mainActor->getWorldPos();

			// update view Projection matrix
			Matrix p = Matrix::perspective(0.01f, 10000.0f, (float)WIDTH / HEIGHT, 45.0f);
			Matrix v = Matrix::lookAt(cameraPos, cameraPos + cameraForward, cameraUp);
			gm->viewProjMatrix = v * p;

			mainCameraController->updatePos(cameraPos);

			// calculate rotation matrix
			float modelYaw = cameraYaw + M_PI / 2.0f;
			Quaternion qYaw = Quaternion::fromYRotation(-modelYaw);
			Quaternion qPitch = Quaternion::fromXRotation(cameraPitch);
			Quaternion qTotal = qYaw*qPitch ;
			Matrix modelRotMatrix = qTotal.toMatrix();

			

			mainCameraController->updateRotation(modelRotMatrix);
		}
		else
		{
			
			/*Matrix p = Matrix::perspective(0.01f, 10000.0f, (float)WIDTH / HEIGHT, 45.0f);
			Matrix v = Matrix::lookAt(cameraPos, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
			gm->viewProjMatrix = v * p;*/
		}
		// update move animation state
		bool isMoving = win.keys['W'] || win.keys['S'] || win.keys['A'] || win.keys['D'];
		FPSActor* fpsActor = dynamic_cast<FPSActor*>(mainActor);
		FPSAnimationStateMachine* fpsAnimation = fpsActor->animStateMachine;
		if (fpsAnimation) {
			fpsAnimation->SetMoving(isMoving); // set move state
			// reload
			if (win.keys['R'] ) {
				fpsAnimation->TriggerReload();
				
			}
			// shoot
			static bool testbool = true;
			static float gaptime = 0.2f;
			if (win.mouseButtons[0]) {
				fpsAnimation->TriggerFire();
				if (testbool)
				{
					Vec3 up = Cross(cameraLeft, cameraForward).normalize();
					myWorld->addActor("bullet1", new BulletActor(fpsActor->getWorldPos() + cameraForward * 2.f - cameraLeft * 0.4 - up * 0.6, cameraForward, 100.f));
					testbool = false;
				}
				
				
				
			}
			
			if (!testbool)
			{
				gaptime -= myWorld->GetDeltatime();
				if (gaptime<=0.f)
				{
					testbool = true;
					gaptime = 0.2f;
				}
			}
		}

		//fpsActor->Tick(myWorld->GetDeltatime());
		myWorld->ExecuteTicks();
		// draw
		core.beginFrame();

		core.getCommandList()->SetGraphicsRootSignature(core.rootSignature);
		core.getCommandList()->SetDescriptorHeaps(1, &core.srvHeap.heap);

		myWorld->ExecuteDraw();
		//SkySphere.draw(&core, myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
		

		//animMesh.drawSingle(&core, myWorld->GetPSOManager(), ANIM_PIPE, myWorld->GetPipelines(), &animatedInstance, myWorld->GetDeltatime());
		
		
		core.finishFrame();
		
		
		


		
	


	}
	
	core.flushGraphicsQueue();


	return 0;

}