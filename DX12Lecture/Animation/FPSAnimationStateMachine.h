#pragma once
#include "Animation/AnimationStateMachine.h"
#include "Animation/AnimationState.h"
class FPSAnimationStateMachine;

class FPSIdleState;
class FPSWalkState;
class FPSReloadState;
class FPSFireState;



// -------------------- FPS状态机主类 --------------------
class FPSAnimationStateMachine : public AnimationStateMachine {


private:
	bool m_isMoving;       // 是否移动（控制Idle↔Walk）
	bool m_isReloading = false;    // 是否正在换弹
	bool m_isFiring = false;       // 是否正在射击
	std::string m_prevState; // 换弹/射击前的状态（用于恢复）

public:
	FPSAnimationStateMachine(AnimatedModel* model, AnimationInstance* instance)
		: AnimationStateMachine(model, instance), m_isMoving(false), m_isReloading(false), m_isFiring(false) {
		// 注册所有状态
		RegisterState(new FPSIdleState(this));
		RegisterState(new FPSWalkState(this));
		RegisterState(new FPSReloadState(this));
		RegisterState(new FPSFireState(this));
		// 初始状态：Idle
		ChangeState("Idle");
	}

	// 外部控制接口（供FPSActor调用）
	void SetMoving(bool isMoving) { m_isMoving = isMoving; }
	void TriggerReload() { 
		if (!m_isReloading && !m_isFiring) 
		{ m_isReloading = true; m_prevState = GetCurrentState()->GetStateName(); ChangeState("Reload"); 
		} 
		ChangeState("Reload");
	}
	void TriggerFire() { if (!m_isReloading && !m_isFiring) { m_isFiring = true; m_prevState = GetCurrentState()->GetStateName(); ChangeState("Fire"); } }

	// 获取状态标记
	bool IsReloading() const { return m_isReloading; }
	bool IsFiring() const { return m_isFiring; }
private:
	friend class FPSIdleState;
	friend class FPSWalkState;
	friend class FPSReloadState;
	friend class FPSFireState;
};
