#pragma once
#include "Pipeline.h"
#include "VertexLayoutCache.h"

const std::string staticPipe = "staticMesh";
const std::string animpipe = "Animation";
const std::string vsName = "VertexShader.hlsl";
const std::string vsAnimPath = "Shaders/VertexShaderAnim.hlsl";
const std::string psName = "PixelShaderTextured.hlsl";

class World
{
	// singleton
	static World* SingleInstance;

	World(Core& core)
	{
		// init PSO_Manager
		m_psos = new PSOManager();
		// init pipelines
		m_pipes = new Pipelines();
		m_pipes->loadPipeline(core, staticPipe, m_psos, vsName, psName, VertexLayoutCache::getStaticLayout());
		m_pipes->loadPipeline(core, animpipe, m_psos, vsAnimPath, psName, VertexLayoutCache::getAnimatedLayout());
	}
	// core
	Core* core;
	// pipelines
	Pipelines* m_pipes;
	// PSO
	PSOManager* m_psos;
			

public:
	// delete copy
	World(const World&) = delete;
	World& operator=(const World&) = delete;

	//Get single instance pointer
	static World* Get() {
		return SingleInstance;
	}

	//Create World Single Instance
	static World* Create(Core& core)
	{
		if (SingleInstance == nullptr)
		{
			SingleInstance = new World(core);
		}
		return SingleInstance;
	}

	inline Pipelines* GetPipelines()
	{
		return m_pipes;
	}

	inline PSOManager* GetPSOManager()
	{
		return m_psos;
	}
};

