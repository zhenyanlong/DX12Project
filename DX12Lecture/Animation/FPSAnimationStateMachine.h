#pragma once
#include "Animation/AnimationStateMachine.h"
#include "Animation/AnimationState.h"
class FPSAnimationStateMachine;

class FPSIdleState;
class FPSWalkState;
class FPSReloadState;
class FPSFireState;




class FPSAnimationStateMachine : public AnimationStateMachine {


private:
	bool m_isMoving;       
	bool m_isReloading = false;   
	bool m_isFiring = false;       
	std::string m_prevState; 

public:
	FPSAnimationStateMachine(AnimatedModel* model, AnimationInstance* instance)
		: AnimationStateMachine(model, instance), m_isMoving(false), m_isReloading(false), m_isFiring(false) {
		
		RegisterState(new FPSIdleState(this));
		RegisterState(new FPSWalkState(this));
		RegisterState(new FPSReloadState(this));
		RegisterState(new FPSFireState(this));
		
		ChangeState("Idle");
	}

	// api
	void SetMoving(bool isMoving) { m_isMoving = isMoving; }
	void TriggerReload() { 
		/*if (!m_isReloading && !m_isFiring)
		{ m_isReloading = true; m_prevState = GetCurrentState()->GetStateName(); ChangeState("Reload");
		} */
		ChangeState("Reload");
	}
	void TriggerFire() 
	{ 
		if (!m_isReloading && !m_isFiring) 
		{ 
			m_isFiring = true; m_prevState = GetCurrentState()->GetStateName(); 
			ChangeState("Fire"); 
		} 
	}

	// get info
	bool IsReloading() const { return m_isReloading; }
	bool IsFiring() const { return m_isFiring; }
private:
	friend class FPSIdleState;
	friend class FPSWalkState;
	friend class FPSReloadState;
	friend class FPSFireState;
};
