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
};

struct AnimationFrame
{
	std::vector<Vec3> positions;
	std::vector<Quaternion> rotations;
	std::vector<Vec3> scales;
};

class AnimationSequence
{
public:
	std::vector<AnimationFrame> frames;
	float ticksPerSecond;
	Vec3 interpolate(Vec3 p1, Vec3 p2, float t) {
		return ((p1 * (1.0f - t)) + (p2 * t));
	}
	Quaternion interpolate(Quaternion q1, Quaternion q2, float t) {
		return Quaternion::slerp(q1, q2, t);
	}
	float duration() {
		return ((float)frames.size() / ticksPerSecond);
	}
	void calcFrame(float t, int& frame, float& interpolationFact)
	{
		interpolationFact = t * ticksPerSecond;
		frame = (int)floorf(interpolationFact);
		interpolationFact = interpolationFact - (float)frame;
		frame = std::min(frame, (int)(frames.size() - 1));
	}

	int nextFrame(int frame)
	{
		return std::min(frame + 1, (int)(frames.size() - 1));
	}

	Matrix interpolateBoneToGlobal(Matrix* matrices, int baseFrame, float interpolationFact,
		Skeleton* skeleton, int boneIndex)
	{
		Matrix scale;
		Matrix::SetScaling(scale, interpolate(frames[baseFrame].scales[boneIndex],
			frames[nextFrame(baseFrame)].scales[boneIndex], interpolationFact));
		Matrix rotation = interpolate(frames[baseFrame].rotations[boneIndex],
			frames[nextFrame(baseFrame)].rotations[boneIndex], interpolationFact).toMatrix();
		Matrix translation;
		Vec3 inter = interpolate(frames[baseFrame].positions[boneIndex],
			frames[nextFrame(baseFrame)].positions[boneIndex], interpolationFact);
		translation = Matrix::SetPositionMatrix(Vec4(inter.x, inter.y, inter.z, 1.f));
		Matrix local = translation * rotation * scale;
		if (skeleton->bones[boneIndex].parentIndex > -1)
		{
			Matrix global = matrices[skeleton->bones[boneIndex].parentIndex] * local;
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

	void calcFrame(std::string name, float t, int& frame, float& interpolationFact) {
		animations[name].calcFrame(t, frame, interpolationFact);
	}
	Matrix interpolateBoneToGlobal(std::string name, Matrix* matrices, int baseFrame, float
		interpolationFact, int boneIndex) {
		return animations[name].interpolateBoneToGlobal(matrices, baseFrame,
			interpolationFact, &skeleton, boneIndex);
	}

	void calcFinalTransforms(Matrix* matrices)
	{
		for (int i = 0; i < bonesSize(); i++)
		{
			matrices[i] = matrices[i] * skeleton.bones[i].offset * skeleton.globalInverse;
		}
	}

};
	
class AnimationInstance
{
public:
	Animation* animation;
	std::string currentAnimation;
	float t;
	Matrix matrices[256];

	void resetAnimationTime()
	{
		t = 0;
	}
	bool animationFinished()
	{
		if (t > animation->animations[currentAnimation].duration())
		{
			return true;
		}
		return false;
	}

	void update(std::string name, float dt) {
		if (name == currentAnimation) {
			t += dt;
		}
		else {
			currentAnimation = name;  t = 0;
		}
		if (animationFinished() == true) { resetAnimationTime(); }
		int frame = 0;
		float interpolationFact = 0;
		animation->calcFrame(name, t, frame, interpolationFact);
		for (int i = 0; i < animation->bonesSize(); i++)
		{
			matrices[i] = animation->interpolateBoneToGlobal(name, matrices, frame, interpolationFact, i);
		}
		animation->calcFinalTransforms(matrices);
	}

};

class Mesh
{
public:
	

	ID3D12Resource* vertexBuffer;
	ID3D12Resource* indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vbView;
	D3D12_INDEX_BUFFER_VIEW ibView;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	unsigned int numMeshIndices;

	//D3D12_INPUT_ELEMENT_DESC inputLayout[2];
	//D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
public:
	void init(Core* core, void* vertices, int vertexSizeInBytes, int numVertices,
		unsigned int* indices, int numIndices);

	void init(Core* core, std::vector<STATIC_VERTEX> vertices, std::vector<unsigned int> indices)
	{
		init(core, &vertices[0], sizeof(STATIC_VERTEX), vertices.size(), &indices[0], indices.size());
		inputLayoutDesc = VertexLayoutCache::getStaticLayout();
	}

	void init(Core* core, std::vector<ANIMATED_VERTEX> vertices, std::vector<unsigned int> indices)
	{
		init(core, &vertices[0], sizeof(ANIMATED_VERTEX), vertices.size(), &indices[0], indices.size());
		inputLayoutDesc = VertexLayoutCache::getAnimatedLayout();
	}

	void draw(Core* core);

	static void CreatePlane(Core* core, Mesh* plane);

	static void CreateCube(Core* core, Mesh* cube);

	static void CreateSphere(Core* core, Mesh* sphere, int rings, int segments, float radius);

	static void CreateGEM(Core* core, std::vector<Mesh>& meshes, std::string filename);

	static STATIC_VERTEX addVertex(Vec3 p, Vec3 n, float tu, float tv)
	{
		STATIC_VERTEX v;
		v.pos = p;
		v.normal = n;
		v.tangent = Vec3(0, 0, 0); // For now
		v.tu = tu;
		v.tv = tv;
		return v;
	}

};

class StaticMesh
{
public:
	std::vector<Mesh> meshes;

	StaticMesh(Core* core, std::string filename);
	//void draw(Core* core, PSOManager& psos, Mesh& prim)
	void CreateFromGEM(Core* core, std::string filename);

	void draw(Core* core, PSOManager& psos, std::string pipeName, Pipelines& pipes);
};

class AnimatedModel
{
public:
	std::vector<Mesh*> meshes;
	Animation animation;
	std::vector<std::string> textureFilenames;

	 
//	void load(Core* core, std::string filename, PSOManager* psos, Shaders* shaders)
//	{
//		GEMLoader::GEMModelLoader loader;
//		std::vector<GEMLoader::GEMMesh> gemmeshes;
//		GEMLoader::GEMAnimation gemanimation;
//		loader.load(filename, gemmeshes, gemanimation);
//		for (int i = 0; i < gemmeshes.size(); i++)
//		{
//			Mesh* mesh = new Mesh();
//			std::vector<ANIMATED_VERTEX> vertices;
//			for (int j = 0; j < gemmeshes[i].verticesAnimated.size(); j++)
//			{
//				ANIMATED_VERTEX v;
//				memcpy(&v, &gemmeshes[i].verticesAnimated[j], sizeof(ANIMATED_VERTEX));
//				vertices.push_back(v);
//			}
//			mesh->init(core, vertices, gemmeshes[i].indices);
//			meshes.push_back(mesh);
//		}
//		shaders->load(core, "AnimatedUntextured", "VSAnim.txt", "PSUntextured.txt");
//		psos->createPSO(core, "AnimatedModelPSO", shaders->find("AnimatedUntextured")->vs, shaders->find("AnimatedUntextured")->ps, VertexLayoutCache::getAnimatedLayout());
//		memcpy(&animation.skeleton.globalInverse, &gemanimation.globalInverse, 16 * sizeof(float));
//		for (int i = 0; i < gemanimation.bones.size(); i++)
//		{
//			Bone bone;
//			bone.name = gemanimation.bones[i].name;
//			memcpy(&bone.offset, &gemanimation.bones[i].offset, 16 * sizeof(float));
//			bone.parentIndex = gemanimation.bones[i].parentIndex;
//			animation.skeleton.bones.push_back(bone);
//		}
//		for (int i = 0; i < gemanimation.animations.size(); i++)
//		{
//			std::string name = gemanimation.animations[i].name;
//			AnimationSequence aseq;
//			aseq.ticksPerSecond = gemanimation.animations[i].ticksPerSecond;
//			for (int j = 0; j < gemanimation.animations[i].frames.size(); j++)
//			{
//				AnimationFrame frame;
//				for (int index = 0; index < gemanimation.animations[i].frames[j].positions.size(); index++)
//				{
//					Vec3 p;
//					Quaternion q;
//					Vec3 s;
//					memcpy(&p, &gemanimation.animations[i].frames[j].positions[index], sizeof(Vec3));
//					frame.positions.push_back(p);
//					memcpy(&q, &gemanimation.animations[i].frames[j].rotations[index], sizeof(Quaternion));
//					frame.rotations.push_back(q);
//					memcpy(&s, &gemanimation.animations[i].frames[j].scales[index], sizeof(Vec3));
//					frame.scales.push_back(s);
//				}
//				aseq.frames.push_back(frame);
//			}
//			animation.animations.insert({ name, aseq });
//		}
//	}
//	void updateWorld(Shaders* shaders, Matrix& w)
//	{
//		shaders->updateConstantVS("AnimatedUntextured", "staticMeshBuffer", "W", &w);
//	}
//	void draw(Core* core, PSOManager* psos, Shaders* shaders, AnimationInstance* instance, Matrix& vp, Matrix& w)
//	{
//		psos->bind(core, "AnimatedModelPSO");
//		shaders->updateConstantVS("AnimatedUntextured", "staticMeshBuffer", "W", &w);
//		shaders->updateConstantVS("AnimatedUntextured", "staticMeshBuffer", "VP", &vp);
//		shaders->updateConstantVS("AnimatedUntextured", "staticMeshBuffer", "bones", instance->matrices);
//		shaders->apply(core, "AnimatedUntextured");
//		for (int i = 0; i < meshes.size(); i++)
//		{
//			meshes[i]->draw(core);
//		}
//	}
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