#pragma once
#include "Mesh.h"
#include <string>
class AnimationStateMachine;

// ===================== 动画状态基类 =====================
class AnimationState {
protected:
	AnimationStateMachine* m_stateMachine; // 所属状态机
	std::string m_stateName;               // 状态名（如"Idle"）
	std::string m_animName;                // 对应动画名（如"idle"）
	bool m_isLoop;                         // 是否循环播放
	float m_animSpeed;                     // 动画播放速度

public:
	AnimationState(AnimationStateMachine* sm, const std::string stateName, const std::string animName, bool isLoop = true, float animSpeed = 1.0f)
		: m_stateMachine(sm), m_stateName(stateName), m_animName(animName), m_isLoop(isLoop), m_animSpeed(animSpeed) {}
	virtual ~AnimationState() = default;

	// 状态进入逻辑（首次进入时调用）
	virtual void OnEnter() {}
	// 状态更新逻辑（每帧调用）
	virtual void OnUpdate(float dt) {}
	// 状态退出逻辑（切换状态时调用）
	virtual void OnExit() {}

	// 获取属性
	const std::string& GetStateName() const { return m_stateName; }
	const std::string& GetAnimName() const { return m_animName; }
	bool IsLoop() const { return m_isLoop; }
	float GetAnimSpeed() const { return m_animSpeed; }
	AnimationStateMachine* GetStateMachine() const;
};
// -------------------- 具体状态实现 --------------------
// Idle状态
class FPSIdleState : public AnimationState {
public:
	FPSIdleState(AnimationStateMachine* sm)
		: AnimationState(sm, "Idle", "04 idle", true, 1.0f) {}
	void OnUpdate(float dt) override;
};

// Walk状态
class FPSWalkState : public AnimationState {
public:
	FPSWalkState(AnimationStateMachine* sm)
		: AnimationState(sm, "Walk", "06 walk", true, 1.2f) {}
	void OnUpdate(float dt) override;
};

// Reload状态（单次播放）
class FPSReloadState : public AnimationState {
public:
	FPSReloadState(AnimationStateMachine* sm)
		: AnimationState(sm, "Reload", "17 reload", false, 1.0f) {}
	void OnEnter() override;
	void OnUpdate(float dt) override;
};

// Fire状态（单次播放）
class FPSFireState : public AnimationState {
public:
	FPSFireState(AnimationStateMachine* sm)
		: AnimationState(sm, "Fire", "08 fire", false, 1.0f) {}
	void OnEnter() override;
	void OnUpdate(float dt) override;
};