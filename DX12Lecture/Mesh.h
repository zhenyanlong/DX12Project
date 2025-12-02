#pragma once
#include "Core.h"
#include "ScreenSpaceTriangle.h"
#include "VertexLayoutCache.h"

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
	void draw(Core* core);

	static void CreatePlane(Core* core, Mesh* plane);

	static void CreateCube(Core* core, Mesh* cube);

	static void CreateSphere(Core* core, Mesh* sphere, int rings, int segments, float radius);

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

