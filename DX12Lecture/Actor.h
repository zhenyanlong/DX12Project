#pragma once
#include "Mesh.h"
#include "GeneralEvent.h"
#include "ICameraControllable.h"
#include "Collision.h"
#include "Animation/FPSAnimationStateMachine.h"
#include "Animation/EnemyAnimationStateMachine.h"
#include <fstream>
#include <string>
#include <map>
#include <functional>

enum class ActorType {
	None,
	Player,        
	Bullet,     
	Enemy,      
	Static      
};
class Actor	: public GeneralEvent
{
	
	// collision system
protected:
	CollisionShapeType m_collisionShapeType = CollisionShapeType::None;
	AABB m_localAABB;       
	Sphere m_localSphere;   
	bool m_isCollidable = false; 
	// Actor type
	ActorType m_actorType;
	
	bool m_isDestroyed;
public:
	Actor() : m_actorType(ActorType::Static), m_isDestroyed(false) {};
	virtual ~Actor() = default;
	

	// collision api
	void setCollidable(bool enable) { m_isCollidable = enable; }
	bool isCollidable() const { return m_isCollidable; }
	void setCollisionShapeType(CollisionShapeType type) { m_collisionShapeType = type; }
	CollisionShapeType getCollisionShapeType() const { return m_collisionShapeType; }
	ActorType getActorType() const { return m_actorType; }
	bool getIsDestroyed() const { return m_isDestroyed; }

	
	const AABB& getLocalAABB() const { return m_localAABB; }
	const Sphere& getLocalSphere() const { return m_localSphere; }


	
	virtual AABB getWorldAABB() const
	{
		if (m_collisionShapeType != CollisionShapeType::AABB)
			return AABB();

		Matrix worldMat = getWorldMatrix();
		auto vertices = m_localAABB.getVertices();
		AABB worldAABB;
		worldAABB.reset();
		for (const auto& v : vertices)
		{
			Vec3 worldV = worldMat.mulPoint(v); 
			worldAABB.extend(worldV);
		}
		return worldAABB;
	}

	// OBB (Oriented bounding box)
	virtual OBB getWorldOBB() const
	{
		if (m_collisionShapeType != CollisionShapeType::OBB)
			return OBB();

		Matrix worldMat = getWorldMatrix();
		return OBB::fromAABB(m_localAABB, worldMat);
	}

	virtual Sphere getWorldSphere() const
	{
		if (m_collisionShapeType != CollisionShapeType::Sphere)
			return Sphere();

		Matrix worldMat = getWorldMatrix();
		Vec3 worldCentre = worldMat.mulPoint(m_localSphere.centre);
		
		Vec3 scale = getWorldScale();
		float worldRadius = m_localSphere.radius * std::max({ scale.x, scale.y, scale.z });
		return Sphere(worldCentre, worldRadius);
	}

	// Calculate local collision bodies from Mesh
	virtual void calculateLocalCollisionShape() = 0;
	// **** world info interface ****//
	
	virtual Matrix getWorldMatrix() const = 0;

	virtual Vec3 getWorldPos() const = 0;
	virtual void setWorldPos(Vec3 worldPos) = 0;

	virtual Vec3 getWorldScale() const = 0;
	virtual void setWorldScale(Vec3 worldScale) = 0;

	virtual Vec3 getWorldRotation() const = 0;
	virtual void setWorldRotation(Vec3 worldRotation) = 0;
	// **** world info interface ****//
public:
	// draw
	virtual void draw() = 0;

	// factory mode
public:
	using ActorCreator = std::function<Actor* ()>;
	static std::map<std::string, ActorCreator> m_actorCreators;
	static void RegisterActor(const std::string& className, ActorCreator creator);
	static Actor* CreateActorByClassName(const std::string& className);

	// Serialization-related
public:
	
	virtual std::string GetClassName() const = 0;
	// save / load
	virtual void Save(std::ofstream& file) const = 0;
	virtual void Load(std::ifstream& file) = 0;

protected:
	
	void SaveBase(std::ofstream& file) const;
	void LoadBase(std::ifstream& file);
};

class SkyBoxActor: public Actor
{
	StaticMesh* skybox;
public:
	SkyBoxActor();
	virtual void draw() override;

	
	virtual void calculateLocalCollisionShape() override {}
	// **** world info interface ****//
	
	virtual Matrix getWorldMatrix() const override { return skybox->GetWorldMatrix(); }

	virtual Vec3 getWorldPos() const override { return skybox->GetWorldPos(); }
	virtual void setWorldPos(Vec3 worldPos) override { skybox->SetWorldPos(worldPos); }

	virtual Vec3 getWorldScale() const override { return skybox->GetWorldScale(); }
	virtual void setWorldScale(Vec3 worldScale) override { skybox->SetWorldScaling(worldScale); }

	virtual Vec3 getWorldRotation() const override { return skybox->GetWorldRotationRadian(); }
	virtual void setWorldRotation(Vec3 worldRotation) override { skybox->SetWorldRotationRadian(worldRotation); }
	// **** world info interface ****//

public:
	
	std::string GetClassName() const override { return "SkyBoxActor"; }
	
	void Save(std::ofstream& file) const override
	{
		SaveBase(file);
	}
	void Load(std::ifstream& file) override
	{
		LoadBase(file);
	}
};

class TreeActor : public Actor
{
	StaticMesh* willow;
	std::vector<Matrix> instanceMatrices; 
	int m_instanceCount; 
	Vec3 m_transIncrement;
public:
	// Generate instance matrices in batches
	void generateInstanceMatrices(int count, Vec3 transIncrement);

	
	TreeActor(int count = 50, Vec3 transIncrement = Vec3(0.f, 0.f, 20.f));
	virtual void draw() override;

	
	virtual void calculateLocalCollisionShape() override {}
	// **** world info interface ****//
	
	virtual Matrix getWorldMatrix() const override { return willow->GetWorldMatrix(); }

	virtual Vec3 getWorldPos() const override { return willow->GetWorldPos(); }
	virtual void setWorldPos(Vec3 worldPos) override { willow->SetWorldPos(worldPos); }

	virtual Vec3 getWorldScale() const override { return willow->GetWorldScale(); }
	virtual void setWorldScale(Vec3 worldScale) override { willow->SetWorldScaling(worldScale); }

	virtual Vec3 getWorldRotation() const override { return willow->GetWorldRotationRadian(); }
	virtual void setWorldRotation(Vec3 worldRotation) override { willow->SetWorldRotationRadian(worldRotation); }
	// **** world info interface ****//

public:
	// class name with tree actor
	std::string GetClassName() const override { return "TreeActor"; }
	// save / load with Serialization
	void Save(std::ofstream& file) const override
	{
		SaveBase(file);
		
		file.write(reinterpret_cast<const char*>(&m_instanceCount), sizeof(int));
		file.write(reinterpret_cast<const char*>(&m_transIncrement), sizeof(Vec3));
	}
	void Load(std::ifstream& file) override
	{
		LoadBase(file);
		
		file.read(reinterpret_cast<char*>(&m_instanceCount), sizeof(int));
		file.read(reinterpret_cast<char*>(&m_transIncrement), sizeof(Vec3));

		generateInstanceMatrices(m_instanceCount, m_transIncrement);
	}
};

class WaterActor : public Actor
{
	StaticMesh* water;

public:
	WaterActor();
	virtual void draw() override;

	
	virtual void calculateLocalCollisionShape() override {}
	// **** world info interface ****//
	
	virtual Matrix getWorldMatrix() const override { return water->GetWorldMatrix(); }

	virtual Vec3 getWorldPos() const override { return water->GetWorldPos(); }
	virtual void setWorldPos(Vec3 worldPos) override { water->SetWorldPos(worldPos); }

	virtual Vec3 getWorldScale() const override { return water->GetWorldScale(); }
	virtual void setWorldScale(Vec3 worldScale) override { water->SetWorldScaling(worldScale); }

	virtual Vec3 getWorldRotation() const override { return water->GetWorldRotationRadian(); }
	virtual void setWorldRotation(Vec3 worldRotation) override { water->SetWorldRotationRadian(worldRotation); }
	// **** world info interface ****//
public:
	
	std::string GetClassName() const override { return "WaterActor"; }
	
	void Save(std::ofstream& file) const override
	{
		SaveBase(file);
	}
	void Load(std::ifstream& file) override
	{
		LoadBase(file);
	}
};

class FPSActor : public Actor, public CameraControllable 
{
	AnimatedModel* fps_Mesh;	
	AnimationInstance* animatedInstance;
public:
	FPSAnimationStateMachine* animStateMachine; 
	
public:
	FPSActor();
	virtual ~FPSActor() override; 
	virtual void draw() override;

	virtual void updatePos(Vec3 pos) override;

	virtual void updateRotation(Vec3 rot) override;

	virtual void updateRotation(Matrix rotMat) override;

	virtual void updateWorldMatrix(Vec3 pos, float yaw, float pitch) override;

	
	virtual void calculateLocalCollisionShape() override;
	// **** world info interface ****//
	
	virtual Matrix getWorldMatrix() const override { return fps_Mesh->GetWorldMatrix(); }

	virtual Vec3 getWorldPos() const override { return fps_Mesh->GetWorldPos(); }
	virtual void setWorldPos(Vec3 worldPos) override { fps_Mesh->SetWorldPos(worldPos); }

	virtual Vec3 getWorldScale() const override { return fps_Mesh->GetWorldScale(); }
	virtual void setWorldScale(Vec3 worldScale) override { fps_Mesh->SetWorldScaling(worldScale); }

	virtual Vec3 getWorldRotation() const override { return fps_Mesh->GetWorldRotationRadian(); }
	virtual void setWorldRotation(Vec3 worldRotation) override { fps_Mesh->SetWorldRotationRadian(worldRotation); }
	// **** world info interface ****//

	virtual void OnBeginPlay() override;
	virtual void OnTick(float dt) override;
public:
	
	std::string GetClassName() const override { return "FPSActor"; }
	
	void Save(std::ofstream& file) const override
	{
		SaveBase(file);
	}
	void Load(std::ifstream& file) override
	{
		LoadBase(file);
	}
};

class EnemyActor : public Actor
{
	AnimatedModel* enemy_Mesh;
	AnimationInstance* animatedInstance;
public:
	EnemyAnimationStateMachine* animStateMachine; 

	EnemyActor();
	virtual ~EnemyActor() override; 
	virtual void draw() override;

	
	virtual void calculateLocalCollisionShape() override;
	// **** world info interface ****//
	
	virtual Matrix getWorldMatrix() const override { return enemy_Mesh->GetWorldMatrix(); }

	virtual Vec3 getWorldPos() const override { return enemy_Mesh->GetWorldPos(); }
	virtual void setWorldPos(Vec3 worldPos) override { enemy_Mesh->SetWorldPos(worldPos); }

	virtual Vec3 getWorldScale() const override { return enemy_Mesh->GetWorldScale(); }
	virtual void setWorldScale(Vec3 worldScale) override { enemy_Mesh->SetWorldScaling(worldScale); }

	virtual Vec3 getWorldRotation() const override { return enemy_Mesh->GetWorldRotationRadian(); }
	virtual void setWorldRotation(Vec3 worldRotation) override { enemy_Mesh->SetWorldRotationRadian(worldRotation); }
	// **** world info interface ****//

	virtual void OnBeginPlay() override;
	virtual void OnTick(float dt) override;

	void Destroy() { m_isDestroyed = true; }

public:
	
	std::string GetClassName() const override { return "EnemyActor"; }
	
	void Save(std::ofstream& file) const override
	{
		SaveBase(file);
	}
	void Load(std::ifstream& file) override
	{
		LoadBase(file);
	}
};
class BoxActor : public Actor
{
	StaticMesh* box;
public:
	BoxActor();
	virtual void draw() override;

	
	virtual void calculateLocalCollisionShape() override;
	// **** world info interface ****//
	
	virtual Matrix getWorldMatrix() const override { return box->GetWorldMatrix(); }

	virtual Vec3 getWorldPos() const override { return box->GetWorldPos(); }
	virtual void setWorldPos(Vec3 worldPos) override { box->SetWorldPos(worldPos); }

	virtual Vec3 getWorldScale() const override { return box->GetWorldScale(); }
	virtual void setWorldScale(Vec3 worldScale) override { box->SetWorldScaling(worldScale); }

	virtual Vec3 getWorldRotation() const override { return box->GetWorldRotationRadian(); }
	virtual void setWorldRotation(Vec3 worldRotation) override { box->SetWorldRotationRadian(worldRotation); }
	// **** world info interface ****//

public:
	
	std::string GetClassName() const override { return "BoxActor"; }
	
	void Save(std::ofstream& file) const override
	{
		SaveBase(file);
	}
	void Load(std::ifstream& file) override
	{
		LoadBase(file);
	}
};

class GroundActor : public Actor
{
	StaticMesh* ground;
public:
	GroundActor();
	virtual void draw() override;

	
	virtual void calculateLocalCollisionShape() override;
	// **** world info interface ****//
	
	virtual Matrix getWorldMatrix() const override { return ground->GetWorldMatrix(); }

	virtual Vec3 getWorldPos() const override { return ground->GetWorldPos(); }
	virtual void setWorldPos(Vec3 worldPos) override { ground->SetWorldPos(worldPos); }

	virtual Vec3 getWorldScale() const override { return ground->GetWorldScale(); }
	virtual void setWorldScale(Vec3 worldScale) override { ground->SetWorldScaling(worldScale); }

	virtual Vec3 getWorldRotation() const override { return ground->GetWorldRotationRadian(); }
	virtual void setWorldRotation(Vec3 worldRotation) override { ground->SetWorldRotationRadian(worldRotation); }
	// **** world info interface ****//
public:
	
	std::string GetClassName() const override { return "GroundActor"; }
	
	void Save(std::ofstream& file) const override
	{
		SaveBase(file);
	}
	void Load(std::ifstream& file) override
	{
		LoadBase(file);
	}
};

class ContainerBlueActor : public Actor
{
	StaticMesh* container;
public:
	ContainerBlueActor();
	virtual void draw() override;

	
	virtual void calculateLocalCollisionShape() override;
	// **** world info interface ****//
	
	virtual Matrix getWorldMatrix() const override { return container->GetWorldMatrix(); }

	virtual Vec3 getWorldPos() const override { return container->GetWorldPos(); }
	virtual void setWorldPos(Vec3 worldPos) override { container->SetWorldPos(worldPos); }

	virtual Vec3 getWorldScale() const override { return container->GetWorldScale(); }
	virtual void setWorldScale(Vec3 worldScale) override { container->SetWorldScaling(worldScale); }

	virtual Vec3 getWorldRotation() const override { return container->GetWorldRotationRadian(); }
	virtual void setWorldRotation(Vec3 worldRotation) override { container->SetWorldRotationRadian(worldRotation); }
	// **** world info interface ****//
public:
	
	std::string GetClassName() const override { return "ContainerBlueActor"; }
	
	void Save(std::ofstream& file) const override
	{
		SaveBase(file);
	}
	void Load(std::ifstream& file) override
	{
		LoadBase(file);
	}
};

class BlockActor :public Actor
{
	StaticMesh* box;
public:
	BlockActor();
	virtual void draw() override;

	
	virtual void calculateLocalCollisionShape() override;
	// **** world info interface ****//
	
	virtual Matrix getWorldMatrix() const override { return box->GetWorldMatrix(); }

	virtual Vec3 getWorldPos() const override { return box->GetWorldPos(); }
	virtual void setWorldPos(Vec3 worldPos) override { box->SetWorldPos(worldPos); }

	virtual Vec3 getWorldScale() const override { return box->GetWorldScale(); }
	virtual void setWorldScale(Vec3 worldScale) override { box->SetWorldScaling(worldScale); }

	virtual Vec3 getWorldRotation() const override { return box->GetWorldRotationRadian(); }
	virtual void setWorldRotation(Vec3 worldRotation) override { box->SetWorldRotationRadian(worldRotation); }
	// **** world info interface ****//
public:
	
	std::string GetClassName() const override { return "BlockActor"; }
	
	void Save(std::ofstream& file) const override
	{
		SaveBase(file);
	}
	void Load(std::ifstream& file) override
	{
		LoadBase(file);
	}
};

class ObstacleActor	:public Actor
{
	StaticMesh* obstacle;
	std::vector<Matrix> instanceMatrices; 
	int m_instanceCount; 
	Vec3 m_offset;
public:
	ObstacleActor(int count = 5, Vec3 offset = Vec3(0.f, 0.f, 5.f));

	void generateInstanceMatrices(int count, Vec3 offset);

	virtual void draw() override;

	
	virtual void calculateLocalCollisionShape() override;
	// **** world info interface ****//
	
	virtual Matrix getWorldMatrix() const override { return obstacle->GetWorldMatrix(); }

	virtual Vec3 getWorldPos() const override { return obstacle->GetWorldPos(); }
	virtual void setWorldPos(Vec3 worldPos) override { obstacle->SetWorldPos(worldPos); }

	virtual Vec3 getWorldScale() const override { return obstacle->GetWorldScale(); }
	virtual void setWorldScale(Vec3 worldScale) override { obstacle->SetWorldScaling(worldScale); }

	virtual Vec3 getWorldRotation() const override { return obstacle->GetWorldRotationRadian(); }
	virtual void setWorldRotation(Vec3 worldRotation) override { obstacle->SetWorldRotationRadian(worldRotation); }
	// **** world info interface ****//
public:
	
	std::string GetClassName() const override { return "ObstacleActor"; }
	
	void Save(std::ofstream& file) const override
	{
		SaveBase(file);

		file.write(reinterpret_cast<const char*>(&m_instanceCount), sizeof(int));
		file.write(reinterpret_cast<const char*>(&m_offset), sizeof(Vec3));

	}
	void Load(std::ifstream& file) override
	{
		LoadBase(file);
		file.read(reinterpret_cast<char*>(&m_instanceCount), sizeof(int));
		file.read(reinterpret_cast<char*>(&m_offset), sizeof(Vec3));

		generateInstanceMatrices(m_instanceCount, m_offset);
	}
};

class GeneralMeshActor :public Actor
{
	StaticMesh* mesh;
	std::string m_path;
public:
	GeneralMeshActor(std::string path = "Models/container_005.gem");
	virtual ~GeneralMeshActor()	override
	{
		if (mesh) {
			delete mesh;
			mesh = nullptr;
		}
	}
	virtual void draw() override;
	
	virtual void calculateLocalCollisionShape() override;
	// **** world info interface ****//
	
	virtual Matrix getWorldMatrix() const override { return mesh->GetWorldMatrix(); }

	virtual Vec3 getWorldPos() const override { return mesh->GetWorldPos(); }
	virtual void setWorldPos(Vec3 worldPos) override { mesh->SetWorldPos(worldPos); }

	virtual Vec3 getWorldScale() const override { return mesh->GetWorldScale(); }
	virtual void setWorldScale(Vec3 worldScale) override { mesh->SetWorldScaling(worldScale); }

	virtual Vec3 getWorldRotation() const override { return mesh->GetWorldRotationRadian(); }
	virtual void setWorldRotation(Vec3 worldRotation) override { mesh->SetWorldRotationRadian(worldRotation); }
	// **** world info interface ****//
public:
	
	std::string GetClassName() const override { return "GeneralMeshActor"; }
	
	void Save(std::ofstream& file) const override
	{
		
		SaveBase(file);

		// Save the length of m_path
		int pathLen = static_cast<int>(m_path.size());
		file.write(reinterpret_cast<const char*>(&pathLen), sizeof(int));

		// Save the character content of m_path
		if (pathLen > 0) {
			file.write(m_path.c_str(), pathLen);
		}
	}
	void Load(std::ifstream& file) override
	{
		
		LoadBase(file);

		// Read the length of m_path
		int pathLen;
		file.read(reinterpret_cast<char*>(&pathLen), sizeof(int));

		// Read the character content of m_path
		std::string path;
		if (pathLen > 0) {
			path.resize(pathLen);
			file.read(&path[0], pathLen);
		}
		Vec3 scale = getWorldScale(); 
		Vec3 pos = getWorldPos();
		Vec3 rot = getWorldRotation();
		
		initMesh(path);

		// Restore the world position, rotation and scale of the base class loading
		
		mesh->SetWorldPos(pos);
		mesh->SetWorldRotationRadian(rot);
		mesh->SetWorldScaling(scale);
	}
private:
	void initMesh(const std::string& path);
};

class BulletActor : public Actor
{
private:
	Vec3 m_direction; 
	float m_speed;    
	int m_damage;     
	float m_lifeTime; 
	const float MAX_LIFE_TIME = 3.0f; 

	StaticMesh* m_bulletMesh;
protected:
	virtual void OnTick(float dt) override;
public:
	BulletActor(const Vec3 pos, const Vec3 dir, float speed = 100.0f, int damage = 10);
	~BulletActor() override = default;

	virtual void draw() override;
	
	virtual void calculateLocalCollisionShape() override;
	// **** world info interface ****//
	
	virtual Matrix getWorldMatrix() const override { return m_bulletMesh->GetWorldMatrix(); }

	virtual Vec3 getWorldPos() const override { return m_bulletMesh->GetWorldPos(); }
	virtual void setWorldPos(Vec3 worldPos) override { m_bulletMesh->SetWorldPos(worldPos); }

	virtual Vec3 getWorldScale() const override { return m_bulletMesh->GetWorldScale(); }
	virtual void setWorldScale(Vec3 worldScale) override { m_bulletMesh->SetWorldScaling(worldScale); }

	virtual Vec3 getWorldRotation() const override { return m_bulletMesh->GetWorldRotationRadian(); }
	virtual void setWorldRotation(Vec3 worldRotation) override { m_bulletMesh->SetWorldRotationRadian(worldRotation); }
	// **** world info interface ****//

	void Destroy() { m_isDestroyed = true; }

public:
	
	std::string GetClassName() const override { return "BulletActor"; }
	
	void Save(std::ofstream& file) const override
	{
		SaveBase(file);

		file.write(reinterpret_cast<const char*>(&m_direction), sizeof(Vec3));
		file.write(reinterpret_cast<const char*>(&m_speed), sizeof(float));
		file.write(reinterpret_cast<const char*>(&m_damage), sizeof(int));
		file.write(reinterpret_cast<const char*>(&m_lifeTime), sizeof(float));
	}
	void Load(std::ifstream& file) override
	{
		LoadBase(file);

		file.read(reinterpret_cast<char*>(&m_direction), sizeof(Vec3));
		file.read(reinterpret_cast<char*>(&m_speed), sizeof(float));
		file.read(reinterpret_cast<char*>(&m_damage), sizeof(int));
		file.read(reinterpret_cast<char*>(&m_lifeTime), sizeof(float));
	}
};
