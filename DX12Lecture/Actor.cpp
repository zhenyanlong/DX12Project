#include "Actor.h"
#include "World.h"
#include "Animation/FPSAnimationStateMachine.h"
#include "Animation/EnemyAnimationStateMachine.h"
#define M_PI       3.14159265358979323846

std::map<std::string, Actor::ActorCreator> Actor::m_actorCreators;

void Actor::RegisterActor(const std::string& className, ActorCreator creator) {
	m_actorCreators[className] = creator;
}

Actor* Actor::CreateActorByClassName(const std::string& className) {
	auto it = m_actorCreators.find(className);
	if (it != m_actorCreators.end()) {
		return it->second();
	}
	return nullptr;
}

// 基类公共数据序列化
void Actor::SaveBase(std::ofstream& file) const {
	// 保存ActorType（转为int）
	int actorType = static_cast<int>(m_actorType);
	file.write(reinterpret_cast<const char*>(&actorType), sizeof(int));

	// 保存位置、旋转、缩放
	Vec3 pos = getWorldPos();
	file.write(reinterpret_cast<const char*>(&pos), sizeof(Vec3));
	Vec3 rot = getWorldRotation();
	file.write(reinterpret_cast<const char*>(&rot), sizeof(Vec3));
	Vec3 scale = getWorldScale();
	file.write(reinterpret_cast<const char*>(&scale), sizeof(Vec3));

	// 保存碰撞相关
	bool isCollidable = m_isCollidable;
	file.write(reinterpret_cast<const char*>(&isCollidable), sizeof(bool));
	int collisionShapeType = static_cast<int>(m_collisionShapeType);
	file.write(reinterpret_cast<const char*>(&collisionShapeType), sizeof(int));

	// 保存销毁状态
	bool isDestroyed = m_isDestroyed;
	file.write(reinterpret_cast<const char*>(&isDestroyed), sizeof(bool));
}

void Actor::LoadBase(std::ifstream& file) {
	// 加载ActorType
	int actorType;
	file.read(reinterpret_cast<char*>(&actorType), sizeof(int));
	m_actorType = static_cast<ActorType>(actorType);

	// 加载位置、旋转、缩放
	Vec3 pos, rot, scale;
	file.read(reinterpret_cast<char*>(&pos), sizeof(Vec3));
	setWorldPos(pos);
	file.read(reinterpret_cast<char*>(&rot), sizeof(Vec3));
	setWorldRotation(rot);
	file.read(reinterpret_cast<char*>(&scale), sizeof(Vec3));
	setWorldScale(scale);

	// 加载碰撞相关
	bool isCollidable;
	file.read(reinterpret_cast<char*>(&isCollidable), sizeof(bool));
	m_isCollidable = isCollidable;
	int collisionShapeType;
	file.read(reinterpret_cast<char*>(&collisionShapeType), sizeof(int));
	m_collisionShapeType = static_cast<CollisionShapeType>(collisionShapeType);

	// 加载销毁状态
	bool isDestroyed;
	file.read(reinterpret_cast<char*>(&isDestroyed), sizeof(bool));
	m_isDestroyed = isDestroyed;
}
namespace {
	struct SkyBoxActorRegistrar {
		SkyBoxActorRegistrar() {
			Actor::RegisterActor("SkyBoxActor", []() { return new SkyBoxActor(); });
		}
	} skyBoxActorRegistrar;
}

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

namespace {
	struct TreeActorRegistrar {
		TreeActorRegistrar() {
			Actor::RegisterActor("TreeActor", []() { return new TreeActor(); });
		}
	} treeActorRegistrar;
}
//TreeActor::TreeActor()
//{
//	m_instanceCount = 0;
//	m_transIncrement = Vec3(0.f,0.f,0.f);
//	World* myWorld = World::Get();
//	willow = new StaticMesh(myWorld->GetCore(), "Models/willow.gem");
//}
TreeActor::TreeActor(int count, Vec3 transIncrement)
{
	m_instanceCount = count;
	m_transIncrement = transIncrement;

	World* myWorld = World::Get();
	willow = new StaticMesh(myWorld->GetCore(), "Models/willow.gem");
	//willow->SetWorldScaling(Vec3(1.f, 1.f, 1.f));
	// 生成50个实例的矩阵（分布在半径50的圆内，高度0~5）
	generateInstanceMatrices(m_instanceCount, m_transIncrement);
}

void TreeActor::generateInstanceMatrices(int count, Vec3 transIncrement)
{
	instanceMatrices.clear();
	
	m_instanceCount = count;
	m_transIncrement = transIncrement;
	for (int i = 0; i < m_instanceCount; i++)
	{
		float scale = 0.05f;
		float rotY = (float)rand() / RAND_MAX * 2 * M_PI;
		//float transIncrement = 20.f;
		Vec3 originPos = willow->GetWorldPos();

		Matrix scaleMat = Matrix::scaling(Vec3(scale, scale, scale));
		Matrix rotMat = Matrix::rotateZ(0.f) * Matrix::rotateY(rotY) * Matrix::rotateX(0.f);
		Matrix transMat = Matrix::translation(Vec3(originPos.x + i * transIncrement.x, originPos.y + i * transIncrement.y, originPos.z + i * transIncrement.z));
		Matrix instanceMat = scaleMat * rotMat * transMat;

		instanceMatrices.push_back(instanceMat);
	}
}

void TreeActor::draw()
{
	World* myWorld = World::Get();
	
	//willow->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_PIPE, myWorld->GetPipelines());
	willow->drawInstances(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_INSTANCE_LIGHT_PIPE, myWorld->GetPipelines(), &instanceMatrices, m_instanceCount);
}

namespace {
	struct WaterActorRegistrar {
		WaterActorRegistrar() {
			Actor::RegisterActor("WaterActor", []() { return new WaterActor(); });
		}
	} waterActorRegistrar;
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

namespace {
	struct FPSActorRegistrar {
		FPSActorRegistrar() {
			Actor::RegisterActor("FPSActor", []() { return new FPSActor(); });
		}
	} fpsActorRegistrar;
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
	fps_Mesh->drawSingle(myWorld->GetCore(), myWorld->GetPSOManager(), ANIM_LIGHT_PIPE, myWorld->GetPipelines(), animatedInstance, myWorld->GetDeltatime(), animStateMachine);
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

namespace {
	struct BoxActorRegistrar {
		BoxActorRegistrar() {
			Actor::RegisterActor("BoxActor", []() { return new BoxActor(); });
		}
	} boxActorRegistrar;
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

namespace {
	struct GroundActorRegistrar {
		GroundActorRegistrar() {
			Actor::RegisterActor("GroundActor", []() { return new GroundActor(); });
		}
	} groundActorRegistrar;
}

GroundActor::GroundActor()
{
	World* myWorld = World::Get();
	ground = new StaticMesh();
	ground->CreateFromPlane(myWorld->GetCore(), 10000, 20000, 10, 10,"Models/Textures/concrete_floor_damaged_01_diff_1k.png","Models/Textures/concrete_floor_damaged_01_nor_dx_1k.png");
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::AABB);
	calculateLocalCollisionShape();
}

void GroundActor::draw()
{
	World* myWorld = World::Get();
	
	ground->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_LIGHT_PIPE, myWorld->GetPipelines());
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

namespace {
	struct ContainerBlueActorRegistrar {
		ContainerBlueActorRegistrar() {
			Actor::RegisterActor("ContainerBlueActor", []() { return new ContainerBlueActor(); });
		}
	} containerBlueActorRegistrar;
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

namespace {
	struct BlockActorRegistrar {
		BlockActorRegistrar() {
			Actor::RegisterActor("BlockActor", []() { return new BlockActor(); });
		}
	} blockActorRegistrar;
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

namespace {
	struct ObstacleActorRegistrar {
		ObstacleActorRegistrar() {
			Actor::RegisterActor("ObstacleActor", []() { return new ObstacleActor(); });
		}
	} obstacleActorRegistrar;
}

ObstacleActor::ObstacleActor(int count, Vec3 offset)
{
	m_instanceCount = count;
	m_offset = offset;

	World* myWorld = World::Get();
	obstacle = new StaticMesh(myWorld->GetCore(), "Models/obstacle_003.gem");
	//setCollidable(true);
	//setCollisionShapeType(CollisionShapeType::AABB);
	obstacle->SetWorldScaling(Vec3(0.1f, 0.1f, 0.1f));
	//box->SetWorldScaling(Vec3(1.1f, 1.1f, 1.1f));
	//calculateLocalCollisionShape();
	generateInstanceMatrices(m_instanceCount, m_offset);
	
}

void ObstacleActor::generateInstanceMatrices(int count, Vec3 offset)
{
	m_instanceCount = count;
	m_offset = offset;
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

	obstacle->drawInstances(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_INSTANCE_LIGHT_PIPE, myWorld->GetPipelines(), &instanceMatrices, m_instanceCount);
}

void ObstacleActor::calculateLocalCollisionShape()
{
}

namespace {
	// 注册GeneralMeshActor到Actor工厂
	struct GeneralMeshActorRegistrar {
		GeneralMeshActorRegistrar() {
			Actor::RegisterActor("GeneralMeshActor", []() {
				// 创建无参实例（加载时会调用Load方法初始化路径）
				return new GeneralMeshActor();
				});
		}
	} generalMeshActorRegistrar; // 全局变量，程序启动时自动执行构造函数
}

GeneralMeshActor::GeneralMeshActor(std::string path)
{
	initMesh(path);
}

void GeneralMeshActor::draw()
{
	World* myWorld = World::Get();
	mesh->draw(myWorld->GetCore(), myWorld->GetPSOManager(), STATIC_LIGHT_PIPE, myWorld->GetPipelines());
}

void GeneralMeshActor::calculateLocalCollisionShape()
{
	// 从Mesh计算局部碰撞体（示例：用Mesh顶点扩展AABB/Sphere）
	if (!mesh)
		return;

	// 遍历Mesh顶点，扩展局部AABB和Sphere
	m_localAABB.reset();
	m_localSphere = Sphere(Vec3(0, 0, 0), 0.0f);

	for (auto submesh : mesh->meshes)
	{
		// 假设Mesh有获取顶点的方法（需根据实际代码调整）
		auto vertices = submesh.getVertices(); // 自定义方法，返回std::vector<Vec3>
		for (const auto& v : vertices)
		{
			m_localAABB.extend(v);
			m_localSphere.extend(v);
		}
	}

	// 调整Sphere中心（与AABB中心一致）
	m_localSphere.centre = m_localAABB.getCenter();
}

void GeneralMeshActor::initMesh(const std::string& path)
{
	m_path = path;

	// 释放旧的mesh（防止内存泄漏）
	if (mesh) {
		delete mesh;
		mesh = nullptr;
	}

	// 重新创建mesh
	World* myWorld = World::Get();
	mesh = new StaticMesh(myWorld->GetCore(), path);
	setCollidable(true);
	setCollisionShapeType(CollisionShapeType::AABB);
	mesh->SetWorldScaling(Vec3(0.1f, 0.1f, 0.1f));
	calculateLocalCollisionShape();
}

//namespace {
//	// 注册GeneralMeshActor到Actor工厂
//	struct GeneralMeshActorRegistrar {
//		GeneralMeshActorRegistrar() {
//			Actor::RegisterActor("GeneralMeshActor", []() {
//				// 创建无参实例（加载时会调用Load方法初始化路径）
//				return new GeneralMeshActor();
//				});
//		}
//	} generalMeshActorRegistrar; // 全局变量，程序启动时自动执行构造函数
//}

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

namespace {
	struct EnemyActorRegistrar {
		EnemyActorRegistrar() {
			Actor::RegisterActor("EnemyActor", []() { return new EnemyActor(); });
		}
	} enemyActorRegistrar;
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
	enemy_Mesh->drawSingle(myWorld->GetCore(), myWorld->GetPSOManager(), ANIM_LIGHT_PIPE, myWorld->GetPipelines(), animatedInstance, myWorld->GetDeltatime(), animStateMachine);
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
