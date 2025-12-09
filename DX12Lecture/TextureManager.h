#pragma once
#include "Texture.h"
#include "map"
#include "string"





class TextureManager
{
	// singleton
	static TextureManager* SingleInstance;
	TextureManager() = default;
public:
	// delete copy
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;

	//Get single instance pointer
	static TextureManager* Get() {
		return SingleInstance;
	}

	//Create TextureManager Single Instance
	static TextureManager* Create()
	{
		if (SingleInstance == nullptr)
		{
			SingleInstance = new TextureManager();
		}
		return SingleInstance;
	}

public:
	
	std::map<std::string, Texture*> textures;

	Texture* loadTexture(Core* core, const std::string& filename) {
		auto it = textures.find(filename);
		if (it != textures.end()) {
			return it->second;
		}

		// 加载新纹理并存储
		auto texture = new Texture(core, filename);
		//texture->load(core, filename);
		Texture* ptr = texture;
		textures[filename] = texture;
		return ptr;
	}
};

