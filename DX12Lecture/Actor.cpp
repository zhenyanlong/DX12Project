#include "Actor.h"
#include "World.h"
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
	for (int i = 0; i < count; i++)
	{
		// 随机极坐标位置
		float angle = (float)rand() / RAND_MAX * 2 * M_PI;
		float r = (float)rand() / RAND_MAX * radius;
		float x = r * cos(angle);
		float z = r * sin(angle);
		float y = (float)rand() / RAND_MAX * height;

		// 随机缩放（0.8~1.2倍）
		float scale = 0.01f;

		// 随机旋转（Y轴）
		float rotY = (float)rand() / RAND_MAX * 2 * M_PI;

		// 构建实例的世界矩阵：缩放 -> 旋转 -> 平移
		Matrix scaleMat = Matrix::scaling(Vec3(scale, scale, scale));
		Matrix rotMat = Matrix::rotateZ(0.f)*Matrix::rotateY(rotY)* Matrix::rotateX(0.f);
		Matrix transMat = Matrix::translation(Vec3(x, y, z));
		Matrix instanceMat = scaleMat* rotMat * transMat;

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
	water->CreateFromPlane(myWorld->GetCore(),10000,1000,1000);
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
	fps_Mesh->SetWorldScaling(Vec3(1.f, 1.f, 1.f));
	animatedInstance = new AnimationInstance();
	animatedInstance->init(&fps_Mesh->animation, 0);
}

void FPSActor::draw()
{
	World* myWorld = World::Get();
	fps_Mesh->drawSingle(myWorld->GetCore(), myWorld->GetPSOManager(), ANIM_PIPE, myWorld->GetPipelines(), animatedInstance, myWorld->GetDeltatime());
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
