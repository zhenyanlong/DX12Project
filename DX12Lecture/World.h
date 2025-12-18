#pragma once
#include "Pipeline.h"
#include "VertexLayoutCache.h"
#include "Levels/Level.h"


class Timer
{
private:
	LARGE_INTEGER freq;   // Frequency of the performance counter
	LARGE_INTEGER start;  // Starting time

public:
	// Constructor that initializes the frequency
	Timer()
	{
		QueryPerformanceFrequency(&freq);
		reset();
	}

	// Resets the timer
	void reset()
	{
		QueryPerformanceCounter(&start);
	}

	// Returns the elapsed time since the last reset in seconds. Note this should only be called once per frame as it resets the timer.
	float dt()
	{
		LARGE_INTEGER cur;
		QueryPerformanceCounter(&cur);
		float value = static_cast<float>(cur.QuadPart - start.QuadPart) / freq.QuadPart;
		reset();
		return value;
	}
};

const std::string STATIC_PIPE = "Static_Mesh";	// 基础：StaticMesh
const std::string STATIC_LIGHT_PIPE = "Static_Mesh_Light";	// StaticMesh + Light
const std::string STATIC_INSTANCE_PIPE = "Static_Instance";	// StaticMesh + Instance
const std::string ANIM_PIPE = "Animation";	// Animation
const std::string STATIC_INSTANCE_LIGHT_PIPE = "Static_Mesh_Instance_Light"; // StaticMesh + Instance + Light
const std::string ANIM_LIGHT_PIPE = "Animation_Light"; // Animation + Light
const std::string STATIC_LIGHT_WATER_PIPE = "Static_Mesh_Light_Water";

const std::string VS_PATH = "VertexShader.hlsl";
const std::string VS_BIT_PATH = "Shaders/VertexShaderBitangent.hlsl";
const std::string VS_ANIM_PATH = "Shaders/VertexShaderAnim.hlsl";
const std::string VS_INS_PATH = "Shaders/VertexShaderInstanced.hlsl";
const std::string VS_INS_BIT_PATH = "Shaders/VertexShaderInstBit.hlsl";
const std::string PS_PATH = "PixelShaderTextured.hlsl";
const std::string PS_LIGHT_PATH = "Shaders/PixelShaderLightTextured.hlsl";
const std::string VS_WATER_PATH = "Shaders/VertexShaderWaveAnim.hlsl";
const std::string PS_WATER_PATH = "Shaders/PixelShaderWaveAnim.hlsl";


class World
{
	// singleton
	static World* SingleInstance;

	World(Core& core)
	{
		this->core = &core;
		// init PSO_Manager
		m_psos = new PSOManager();
		// init pipelines
		m_pipes = new Pipelines();
		m_pipes->loadPipeline(core, STATIC_PIPE, m_psos, VS_PATH, PS_PATH, VertexLayoutCache::getStaticLayout());									// static mesh no light
		m_pipes->loadPipeline(core, ANIM_PIPE, m_psos, VS_ANIM_PATH, PS_PATH, VertexLayoutCache::getAnimatedLayout());								// anim mesh no light
		m_pipes->loadPipeline(core, STATIC_INSTANCE_PIPE, m_psos, VS_INS_PATH, PS_PATH, VertexLayoutCache::getInstanceLayout());					// static instance mesh no light
		m_pipes->loadPipeline(core, STATIC_LIGHT_PIPE, m_psos, VS_BIT_PATH, PS_LIGHT_PATH, VertexLayoutCache::getStaticLayout());					// static mesh with light
		m_pipes->loadPipeline(core, STATIC_INSTANCE_LIGHT_PIPE, m_psos, VS_INS_BIT_PATH, PS_LIGHT_PATH, VertexLayoutCache::getInstanceLayout());	// static instance mesh with light
		m_pipes->loadPipeline(core, STATIC_LIGHT_WATER_PIPE, m_psos, VS_WATER_PATH, PS_WATER_PATH, VertexLayoutCache::getStaticLayout());			// static water anim with light
	}
	// core
	Core* core;
	// pipelines
	Pipelines* m_pipes;
	// PSO
	PSOManager* m_psos;
	// Current Level 
	std::shared_ptr<Level> m_currentLevel;
	// Timer
	Timer timer;
	float cultime = 0;
	float dt = 0;
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

	inline Core* GetCore()
	{
		return core;
	}

	inline Pipelines* GetPipelines()
	{
		return m_pipes;
	}

	inline PSOManager* GetPSOManager()
	{
		return m_psos;
	} 

	// level getter and setter
	inline std::shared_ptr<Level> GetLevel()
	{
		return m_currentLevel;
	}
	inline void LoadNewLevel(std::shared_ptr<Level> level)
	{
		m_currentLevel = level;
	}


	// 新增：添加Actor到世界
	void addActor(std::string name, Actor* actor)
	{
		SingleInstance->GetLevel()->AddActor(name, actor);
		//actor->BeginPlay();
	}
	void garbageCollection() {
		SingleInstance->GetLevel()->garbageColloection();
	}
	// 新增：获取所有可碰撞Actor
	std::vector<Actor*> getCollidableActors() const
	{
		std::vector<Actor*> collidable;
		for (auto actor : SingleInstance->GetLevel()->GetAllActors())
		{
			
			if (actor.second->isCollidable())
				collidable.push_back(actor.second);
		}
		return collidable;
	}

	// time getter and setter
	void UpdateTime()
	{
		dt = timer.dt();
		cultime += dt;
	}
	inline float GetDeltatime()
	{
		return dt;
	}
	inline float GetCultime()
	{
		return cultime;
	}
	// execute begin play, tick, and draw
	void ExecuteBeginPlays()
	{
		m_currentLevel->BeginPlayInLevel();
	}

	void ExecuteTicks()
	{
		m_currentLevel->TickInLevel(dt);
	}

	void ExecuteDraw()
	{
		// level draw
		m_currentLevel->draw();
	}
};

