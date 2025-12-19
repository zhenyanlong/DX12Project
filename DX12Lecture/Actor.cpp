#include "Actor.h"
#include "World.h"
#include "Animation/FPSAnimationStateMachine.h"
#include "Animation/EnemyAnimationStateMachine.h"
#define M_PI       3.14159265358979323846
SkyBoxActor::SkyBoxActor()
{
	World* myWorld = World::Get();
	skybox = new StaticMesh();
	skybox->CreateFromSphere(myWorld->GetCore(), 64, 64, 5, "Models/Textures/sky_ps.png");
	skybox->SetWorldScaling(Vec3(1000.f, 1000.f, 1000.f));
	skybox->SetWorldRotationRadian(Vec3(M_PI, 0.f, 0.f));
}

void SkyBoxActor::draw()
{
	World* myWorld = World::Get();
	skybox->drawSingle(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
}

TreeActor::TreeActor(int count)
{
	instanceCount = count;
	World* myWorld = World::Get();
	willow = new StaticMesh(myWorld->GetCore(), "Models/willow.gem");
	//willow->SetWorldScaling(Vec3(1.f, 1.f, 1.f));
	// 生成50个实例的矩阵（分布在半径50的圆内，高度0~5）
	generateInstanceMatrices(instanceCount, 50.0f, 5.0f);
}

void TreeActor::generateInstanceMatrices(int count, float radius, float height)
{
	instanceMatrices.clear();
	//for (int i = 0; i < count; i++)
	//{
	//	// 随机极坐标位置
	//	float angle = (float)rand() / RAND_MAX * 2 * M_PI;
	//	float r = (float)rand() / RAND_MAX * radius;
	//	float x = r * cos(angle);
	//	float z = r * sin(angle);
	//	float y = (float)rand() / RAND_MAX * height;

	//	// 随机缩放（0.8~1.2倍）
	//	float scale = 0.01f;

	//	// 随机旋转（Y轴）
	//	float rotY = (float)rand() / RAND_MAX * 2 * M_PI;

	//	// 构建实例的世界矩阵：缩放 -> 旋转 -> 平移
	//	Matrix scaleMat = Matrix::scaling(Vec3(scale, scale, scale));
	//	Matrix rotMat = Matrix::rotateZ(0.f)*Matrix::rotateY(rotY)* Matrix::rotateX(0.f);
	//	Matrix transMat = Matrix::translation(Vec3(x, y, z));
	//	Matrix instanceMat = scaleMat* rotMat * transMat;

	//	instanceMatrices.push_back(instanceMat);
	//}
	for (int i = 0; i < count; i++)
	{
		float scale = 0.05f;
		float rotY = (float)rand() / RAND_MAX * 2 * M_PI;
		float transIncrement = 20.f;
		Vec3 originPos = willow->GetWorldPos();

		Matrix scaleMat = Matrix::scaling(Vec3(scale, scale, scale));
		Matrix rotMat = Matrix::rotateZ(0.f) * Matrix::rotateY(rotY) * Matrix::rotateX(0.f);
		Matrix transMat = Matrix::translation(Vec3(originPos.x, originPos.y, originPos.z + i * transIncrement));
		Matrix instanceMat = scaleMat * rotMat * transMat;

		instanceMatrices.push_back(instanceMat);
	}
}

void TreeActor::draw()
{
	World* myWorld = World::Get();
	
	//willow->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
	willow->drawInstances(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_INSTANCE_LIGHT_PIPE, myWorld->GetPipelines(), &instanceMatrices, instanceCount);
}

WaterActor::WaterActor()
{
	World* myWorld = World::Get();
	water = new StaticMesh();
	water->CreateFromPlane(myWorld->GetCore(),100000, 100000,2000,2000);
}

void WaterActor::draw()
{
	World* myWorld = World::Get();
	water->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_LIGHT_WATER_PIPE, myWorld->GetPipelines());
}

FPSActor::FPSActor()
{
	World* myWorld = World::Get();
	fps_Mesh = new AnimatedModel(myWorld->GetCore(), "Models/Uzi.gem");
	fps_Mesh->SetWorldScaling(Vec3(0.1f, 0.1f, 0.1f));
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::Sphere);
	animatedInstance = new AnimationInstance();
	animatedInstance->init(&fps_Mesh->animation, 0);
	m_actorType = ActorType::Player;
	calculateLocalCollisionShape();

	// 新增：初始化动画状态机
	animStateMachine = new FPSAnimationStateMachine(fps_Mesh, animatedInstance);
}

FPSActor::~FPSActor()
{
	delete animStateMachine;
	delete animatedInstance;
	delete fps_Mesh;
}

void FPSActor::draw()
{
	World* myWorld = World::Get();
	fps_Mesh->drawSingle(myWorld->GetCore(), myWorld->GetPSOManager(), ANIM_PIPE, myWorld->GetPipelines(), animatedInstance, myWorld->GetDeltatime(), animStateMachine);
}

void FPSActor::updatePos(Vec3 pos)
{
	fps_Mesh->SetWorldPos(pos);
}

void FPSActor::updateRotation(Vec3 rot)
{
	fps_Mesh->SetWorldRotationRadian(rot);
}

void FPSActor::updateRotation(Matrix rotMat)
{
	fps_Mesh->SetRotationMatrix(rotMat);
}

void FPSActor::updateWorldMatrix(Vec3 pos, float yaw, float pitch)
{
	/*Matrix rotation = Matrix::rotateY(m_yaw) * Matrix::rotateX(m_pitch);
	return Matrix::translation(m_position) * rotation;*/
}

void FPSActor::calculateLocalCollisionShape()
{
	// 从Mesh计算局部碰撞体（示例：用Mesh顶点扩展AABB/Sphere）
	if (!fps_Mesh)
		return;

	// 遍历Mesh顶点，扩展局部AABB和Sphere
	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto* mesh : fps_Mesh->meshes)
	{
		// 假设Mesh有获取顶点的方法（需根据实际代码调整）
		auto vertices = mesh->getVertices(); // 自定义方法，返回std::vector<Vec3>
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	// 调整Sphere中心（与AABB中心一致）
	m_localSphere.centre = m_localAABB.getCenter();
}

void FPSActor::OnBeginPlay()
{
}

void FPSActor::OnTick(float dt)
{
	if (animStateMachine) {
		animStateMachine->Update(dt); // 调用状态机更新
	}
}

BoxActor::BoxActor()
{
	World* myWorld = World::Get();
	box = new StaticMesh(myWorld->GetCore(), "Models/box_024.gem");
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::AABB);
	box->SetWorldScaling(Vec3(0.1f, 0.1f, 0.1f));
	//box->SetWorldScaling(Vec3(1.1f, 1.1f, 1.1f));
	calculateLocalCollisionShape();
}

void BoxActor::draw()
{
	World* myWorld = World::Get();
	box->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
}

void BoxActor::calculateLocalCollisionShape()
{
	// 从Mesh计算局部碰撞体（示例：用Mesh顶点扩展AABB/Sphere）
	if (!box)
		return;

	// 遍历Mesh顶点，扩展局部AABB和Sphere
	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto mesh : box->meshes)
	{
		// 假设Mesh有获取顶点的方法（需根据实际代码调整）
		auto vertices = mesh.getVertices(); // 自定义方法，返回std::vector<Vec3>
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	// 调整Sphere中心（与AABB中心一致）
	m_localSphere.centre = m_localAABB.getCenter();
}

GroundActor::GroundActor()
{
	World* myWorld = World::Get();
	ground = new StaticMesh();
	ground->CreateFromPlane(myWorld->GetCore(), 10000, 20000, 10, 10);
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::AABB);
	calculateLocalCollisionShape();
}

void GroundActor::draw()
{
	World* myWorld = World::Get();
	
	ground->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
}

void GroundActor::calculateLocalCollisionShape()
{
	// 从Mesh计算局部碰撞体（示例：用Mesh顶点扩展AABB/Sphere）
	if (!ground)
		return;

	// 遍历Mesh顶点，扩展局部AABB和Sphere
	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto mesh : ground->meshes)
	{
		// 假设Mesh有获取顶点的方法（需根据实际代码调整）
		auto vertices = mesh.getVertices(); // 自定义方法，返回std::vector<Vec3>
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	// 调整Sphere中心（与AABB中心一致）
	m_localSphere.centre = m_localAABB.getCenter();
}

ContainerBlueActor::ContainerBlueActor()
{
	World* myWorld = World::Get();
	container = new StaticMesh(myWorld->GetCore(), "Models/container_005.gem");
	//container->setWorldRotation(Vec3(0.f, PI / 2, 0.f));
	container->SetWorldRotationRadian(Vec3(0.f, PI / 2, 0.f));
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::AABB);
	calculateLocalCollisionShape();
}

void ContainerBlueActor::draw()
{
	World* myWorld = World::Get();

	container->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
}

void ContainerBlueActor::calculateLocalCollisionShape()
{
	// 从Mesh计算局部碰撞体（示例：用Mesh顶点扩展AABB/Sphere）
	if (!container)
		return;

	// 遍历Mesh顶点，扩展局部AABB和Sphere
	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto mesh : container->meshes)
	{
		// 假设Mesh有获取顶点的方法（需根据实际代码调整）
		auto vertices = mesh.getVertices(); // 自定义方法，返回std::vector<Vec3>
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	// 调整Sphere中心（与AABB中心一致）
	m_localSphere.centre = m_localAABB.getCenter();
}

BlockActor::BlockActor()
{
	World* myWorld = World::Get();
	box = new StaticMesh(myWorld->GetCore(), "Models/box_024.gem");
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::AABB);
	box->SetWorldScaling(Vec3(0.1f, 0.1f, 0.1f));
	//box->SetWorldScaling(Vec3(1.1f, 1.1f, 1.1f));
	calculateLocalCollisionShape();
}

void BlockActor::draw()
{
	World* myWorld = World::Get();

	//box->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
}

void BlockActor::calculateLocalCollisionShape()
{
	// 从Mesh计算局部碰撞体（示例：用Mesh顶点扩展AABB/Sphere）
	if (!box)
		return;

	// 遍历Mesh顶点，扩展局部AABB和Sphere
	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto mesh : box->meshes)
	{
		// 假设Mesh有获取顶点的方法（需根据实际代码调整）
		auto vertices = mesh.getVertices(); // 自定义方法，返回std::vector<Vec3>
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	// 调整Sphere中心（与AABB中心一致）
	m_localSphere.centre = m_localAABB.getCenter();
}

ObstacleActor::ObstacleActor(int count)
{
	World* myWorld = World::Get();
	obstacle = new StaticMesh(myWorld->GetCore(), "Models/obstacle_003.gem");
	//setCollidable(true);
	//setCollisionShapeType(CollisionShapeType::AABB);
	obstacle->SetWorldScaling(Vec3(0.1f, 0.1f, 0.1f));
	//box->SetWorldScaling(Vec3(1.1f, 1.1f, 1.1f));
	//calculateLocalCollisionShape();
	generateInstanceMatrices(count, Vec3(0.f, 0.f, 5.f));
	
}

void ObstacleActor::generateInstanceMatrices(int count, Vec3 offset)
{
	instanceCount = count;
	instanceMatrices.clear();
	
	for (int i = 0; i < count; i++)
	{
		Vec3 scale = obstacle->GetWorldScale();
		Vec3 rot = obstacle->GetWorldRotationRadian();
		//float rotY = (float)rand() / RAND_MAX * 2 * M_PI;
		float transIncrement = 20.f;
		Vec3 originPos = obstacle->GetWorldPos();

		Matrix scaleMat = Matrix::scaling(Vec3(scale.x, scale.y, scale.z));
		Matrix rotMat = Matrix::rotateZ(rot.z) * Matrix::rotateY(rot.y) * Matrix::rotateX(rot.x);
		Matrix transMat = Matrix::translation(Vec3(originPos.x + i * offset.x, originPos.y + i * offset.y, originPos.z + i * offset.z));
		Matrix instanceMat = scaleMat * rotMat * transMat;

		instanceMatrices.push_back(instanceMat);
	}
}

void ObstacleActor::draw()
{	
	World* myWorld = World::Get();

	obstacle->drawInstances(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_INSTANCE_LIGHT_PIPE, myWorld->GetPipelines(), &instanceMatrices, instanceCount);
}

void ObstacleActor::calculateLocalCollisionShape()
{
}

GeneralMeshActor::GeneralMeshActor(std::string path)
{
	World* myWorld = World::Get();
	mesh = new StaticMesh(myWorld->GetCore(), path);
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::AABB);
	mesh->SetWorldScaling(Vec3(0.1f, 0.1f, 0.1f));
	//box->SetWorldScaling(Vec3(1.1f, 1.1f, 1.1f));
	calculateLocalCollisionShape();
}

void GeneralMeshActor::draw()
{
	World* myWorld = World::Get();
	mesh->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
}

void GeneralMeshActor::calculateLocalCollisionShape()
{
	// 从Mesh计算局部碰撞体（示例：用Mesh顶点扩展AABB/Sphere）
	if (!mesh)
		return;

	// 遍历Mesh顶点，扩展局部AABB和Sphere
	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto mesh : mesh->meshes)
	{
		// 假设Mesh有获取顶点的方法（需根据实际代码调整）
		auto vertices = mesh.getVertices(); // 自定义方法，返回std::vector<Vec3>
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	// 调整Sphere中心（与AABB中心一致）
	m_localSphere.centre = m_localAABB.getCenter();
}

void BulletActor::OnTick(float dt)
{
	World* myWorld = World::Get();
	// 1. 更新生命周期，超时销毁
	m_lifeTime += dt;
	if (m_lifeTime >= MAX_LIFE_TIME) {
		Destroy();
		return;
	}

	// 2. 移动子弹
	setWorldPos(getWorldPos() + m_direction * m_speed * dt);

	calculateLocalCollisionShape();
	auto collidableActors = myWorld->getCollidableActors();
	std::vector<Actor*> collisions = CollisionResolver::CheckCollision(this, collidableActors);

	for (auto* actor : collisions)
	{
		if (actor->getActorType() == ActorType::Static)
		{
			Destroy();
		}
		else if (actor->getActorType() == ActorType::Enemy)
		{
			// execute damage api
			dynamic_cast<EnemyActor*>(actor)->animStateMachine->TriggerDeath();

		}
	}
}

BulletActor::BulletActor(const Vec3 pos, const Vec3 dir, float speed, int damage)
{
	World* myWorld = World::Get();
	m_bulletMesh = new StaticMesh();
	m_bulletMesh->CreateFromSphere(myWorld->GetCore(), 64, 64, 10, "Models/Textures/arms_1_Albedo_nh.png");
	// init mesh 
	setWorldPos(pos);
	//setWorldScale(Vec3(1.f, 1.f, 1.f));
	m_direction = dir.normalize();
	m_speed = speed;
	m_damage = damage;
	m_lifeTime = 0.0f;
	m_actorType = ActorType::Bullet;

	
	
	

	// init collision
	//setCollidable(true);
	setCollisionShapeType(CollisionShapeType::Sphere);
	calculateLocalCollisionShape();
}

void BulletActor::draw()
{
	World* myWorld = World::Get();
	m_bulletMesh->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
}

void BulletActor::calculateLocalCollisionShape()
{
	// 从Mesh计算局部碰撞体（示例：用Mesh顶点扩展AABB/Sphere）
	if (!m_bulletMesh)
		return;

	// 遍历Mesh顶点，扩展局部AABB和Sphere
	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto mesh : m_bulletMesh->meshes)
	{
		// 假设Mesh有获取顶点的方法（需根据实际代码调整）
		auto vertices = mesh.getVertices(); // 自定义方法，返回std::vector<Vec3>
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	// 调整Sphere中心（与AABB中心一致）
	m_localSphere.centre = m_localAABB.getCenter();
}

EnemyActor::EnemyActor()
{
	World* myWorld = World::Get();
	enemy_Mesh = new AnimatedModel(myWorld->GetCore(), "Models/Duck-white.gem");
	enemy_Mesh->SetWorldScaling(Vec3(0.1f, 0.1f, 0.1f));
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::Sphere);
	animatedInstance = new AnimationInstance();
	animatedInstance->init(&enemy_Mesh->animation, 0);
	m_actorType = ActorType::Enemy;
	calculateLocalCollisionShape();

	// 新增：初始化动画状态机
	animStateMachine = new EnemyAnimationStateMachine(enemy_Mesh, animatedInstance);
}

EnemyActor::~EnemyActor()
{
	delete animStateMachine;
	delete animatedInstance;
	delete enemy_Mesh;
}

void EnemyActor::draw()
{
	World* myWorld = World::Get();
	enemy_Mesh->drawSingle(myWorld->GetCore(), myWorld->GetPSOManager(), ANIM_PIPE, myWorld->GetPipelines(), animatedInstance, myWorld->GetDeltatime(), animStateMachine);
}

void EnemyActor::calculateLocalCollisionShape()
{
	// 从Mesh计算局部碰撞体（示例：用Mesh顶点扩展AABB/Sphere）
	if (!enemy_Mesh)
		return;

	// 遍历Mesh顶点，扩展局部AABB和Sphere
	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto* mesh : enemy_Mesh->meshes)
	{
		// 假设Mesh有获取顶点的方法（需根据实际代码调整）
		auto vertices = mesh->getVertices(); // 自定义方法，返回std::vector<Vec3>
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	// 调整Sphere中心（与AABB中心一致）
	m_localSphere.centre = m_localAABB.getCenter();
}

void EnemyActor::OnBeginPlay()
{
}

void EnemyActor::OnTick(float dt)
{
	if (animStateMachine) {
		animStateMachine->Update(dt); // 调用状态机更新
	}
	if (animStateMachine->IsDeathFinished())
	{
		Destroy();
	}
}
