#pragma once
#include "Mesh.h"
#include <string>
class AnimationStateMachine;

// **** Animation State Base Class ****//
class AnimationState {
protected:
	AnimationStateMachine* m_stateMachine; 
	std::string m_stateName;              
	std::string m_animName;                
	bool m_isLoop;                         
	float m_animSpeed;                     

public:
	AnimationState(AnimationStateMachine* sm, const std::string stateName, const std::string animName, bool isLoop = true, float animSpeed = 1.0f)
		: m_stateMachine(sm), m_stateName(stateName), m_animName(animName), m_isLoop(isLoop), m_animSpeed(animSpeed) {}
	virtual ~AnimationState() = default;

	// state enter
	virtual void OnEnter() {}
	// state update
	virtual void OnUpdate(float dt) {}
	// state exit
	virtual void OnExit() {}

	// get state info
	const std::string& GetStateName() const { return m_stateName; }
	const std::string& GetAnimName() const { return m_animName; }
	bool IsLoop() const { return m_isLoop; }
	float GetAnimSpeed() const { return m_animSpeed; }
	AnimationStateMachine* GetStateMachine() const;
};

// Idle state
class FPSIdleState : public AnimationState {
public:
	FPSIdleState(AnimationStateMachine* sm)
		: AnimationState(sm, "Idle", "04 idle", true, 1.0f) {}
	void OnUpdate(float dt) override;
};

// Walk state
class FPSWalkState : public AnimationState {
public:
	FPSWalkState(AnimationStateMachine* sm)
		: AnimationState(sm, "Walk", "06 walk", true, 1.2f) {}
	void OnUpdate(float dt) override;
};

// Reload state
class FPSReloadState : public AnimationState {
public:
	FPSReloadState(AnimationStateMachine* sm)
		: AnimationState(sm, "Reload", "17 reload", false, 1.0f) {}
	void OnEnter() override;
	void OnUpdate(float dt) override;
};

// Fire State
class FPSFireState : public AnimationState {
public:
	FPSFireState(AnimationStateMachine* sm)
		: AnimationState(sm, "Fire", "08 fire", false, 1.0f) {}
	void OnEnter() override;
	void OnUpdate(float dt) override;
};

//enemy state
class DuckIdlestate : public AnimationState
{
public:
	DuckIdlestate(AnimationStateMachine* sm)
		: AnimationState(sm, "Idle", "idle", true, 1.0f) {}
	
	void OnUpdate(float dt) override;
};

class DuckDeathState : public AnimationState
{
public:
	DuckDeathState(AnimationStateMachine* sm)
		: AnimationState(sm, "Death", "death", false, 1.0f) {}
	void OnEnter() override;
	void OnUpdate(float dt) override;
};