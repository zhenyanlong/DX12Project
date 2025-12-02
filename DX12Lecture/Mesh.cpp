#include "Mesh.h"
#include "GEMLoader.h"
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
