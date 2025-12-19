#pragma once
#include "Core.h"
#include "ScreenSpaceTriangle.h"
#include "VertexLayoutCache.h"
#include "PSOManager.h"
#include "map"
#include "iostream"
#include <algorithm>
#include "math.h"
#include "Pipeline.h"
#undef min
#undef max



struct Bone
{
	std::string name;
	Matrix offset;
	int parentIndex;
};

struct Skeleton
{
	std::vector<Bone> bones;
	Matrix globalInverse;
	int findBone(std::string name)
	{
		for (int i = 0; i < bones.size(); i++)
		{
			if (bones[i].name == name)
			{
				return i;
			}
		}
		return -1;
	}
};

struct AnimationFrame
{
	std::vector<Vec3> positions;
	std::vector<Quaternion> rotations;
	std::vector<Vec3> scales;
};

struct AnimationSequence // This holds rescaled times
{
	std::vector<AnimationFrame> frames;
	float ticksPerSecond;
	Vec3 interpolate(Vec3 p1, Vec3 p2, float t)
	{
		return ((p1 * (1.0f - t)) + (p2 * t));
	}
	Quaternion interpolate(Quaternion q1, Quaternion q2, float t)
	{
		return Quaternion::slerp(q1, q2, t);
	}
	float duration()
	{
		return ((float)frames.size() / ticksPerSecond);
	}
	void calcFrame(float t, int& frame, float& interpolationFact)
	{
		interpolationFact = t * ticksPerSecond;
		frame = (int)floorf(interpolationFact);
		interpolationFact = interpolationFact - (float)frame;
		frame = std::min(frame, (int)(frames.size() - 1));
	}
	bool running(float t)
	{
		if ((int)floorf(t * ticksPerSecond) < frames.size())
		{
			return true;
		}
		return false;
	}
	int nextFrame(int frame)
	{
		return std::min(frame + 1, (int)(frames.size() - 1));
	}
	Matrix interpolateBoneToGlobal(Matrix* matrices, int baseFrame, float interpolationFact, Skeleton* skeleton, int boneIndex)
	{
		Matrix scale = Matrix::scaling(interpolate(frames[baseFrame].scales[boneIndex], frames[nextFrame(baseFrame)].scales[boneIndex], interpolationFact));
		Matrix rotation = interpolate(frames[baseFrame].rotations[boneIndex], frames[nextFrame(baseFrame)].rotations[boneIndex], interpolationFact).toMatrix();
		Matrix translation = Matrix::translation(interpolate(frames[baseFrame].positions[boneIndex], frames[nextFrame(baseFrame)].positions[boneIndex], interpolationFact));
		Matrix local = scale * rotation * translation;
		if (skeleton->bones[boneIndex].parentIndex > -1)
		{
			Matrix global = local * matrices[skeleton->bones[boneIndex].parentIndex];
			return global;
		}
		return local;
	}
};

class Animation
{
public:
	std::map<std::string, AnimationSequence> animations;
	Skeleton skeleton;
	int bonesSize()
	{
		return skeleton.bones.size();
	}
	void calcFrame(std::string name, float t, int& frame, float& interpolationFact)
	{
		animations[name].calcFrame(t, frame, interpolationFact);
	}
	Matrix interpolateBoneToGlobal(std::string name, Matrix* matrices, int baseFrame, float interpolationFact, int boneIndex)
	{
		return animations[name].interpolateBoneToGlobal(matrices, baseFrame, interpolationFact, &skeleton, boneIndex);
	}
	void calcTransforms(Matrix* matrices, Matrix coordTransform)
	{
		for (int i = 0; i < bonesSize(); i++)
		{
			matrices[i] = skeleton.bones[i].offset * matrices[i] * skeleton.globalInverse * coordTransform;
		}
	}
	bool hasAnimation(std::string name)
	{
		if (animations.find(name) == animations.end())
		{
			return false;
		}
		return true;
	}

	std::vector<std::string> getAllAnimationNames() const
	{
		std::vector<std::string> animationNames;
		// get all keys
		for (const auto& animationPair : animations)
		{
			animationNames.push_back(animationPair.first);
		}
		return animationNames;
	}
};

class AnimationInstance
{
public:
	Animation* animation;
	std::string usingAnimation;
	float t;
	Matrix matrices[256]; // This is defined as 256 to match the maximum number in the shader
	Matrix matricesPose[256]; // This is to store transforms needed for finding bone positions
	Matrix coordTransform;
	void init(Animation* _animation, int fromYZX)
	{
		animation = _animation;
		if (fromYZX == 1)
		{
			memset(coordTransform.a, 0, 16 * sizeof(float));
			coordTransform.a[0][0] = 1.0f;
			coordTransform.a[2][1] = 1.0f;
			coordTransform.a[1][2] = -1.0f;
			coordTransform.a[3][3] = 1.0f;
		}
	}
	void update(std::string name, float dt)
	{
		if (name == usingAnimation)
		{
			t += dt;
		}
		else
		{
			usingAnimation = name;
			t = 0;
		}
		if (animationFinished() == true)
		{
			return;
		}
		int frame = 0;
		float interpolationFact = 0;
		animation->calcFrame(name, t, frame, interpolationFact);
		for (int i = 0; i < animation->bonesSize(); i++)
		{
			matrices[i] = animation->interpolateBoneToGlobal(name, matrices, frame, interpolationFact, i);
		}
		animation->calcTransforms(matrices, coordTransform);
	}
	void resetAnimationTime()
	{
		t = 0;
	}
	bool animationFinished()
	{
		if (t > animation->animations[usingAnimation].duration())
		{
			return true;
		}
		return false;
	}
	Matrix findWorldMatrix(std::string boneName)
	{
		int boneID = animation->skeleton.findBone(boneName);
		std::vector<int> boneChain;
		int ID = boneID;
		while (ID != -1)
		{
			boneChain.push_back(ID);
			ID = animation->skeleton.bones[ID].parentIndex;
		}
		int frame = 0;
		float interpolationFact = 0;
		animation->calcFrame(usingAnimation, t, frame, interpolationFact);
		for (int i = boneChain.size() - 1; i > -1; i = i - 1)
		{
			matricesPose[boneChain[i]] = animation->interpolateBoneToGlobal(usingAnimation, matricesPose, frame, interpolationFact, boneChain[i]);
		}
		return (matricesPose[boneID] * coordTransform);
	}
};

class Mesh
{
public:
	enum class VertexType
	{
		Static,     // 静态顶点（STATIC_VERTEX）
		Animated,   // 动画顶点（ANIMATED_VERTEX）
		Unknown     // 未知类型（通用void*顶点，暂不支持）
	};

	ID3D12Resource* vertexBuffer;
	ID3D12Resource* indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vbView;
	D3D12_INDEX_BUFFER_VIEW ibView;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	unsigned int numMeshIndices;

	//D3D12_INPUT_ELEMENT_DESC inputLayout[2];
	//D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	std::vector<STATIC_VERTEX> m_staticVertices;    // 静态顶点数据
	std::vector<ANIMATED_VERTEX> m_animatedVertices;// 动画顶点数据
	VertexType m_vertexType = VertexType::Unknown;  // 当前顶点类型
	
public:
	void init(Core* core, void* vertices, int vertexSizeInBytes, int numVertices,
		unsigned int* indices, int numIndices);

	void init(Core* core, std::vector<STATIC_VERTEX> vertices, std::vector<unsigned int> indices)
	{
		m_staticVertices = std::move(vertices);
		m_vertexType = VertexType::Static;

		init(core, &m_staticVertices[0], sizeof(STATIC_VERTEX), m_staticVertices.size(), &indices[0], indices.size());
		inputLayoutDesc = VertexLayoutCache::getStaticLayout();
	}

	void init(Core* core, std::vector<ANIMATED_VERTEX> vertices, std::vector<unsigned int> indices)
	{
		m_animatedVertices = std::move(vertices);
		m_vertexType = VertexType::Animated;

		init(core, &m_animatedVertices[0], sizeof(ANIMATED_VERTEX), m_animatedVertices.size(), &indices[0], indices.size());
		inputLayoutDesc = VertexLayoutCache::getAnimatedLayout();
	}

	void draw(Core* core);

	void drawInstanced(Core* core, int instanceCount);

	static void CreatePlane(Core* core, Mesh* plane);

	static void CreateCube(Core* core, Mesh* cube);

	static void CreateSphere(Core* core, Mesh* sphere, int rings, int segments, float radius);

	static void CreateGEM(Core* core, std::vector<Mesh>& meshes, std::string filename);

	static STATIC_VERTEX addVertex(Vec3 p, Vec3 n, float tu, float tv, Vec3 tang = Vec3(0, 0, 0))
	{
		STATIC_VERTEX v;
		v.pos = p;
		v.normal = n;
		v.tangent = tang; // For now
		v.tu = tu;
		v.tv = tv;
		return v;
	}
	// ===== 新增：getVertices方法（返回所有顶点的位置）=====
	std::vector<Vec3> getVertices() const
	{
		std::vector<Vec3> vertices;
		switch (m_vertexType)
		{
		case VertexType::Static:
			// 提取静态顶点的位置
			vertices.reserve(m_staticVertices.size());
			for (const auto& v : m_staticVertices)
			{
				vertices.push_back(v.pos);
			}
			break;

		case VertexType::Animated:
			// 提取动画顶点的位置
			vertices.reserve(m_animatedVertices.size());
			for (const auto& v : m_animatedVertices)
			{
				vertices.push_back(v.pos);
			}
			break;

		case VertexType::Unknown:
			// 通用void*顶点暂不支持（可根据需求扩展）
			std::cerr << "Mesh: Unknown vertex type, cannot get vertices!" << std::endl;
			break;
		}
		return vertices;
	}

	// ===== 重载：获取经过模型矩阵变换后的世界空间顶点（可选，用于碰撞体的世界空间计算）=====
	std::vector<Vec3> getVertices(Matrix& modelMatrix) const
	{
		std::vector<Vec3> vertices = getVertices();
		// 将局部空间顶点变换到世界空间
		for (auto& v : vertices)
		{
			v = modelMatrix.mulPoint(v); // 假设Matrix类有transformPoint方法（点变换）
		}
		return vertices;
	}
};

class WorldPosParam
{
protected:
	Matrix m_worldPosMat;
	Vec3 worldPos;
	Vec3 worldScaling;
	Vec3 worldRotationRadian;
	Matrix m_worldRotation;
public:
	WorldPosParam()
	{
		worldRotationRadian = Vec3(0.f, 0.f, 0.f);
		worldScaling = Vec3(0.01f, 0.01f, 0.01f);
		updateWorldMatrix();
	}
	inline Vec3 GetWorldPos() const
	{
		return worldPos;
	}
	inline Vec3 GetWorldScale() const
	{
		return worldScaling;
	}
	inline Vec3 GetWorldRotationRadian() const
	{
		return worldRotationRadian;
	}

	inline Matrix GetWorldMatrix() const
	{
		return m_worldPosMat;
	}

	void SetWorldPos(const Vec3& pos)
	{
		worldPos = pos;
		updateWorldMatrix();
	}
	void SetWorldScaling(const Vec3& scaling)
	{
		worldScaling = scaling;
		updateWorldMatrix();
	}
	void SetWorldRotationRadian(const Vec3& rotationRadian)
	{
		worldRotationRadian = rotationRadian;
		m_worldRotation = Matrix::rotateY(worldRotationRadian.y) * Matrix::rotateX(worldRotationRadian.x) * Matrix::rotateZ(worldRotationRadian.z);
		updateWorldMatrix();
	}

	void SetRotationMatrix(Matrix rotMat)
	{
		m_worldRotation = rotMat;
		updateWorldMatrix();
	}

	void updateWorldMatrix()
	{
		
		m_worldPosMat = Matrix::scaling(worldScaling) * m_worldRotation * Matrix::translation(worldPos) ;
	}
};

class StaticMesh : public WorldPosParam
{
	
public:
	std::vector<Mesh> meshes;
	std::vector<std::string> textureFilenames;			// albedo Textures
	std::vector<std::string> normalTextureFilenames;	// nh Textures	

	StaticMesh();
	StaticMesh(Core* core, std::string filename);
	
	//void draw(Core* core, PSOManager& psos, Mesh& prim)
	void CreateFromGEM(Core* core, std::string filename);
	void CreateFromSphere(Core* core, int rings, int segments, float radius, std::string skyPath);
	void CreateFromPlane(Core* core, float sizeX = 100.0f, float sizeZ = 100.f, int xSegments = 100, int zSegments = 100, std::string texName = "Models/Textures/Textures1_ALB.png", std::string nhName = "Models/Textures/Textures1_NH.png");

private:
	void drawCommon(Core* core, PSOManager* psos, Pipelines* pipes, const std::string& pipeName, int instanceCount = 1);
	
public:
	void draw(Core* core, PSOManager* psos, std::string pipeName, Pipelines* pipes, std::vector<Matrix>* instanceMatrices = nullptr, int instanceCount = 1);
	void drawSingle(Core* core, PSOManager* const psos, std::string pipeName, Pipelines* const pipes);
	void drawInstances(Core* core, PSOManager* const psos, std::string pipeName, Pipelines* const pipes, std::vector<Matrix>* const instanceMatrices, int count);
};
class AnimationStateMachine;

class AnimatedModel : public WorldPosParam
{
	
public:
	std::vector<Mesh*> meshes;
	Animation animation;
	std::vector<std::string> textureFilenames;			// albedo Textures
	std::vector<std::string> normalTextureFilenames;	// nh Textures	

	// init function (load)
	AnimatedModel(Core* core, std::string filename);
	void CreateFromGEM(Core* core, std::string filename);
private:
	// ===== 新增：公共绘制逻辑（参考StaticMesh的drawCommon）=====
	void drawCommon(Core* core, PSOManager* psos, Pipelines* pipes, const std::string& pipeName,
		AnimationInstance* instance, int instanceCount = 1);

public:
	void draw(Core* core, PSOManager* psos, std::string pipeName, Pipelines* pipes,
		AnimationInstance* instance, const std::string& animName, float dt, int instanceCount = 1);

	void drawSingle(Core* core, PSOManager* const psos, std::string pipeName, Pipelines* const pipes, AnimationInstance* instance, float dt, AnimationStateMachine* fpsSM = nullptr);
	
};

struct GeneralMatrix
{
	Matrix worldMatrix;
	Matrix viewProjMatrix;


	//Get single instance pointer
	static GeneralMatrix* Get() {
		return SingleInstance;
	}

	//Create World Single Instance
	static GeneralMatrix* Create()
	{
		if (SingleInstance == nullptr)
		{
			SingleInstance = new GeneralMatrix();
		}
		return SingleInstance;
	}
private:
	//single instance
	static GeneralMatrix* SingleInstance;
};