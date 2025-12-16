#include "Mesh.h"
#include "GEMLoader.h"
#include "TextureManager.h"
#include "World.h"
#include "StringUtils.h"
void Mesh::init(Core* core, void* vertices, int vertexSizeInBytes, int numVertices,
	unsigned int* indices, int numIndices)

{
	D3D12_HEAP_PROPERTIES heapprops = {};
	heapprops.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapprops.CreationNodeMask = 1;
	heapprops.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC vbDesc = {};
	vbDesc.Width = numVertices * vertexSizeInBytes;
	vbDesc.Height = 1;
	vbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vbDesc.DepthOrArraySize = 1;
	vbDesc.MipLevels = 1;
	vbDesc.SampleDesc.Count = 1;
	vbDesc.SampleDesc.Quality = 0;
	vbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &vbDesc,
		D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&vertexBuffer));

	core->uploadResource(vertexBuffer, vertices, numVertices * vertexSizeInBytes,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	// init view
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.StrideInBytes = vertexSizeInBytes;
	vbView.SizeInBytes = numVertices * vertexSizeInBytes;

	/*inputLayout[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout[1] = { "COLOUR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayoutDesc.NumElements = 2;
	inputLayoutDesc.pInputElementDescs = inputLayout;*/

	D3D12_RESOURCE_DESC ibDesc;
	memset(&ibDesc, 0, sizeof(D3D12_RESOURCE_DESC));
	ibDesc.Width = numIndices * sizeof(unsigned int);
	ibDesc.Height = 1;
	ibDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ibDesc.DepthOrArraySize = 1;
	ibDesc.MipLevels = 1;
	ibDesc.SampleDesc.Count = 1;
	ibDesc.SampleDesc.Quality = 0;
	ibDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	HRESULT hr = core->device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &ibDesc,
		D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&indexBuffer));
	core->uploadResource(indexBuffer, indices, numIndices * sizeof(unsigned int),
		D3D12_RESOURCE_STATE_INDEX_BUFFER);

	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R32_UINT;
	ibView.SizeInBytes = numIndices * sizeof(unsigned int);
	numMeshIndices = numIndices;


}

void Mesh::draw(Core* core)
{
	core->getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	core->getCommandList()->IASetVertexBuffers(0, 1, &vbView);
	core->getCommandList()->IASetIndexBuffer(&ibView);
	core->getCommandList()->DrawIndexedInstanced(numMeshIndices, 1, 0, 0, 0);

}

void Mesh::drawInstanced(Core* core, int instanceCount)
{
	core->getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	core->getCommandList()->IASetVertexBuffers(0, 1, &vbView);
	core->getCommandList()->IASetIndexBuffer(&ibView);
	core->getCommandList()->DrawIndexedInstanced(numMeshIndices, instanceCount, 0, 0, 0);
}

void Mesh::CreatePlane(Core* core, Mesh* plane)
{
	std::vector<STATIC_VERTEX> vertices;
	vertices.push_back(addVertex(Vec3(-15, 0, -15), Vec3(0, 1, 0), 0, 0));
	vertices.push_back(addVertex(Vec3(15, 0, -15), Vec3(0, 1, 0), 1, 0));
	vertices.push_back(addVertex(Vec3(-15, 0, 15), Vec3(0, 1, 0), 0, 1));
	vertices.push_back(addVertex(Vec3(15, 0, 15), Vec3(0, 1, 0), 1, 1));
	std::vector<unsigned int> indices;
	indices.push_back(2); indices.push_back(1); indices.push_back(0);
	indices.push_back(1); indices.push_back(2); indices.push_back(3);
	plane->init(core, vertices, indices);
}

void Mesh::CreateCube(Core* core, Mesh* cube)
{
	// init point
	std::vector<STATIC_VERTEX> vertices;
	Vec3 p0 = Vec3(-1.0f, -1.0f, -1.0f);
	Vec3 p1 = Vec3(1.0f, -1.0f, -1.0f);
	Vec3 p2 = Vec3(1.0f, 1.0f, -1.0f);
	Vec3 p3 = Vec3(-1.0f, 1.0f, -1.0f);
	Vec3 p4 = Vec3(-1.0f, -1.0f, 1.0f);
	Vec3 p5 = Vec3(1.0f, -1.0f, 1.0f);
	Vec3 p6 = Vec3(1.0f, 1.0f, 1.0f);
	Vec3 p7 = Vec3(-1.0f, 1.0f, 1.0f);
	// init vertices
	vertices.push_back(addVertex(p0, Vec3(0.0f, 0.0f, -1.0f), 0.0f, 1.0f));
	vertices.push_back(addVertex(p1, Vec3(0.0f, 0.0f, -1.0f), 1.0f, 1.0f));
	vertices.push_back(addVertex(p2, Vec3(0.0f, 0.0f, -1.0f), 1.0f, 0.0f));
	vertices.push_back(addVertex(p3, Vec3(0.0f, 0.0f, -1.0f), 0.0f, 0.0f));

	vertices.push_back(addVertex(p5, Vec3(0.0f, 0.0f, 1.0f), 0.0f, 1.0f));
	vertices.push_back(addVertex(p4, Vec3(0.0f, 0.0f, 1.0f), 1.0f, 1.0f));
	vertices.push_back(addVertex(p7, Vec3(0.0f, 0.0f, 1.0f), 1.0f, 0.0f));
	vertices.push_back(addVertex(p6, Vec3(0.0f, 0.0f, 1.0f), 0.0f, 0.0f));

	vertices.push_back(addVertex(p4, Vec3(-1.0f, 0.0f, 0.0f), 0.0f, 1.0f));
	vertices.push_back(addVertex(p0, Vec3(-1.0f, 0.0f, 0.0f), 1.0f, 1.0f));
	vertices.push_back(addVertex(p3, Vec3(-1.0f, 0.0f, 0.0f), 1.0f, 0.0f));
	vertices.push_back(addVertex(p7, Vec3(-1.0f, 0.0f, 0.0f), 0.0f, 0.0f));

	vertices.push_back(addVertex(p1, Vec3(1.0f, 0.0f, 0.0f), 0.0f, 1.0f));
	vertices.push_back(addVertex(p5, Vec3(1.0f, 0.0f, 0.0f), 1.0f, 1.0f));
	vertices.push_back(addVertex(p6, Vec3(1.0f, 0.0f, 0.0f), 1.0f, 0.0f));
	vertices.push_back(addVertex(p2, Vec3(1.0f, 0.0f, 0.0f), 0.0f, 0.0f));

	vertices.push_back(addVertex(p3, Vec3(0.0f, 1.0f, 0.0f), 0.0f, 1.0f));
	vertices.push_back(addVertex(p2, Vec3(0.0f, 1.0f, 0.0f), 1.0f, 1.0f));
	vertices.push_back(addVertex(p6, Vec3(0.0f, 1.0f, 0.0f), 1.0f, 0.0f));
	vertices.push_back(addVertex(p7, Vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f));

	vertices.push_back(addVertex(p4, Vec3(0.0f, -1.0f, 0.0f), 0.0f, 1.0f));
	vertices.push_back(addVertex(p5, Vec3(0.0f, -1.0f, 0.0f), 1.0f, 1.0f));
	vertices.push_back(addVertex(p1, Vec3(0.0f, -1.0f, 0.0f), 1.0f, 0.0f));
	vertices.push_back(addVertex(p0, Vec3(0.0f, -1.0f, 0.0f), 0.0f, 0.0f));
	// init indices
	std::vector<unsigned int> indices;
	indices.push_back(0); indices.push_back(1); indices.push_back(2);
	indices.push_back(0); indices.push_back(2); indices.push_back(3);
	indices.push_back(4); indices.push_back(5); indices.push_back(6);
	indices.push_back(4); indices.push_back(6); indices.push_back(7);
	indices.push_back(8); indices.push_back(9); indices.push_back(10);
	indices.push_back(8); indices.push_back(10); indices.push_back(11);
	indices.push_back(12); indices.push_back(13); indices.push_back(14);
	indices.push_back(12); indices.push_back(14); indices.push_back(15);
	indices.push_back(16); indices.push_back(17); indices.push_back(18);
	indices.push_back(16); indices.push_back(18); indices.push_back(19);
	indices.push_back(20); indices.push_back(21); indices.push_back(22);
	indices.push_back(20); indices.push_back(22); indices.push_back(23);

	cube->init(core, vertices, indices);
}

void Mesh::CreateSphere(Core* core, Mesh* sphere, int rings, int segments, float radius)
{
	std::vector<STATIC_VERTEX> vertices;
	for (int lat = 0; lat <= rings; lat++) {
		float theta = lat * PI / rings;
		float sinTheta = sinf(theta);
		float cosTheta = cosf(theta);
		for (int lon = 0; lon <= segments; lon++) {
			float phi = lon * 2.0f * PI / segments;
			float sinPhi = sinf(phi);
			float cosPhi = cosf(phi);
			Vec3 position(radius * sinTheta * cosPhi, radius * cosTheta,
				radius * sinTheta * sinPhi);
			Vec3 normal = position.normalize();
			float tu = 1.0f - (float)lon / segments;
			float tv = 1.0f - (float)lat / rings;
			vertices.push_back(addVertex(position, normal, tu, tv));
		}
	}
	std::vector<unsigned int> indices;
	for (int lat = 0; lat < rings; lat++)
	{
		for (int lon = 0; lon < segments; lon++)
		{
			int current = lat * (segments + 1) + lon;
			int next = current + segments + 1;
			indices.push_back(current);
			indices.push_back(next);
			indices.push_back(current + 1);
			indices.push_back(current + 1);
			indices.push_back(next);
			indices.push_back(next + 1);
		}
	}
	sphere->init(core, vertices, indices);
}

void Mesh::CreateGEM(Core* core, std::vector<Mesh>& meshes, std::string filename)
{
	GEMLoader::GEMModelLoader loader;
	std::vector<GEMLoader::GEMMesh> gemmeshes;
	loader.load(filename, gemmeshes);
	for (int i = 0; i < gemmeshes.size(); i++) {
		Mesh mesh;
		std::vector<STATIC_VERTEX> vertices;
		for (int j = 0; j < gemmeshes[i].verticesStatic.size(); j++) {
			STATIC_VERTEX v;
			memcpy(&v, &gemmeshes[i].verticesStatic[j], sizeof(STATIC_VERTEX));
			vertices.push_back(v);
		}
		mesh.init(core, vertices, gemmeshes[i].indices);
		meshes.push_back(mesh);
	}

}

StaticMesh::StaticMesh(Core* core, std::string filename)
{
	CreateFromGEM(core, filename);
}

void StaticMesh::CreateFromGEM(Core* core, std::string filename)
{
	GEMLoader::GEMModelLoader loader;
	std::vector<GEMLoader::GEMMesh> gemmeshes;
	TextureManager* texMgr = TextureManager::Get();

	loader.load(filename, gemmeshes);
	for (int i = 0; i < gemmeshes.size(); i++) {
		Mesh mesh;
		std::vector<STATIC_VERTEX> vertices;
		for (int j = 0; j < gemmeshes[i].verticesStatic.size(); j++) {
			STATIC_VERTEX v;
			memcpy(&v, &gemmeshes[i].verticesStatic[j], sizeof(STATIC_VERTEX));
			vertices.push_back(v);
		}
		textureFilenames.push_back(gemmeshes[i].material.find("albedo").getValue());
		texMgr->loadTexture(core, gemmeshes[i].material.find("albedo").getValue());
		// normal texture (nh)
		std::string normalPath = gemmeshes[i].material.find("nh").getValue();
		normalTextureFilenames.push_back(normalPath);
		texMgr->loadTexture(core, normalPath);

		mesh.init(core, vertices, gemmeshes[i].indices);
		meshes.push_back(mesh);
	}
}

void StaticMesh::draw(Core* core, PSOManager* psos, std::string pipeName, Pipelines* pipes, std::vector<Matrix>* instanceMatrices, int instanceCount)
{
	GeneralMatrix* gm = GeneralMatrix::Get();
	World* myWorld = World::Get();

	// ===== 步骤1：根据关键词执行Buffer更新策略（核心逻辑）=====
	// 策略组合：支持多个关键词同时生效
	bool isInstance = hasKeyword(pipeName, "Instance", { "Inst" }); // 兼容"Inst"缩写
	bool isLight = hasKeyword(pipeName, "Light");
	bool isAnim = hasKeyword(pipeName, "Anim");
	bool isStatic = hasKeyword(pipeName, "Static", { "StaticMesh" }); // 兼容"Static"缩写
	bool isWater = hasKeyword(pipeName, "Water", { "Wat" });

	// 1. 基础静态策略（StaticMesh）：仅当非实例化时执行
	if (isStatic && !isInstance)
	{
		Pipelines::updateBaseStaticBuffer(pipeName, pipes, m_worldPosMat);
	}

	// 2. 实例化策略（Instance）
	if (isInstance)
	{
		Pipelines::updateInstanceBuffer(pipeName, pipes, instanceMatrices);
	}

	// 3. 光照策略（Light）
	if (isLight)
	{
		Pipelines::updateLightBuffer(pipeName, pipes);
		// 提交PS常量缓冲区（光照）
		
	}

	// 4. 动画策略（Anim）：未来拓展
	if (isAnim)
	{
		//Pipelines::updateAnimBuffer(pipeName, pipes);
	}

	if (isWater)
	{
		Pipelines::updateWaveBuffer(pipeName, pipes);
	}


	// ===== 步骤2：提交VS常量缓冲区（公共）=====
	Pipelines::submitToCommandList(core, pipes->pipelines[pipeName].vsConstantBuffers);
	Pipelines::submitToCommandList(core, pipes->pipelines[pipeName].psConstantBuffers);
	
	
	// ===== 步骤3：执行公共绘制逻辑 =====
	drawCommon(core, psos, pipes, pipeName, instanceCount);
}

void StaticMesh::drawSingle(Core* core, PSOManager* const psos, std::string pipeName, Pipelines* const pipes)
{
	/*World* myWorld = World::Get();
	GeneralMatrix* gm = GeneralMatrix::Get();
	
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers, "staticMeshBuffer", "W", &m_worldPosMat);
	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers, "staticMeshBuffer", "VP", &gm->viewProjMatrix);
	Pipelines::submitToCommandList(core, pipes->pipelines[pipeName].vsConstantBuffers);
	
	TextureManager* texs = TextureManager::Get();
	for (int i=0;i<meshes.size();i++)
	{
		core->beginRenderPass();
		Pipelines::updateTexture(&pipes->pipelines[pipeName].textureBindPoints, core, "tex", texs->textures.find(textureFilenames[i])->second->heapOffset, myWorld->GetCore()->srvTableRootIndex);
		psos->bind(core, pipes->pipelines[pipeName].psoName);
		meshes[i].draw(core);
	}*/
	// 调用整合后的draw方法：单个绘制，无实例矩阵
	draw(core, psos, pipeName, pipes, nullptr, 1);
}

void StaticMesh::drawInstances(Core* core, PSOManager* const psos, std::string pipeName, Pipelines* const pipes, std::vector<Matrix>* const instanceMatrices, int instanceCount)
{
	
	/*	 GeneralMatrix* gm = GeneralMatrix::Get();


	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"staticMeshBuffer", "VP", &gm->viewProjMatrix);


	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
		"instanceBuffer", "instanceMatrices", instanceMatrices->data());


	Pipelines::submitToCommandList(core, pipes->pipelines[pipeName].vsConstantBuffers);


	TextureManager* texs = TextureManager::Get();
	for (int i = 0; i < meshes.size(); i++)
	{
		core->beginRenderPass();

		Pipelines::updateTexture(&pipes->pipelines[pipeName].textureBindPoints,
			core, "tex", texs->textures.find(textureFilenames[i])->second->heapOffset);

		psos->bind(core, pipes->pipelines[pipeName].psoName);

		meshes[i].drawInstanced(core, instanceCount);
	}*/
	// 调用整合后的draw方法：实例化绘制
	draw(core, psos, pipeName, pipes, instanceMatrices, instanceCount);
}

StaticMesh::StaticMesh()
{
}

void StaticMesh::CreateFromPlane(Core* core, float size, int xSegments, int zSegments, std::string texName)
{
	std::vector<STATIC_VERTEX> vertices;
	std::vector<unsigned int> indices;

	// 生成顶点（平面：x从-size/2到size/2，z从-size/2到size/2，y=0）
	for (int z = 0; z <= zSegments; z++)
	{
		for (int x = 0; x <= xSegments; x++)
		{
			float u = (float)x / xSegments;
			float v = (float)z / zSegments;
			float posX = (u - 0.5f) * size;
			float posZ = (v - 0.5f) * size;
			float posY = 0.0f;

			//STATIC_VERTEX vert;
			Vec3 Pos = Vec3(posX, posY, posZ);
			Vec3 Normal = Vec3(0.0f, 1.0f, 0.0f); // 初始法线向上
			Vec3 Tangent = Vec3(1.0f, 0.0f, 0.0f); // 初始切线向右
			//TexCoords = DirectX::XMFLOAT2(u, v);
			
			vertices.push_back(Mesh::addVertex(Pos, Normal, u ,v, Tangent));
		}
	}
	
	TextureManager* texMgr = TextureManager::Get();
	textureFilenames.push_back(texName);
	texMgr->loadTexture(core, texName);
	
	

	// 生成索引（三角面）
	for (int z = 0; z < zSegments; z++)
	{
		for (int x = 0; x < xSegments; x++)
		{
			int i0 = z * (xSegments + 1) + x;
			int i1 = z * (xSegments + 1) + x + 1;
			int i2 = (z + 1) * (xSegments + 1) + x;
			int i3 = (z + 1) * (xSegments + 1) + x + 1;

			// 两个三角面
			indices.push_back(i0);
			indices.push_back(i2);
			indices.push_back(i1);
			indices.push_back(i1);
			indices.push_back(i2);
			indices.push_back(i3);
		}
	}

	// 创建Mesh（复用原有meshes的初始化逻辑）
	Mesh waterMesh;
	waterMesh.init(core, vertices, indices);
	meshes.push_back(waterMesh);
}

void StaticMesh::drawCommon(Core* core, PSOManager* psos, Pipelines* pipes, const std::string& pipeName, int instanceCount)
{
	TextureManager* texs = TextureManager::Get();
	for (int i = 0; i < meshes.size(); i++)
	{
		core->beginRenderPass();

		// 纹理绑定（兼容原有逻辑，若有多个纹理可拓展）
		//Pipelines::updateTexture(&pipes->pipelines[pipeName].textureBindPoints,
		//	core, "tex", texs->textures.find(textureFilenames[i])->second->heapOffset,
		//	World::Get()->GetCore()->srvTableRootIndex); // 若有重载则传参，否则省略
		//if (!normalTextureFilenames.empty())
		//{
		//	Pipelines::updateTexture(&pipes->pipelines[pipeName].textureBindPoints,
		//		core, "normalTex", texs->textures.find(normalTextureFilenames[i])->second->heapOffset,
		//		World::Get()->GetCore()->srvTableRootIndex); // 若有重载则传参，否则省略
		//}
		
		std::map<std::string, int> textureHeapOffsets;

		if (i<textureFilenames.size())
		{
			textureHeapOffsets["tex"] = texs->textures[textureFilenames[i]]->heapOffset;
		}
		
		// 法线贴图（t1）
		if (i<normalTextureFilenames.size())
		{
			textureHeapOffsets["normalTex"] = texs->textures[normalTextureFilenames[i]]->heapOffset;
		}
		
		
		Pipelines::updateTexture(&pipes->pipelines[pipeName].textureBindPoints, core, textureHeapOffsets, World::Get()->GetCore()->srvTableRootIndex);
		// 绑定PSO
		psos->bind(core, pipes->pipelines[pipeName].psoName);

		// 绘制：单个/实例化
		if (instanceCount > 1)
		{
			meshes[i].drawInstanced(core, instanceCount);
		}
		else
		{
			meshes[i].draw(core);
		}
	}
}

void StaticMesh::CreateFromSphere(Core* core, int rings, int segments, float radius, std::string skyPath)
{
	// create sphere's vertices and indices
	Mesh sphere;
	TextureManager* texMgr = TextureManager::Get();
	std::vector<STATIC_VERTEX> vertices;
	for (int lat = 0; lat <= rings; lat++) {
		float theta = lat * PI / rings;
		float sinTheta = sinf(theta);
		float cosTheta = cosf(theta);
		for (int lon = 0; lon <= segments; lon++) {
			float phi = lon * 2.0f * PI / segments;
			float sinPhi = sinf(phi);
			float cosPhi = cosf(phi);
			Vec3 position(radius * sinTheta * cosPhi, radius * cosTheta,
				radius * sinTheta * sinPhi);
			Vec3 normal = position.normalize();
			float tu = 1.0f - (float)lon / segments;
			float tv = 1.0f - (float)lat / rings;
			vertices.push_back(Mesh::addVertex(position, normal, tu, tv));
		}
	}
	std::vector<unsigned int> indices;
	for (int lat = 0; lat < rings; lat++)
	{
		for (int lon = 0; lon < segments; lon++)
		{
			int current = lat * (segments + 1) + lon;
			int next = current + segments + 1;
			indices.push_back(current);
			indices.push_back(next);
			indices.push_back(current + 1);
			indices.push_back(current + 1);
			indices.push_back(next);
			indices.push_back(next + 1);
		}
	}
	// load texture
	textureFilenames.push_back(skyPath);
	texMgr->loadTexture(core, skyPath);
	// init sphere
	sphere.init(core, vertices, indices);
	// load mesh
	meshes.push_back(sphere);
}


GeneralMatrix* GeneralMatrix::SingleInstance = nullptr;

AnimatedModel::AnimatedModel(Core* core, std::string filename)
{
	CreateFromGEM(core, filename);
}

void AnimatedModel::CreateFromGEM(Core* core, std::string filename)
{
	GEMLoader::GEMModelLoader loader;
	std::vector<GEMLoader::GEMMesh> gemmeshes;
	GEMLoader::GEMAnimation gemanimation;
	TextureManager* texMgr = TextureManager::Get();

	loader.load(filename, gemmeshes, gemanimation);
	for (int i = 0; i < gemmeshes.size(); i++)
	{
		Mesh* mesh = new Mesh();
		std::vector<ANIMATED_VERTEX> vertices;
		for (int j = 0; j < gemmeshes[i].verticesAnimated.size(); j++)
		{
			ANIMATED_VERTEX v;
			memcpy(&v, &gemmeshes[i].verticesAnimated[j], sizeof(ANIMATED_VERTEX));
			vertices.push_back(v);
		}
		textureFilenames.push_back(gemmeshes[i].material.find("albedo").getValue());
		texMgr->loadTexture(core, gemmeshes[i].material.find("albedo").getValue());

		// 新增：加载法线贴图
		std::string normalPath = gemmeshes[i].material.find("nh").getValue();
		normalTextureFilenames.push_back(normalPath);
		texMgr->loadTexture(core, normalPath);

		mesh->init(core, vertices, gemmeshes[i].indices);
		meshes.push_back(mesh);
	}

	memcpy(&animation.skeleton.globalInverse, &gemanimation.globalInverse, 16 * sizeof(float));
	for (int i = 0; i < gemanimation.bones.size(); i++)
	{
		Bone bone;
		bone.name = gemanimation.bones[i].name;
		memcpy(&bone.offset, &gemanimation.bones[i].offset, 16 * sizeof(float));
		bone.parentIndex = gemanimation.bones[i].parentIndex;
		animation.skeleton.bones.push_back(bone);
	}
	for (int i = 0; i < gemanimation.animations.size(); i++)
	{
		std::string name = gemanimation.animations[i].name;
		AnimationSequence aseq;
		aseq.ticksPerSecond = gemanimation.animations[i].ticksPerSecond;
		for (int j = 0; j < gemanimation.animations[i].frames.size(); j++)
		{
			AnimationFrame frame;
			for (int index = 0; index < gemanimation.animations[i].frames[j].positions.size(); index++)
			{
				Vec3 p;
				Quaternion q;
				Vec3 s;
				memcpy(&p, &gemanimation.animations[i].frames[j].positions[index], sizeof(Vec3));
				frame.positions.push_back(p);
				memcpy(&q, &gemanimation.animations[i].frames[j].rotations[index], sizeof(Quaternion));
				frame.rotations.push_back(q);
				memcpy(&s, &gemanimation.animations[i].frames[j].scales[index], sizeof(Vec3));
				frame.scales.push_back(s);
			}
			aseq.frames.push_back(frame);
		}
		animation.animations.insert({ name, aseq });
	}
}
void AnimatedModel::draw(Core* core, PSOManager* psos, std::string pipeName, Pipelines* pipes,
	AnimationInstance* instance, const std::string& animName, float dt, int instanceCount)
{
	// ===== 步骤1：动画实例的前置更新（核心：处理动画播放、骨骼矩阵计算）=====
	if (instance != nullptr)
	{
		// 检查动画是否存在，不存在则取第一个动画
		std::string targetAnimName = animName;
		if (!instance->animation->hasAnimation(targetAnimName))
		{
			auto animNames = instance->animation->getAllAnimationNames();
			if (!animNames.empty())
			{
				targetAnimName = animNames[0];
			}
			else
			{
				// 无动画时直接返回
				return;
			}
		}

		// 更新动画实例（推进时间、计算骨骼矩阵）
		instance->update(targetAnimName, dt);

		// 动画结束后重置时间（循环播放）
		if (instance->animationFinished())
		{
			instance->resetAnimationTime();
		}
	}

	// ===== 步骤2：关键词策略处理（参考StaticMesh，拓展动画逻辑）=====
	GeneralMatrix* gm = GeneralMatrix::Get();
	bool isStatic = hasKeyword(pipeName, "Static", { "StaticMesh" });    // 静态矩阵（W/VP）
	bool isAnim = hasKeyword(pipeName, "Animation", { "Anim" });          // 动画（骨骼矩阵）
	bool isLight = hasKeyword(pipeName, "Light");                        // 光照
	bool isInstance = hasKeyword(pipeName, "Instance", { "Inst" });      // 实例化（预留）
	bool isWater = hasKeyword(pipeName, "Water", { "Wat" });              // 水波纹（兼容）

	// 1. 静态缓冲区更新：W（世界矩阵）、VP（视图投影矩阵）
	if (isStatic && !isInstance)
	{
		Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
			"staticMeshBuffer", "W", &m_worldPosMat);
		Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
			"staticMeshBuffer", "VP", &gm->viewProjMatrix);
	}

	// 2. 动画缓冲区更新：骨骼矩阵（核心）
	if (isAnim && instance != nullptr)
	{
		Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
			"staticMeshBuffer", "W", &m_worldPosMat);
		Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
			"staticMeshBuffer", "VP", &gm->viewProjMatrix);
		Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers,
			"AnimMeshBuffer", "bones", instance->matrices);
	}

	// 3. 光照缓冲区更新（参考StaticMesh）
	if (isLight)
	{
		Pipelines::updateLightBuffer(pipeName, pipes);
	}

	// 4. 水波纹缓冲区更新（兼容，动画模型一般不用）
	if (isWater)
	{
		Pipelines::updateWaveBuffer(pipeName, pipes);
	}

	// 5. 实例化缓冲区更新（预留：后续实现动画模型的实例化）
	if (isInstance)
	{
		// Pipelines::updateInstanceBuffer(pipeName, pipes, instanceMatrices);
	}

	// ===== 步骤3：提交常量缓冲区到命令列表 =====
	Pipelines::submitToCommandList(core, pipes->pipelines[pipeName].vsConstantBuffers);
	Pipelines::submitToCommandList(core, pipes->pipelines[pipeName].psConstantBuffers);

	// ===== 步骤4：执行公共绘制逻辑 =====
	drawCommon(core, psos, pipes, pipeName, instance, instanceCount);
}

// ===== AnimatedModel的drawCommon公共绘制逻辑实现 =====
void AnimatedModel::drawCommon(Core* core, PSOManager* psos, Pipelines* pipes, const std::string& pipeName,
	AnimationInstance* instance, int instanceCount)
{
	TextureManager* texs = TextureManager::Get();
	World* myWorld = World::Get();

	for (int i = 0; i < meshes.size(); i++)
	{
		core->beginRenderPass();

		// ===== 纹理绑定（支持albedo和法线贴图，与StaticMesh一致）=====
		std::map<std::string, int> textureHeapOffsets;

		// Albedo纹理（tex）
		if (i < textureFilenames.size() && !textureFilenames[i].empty())
		{
			textureHeapOffsets["tex"] = texs->textures[textureFilenames[i]]->heapOffset;
		}

		// 法线贴图（normalTex）
		if (i < normalTextureFilenames.size() && !normalTextureFilenames[i].empty())
		{
			textureHeapOffsets["normalTex"] = texs->textures[normalTextureFilenames[i]]->heapOffset;
		}

		// 批量更新纹理绑定
		Pipelines::updateTexture(&pipes->pipelines[pipeName].textureBindPoints, core,
			textureHeapOffsets, myWorld->GetCore()->srvTableRootIndex);

		// ===== 绑定PSO =====
		psos->bind(core, pipes->pipelines[pipeName].psoName);

		// ===== 绘制：单个/实例化（动画模型暂默认单个，可拓展）=====
		if (meshes[i] != nullptr)
		{
			if (instanceCount > 1)
			{
				meshes[i]->drawInstanced(core, instanceCount);
			}
			else
			{
				meshes[i]->draw(core);
			}
		}
	}
}
void AnimatedModel::drawSingle(Core* core, PSOManager* const psos, std::string pipeName, Pipelines* const pipes, AnimationInstance* instance, float dt)
{
	//instance->update("walk", dt);
	//std::vector<std::string> names;
	//names = instance->animation->getAllAnimationNames();
	//if (instance->animationFinished() == true)
	//{
	//	instance->resetAnimationTime();
	//}
	//{
	//	GeneralMatrix* gm = GeneralMatrix::Get();
	//	
	//	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers, "staticMeshBuffer", "W", &m_worldPosMat);
	//	Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers, "staticMeshBuffer", "VP", &gm->viewProjMatrix);
	//	if (instance != nullptr)
	//	{
	//		Pipelines::updateConstantBuffer(pipes->pipelines[pipeName].vsConstantBuffers, "AnimMeshBuffer", "bones", instance->matrices);
	//	}

	//	Pipelines::submitToCommandList(core, pipes->pipelines[pipeName].vsConstantBuffers);
	//	//shaders->apply(core, "AnimatedUntextured");
	//	TextureManager* texs = TextureManager::Get();
	//	for (int i = 0; i < meshes.size(); i++)
	//	{
	//		core->beginRenderPass();
	//		Pipelines::updateTexture(&pipes->pipelines[pipeName].textureBindPoints, core, "tex", texs->textures.find(textureFilenames[i])->second->heapOffset);
	//		psos->bind(core, pipes->pipelines[pipeName].psoName);
	//		meshes[i]->draw(core);
	//	}
	//}

	draw(core, psos, pipeName, pipes, instance, "17 reload", dt, 1);
}
