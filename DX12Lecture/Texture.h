#pragma once

#include "iostream"
#include "Core.h"
class Texture
{

public:
	ID3D12Resource* tex;
	int heapOffset;

	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	Texture(Core* core, std::string filename);
};

