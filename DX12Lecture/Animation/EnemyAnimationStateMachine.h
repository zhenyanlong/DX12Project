#pragma once		  
#include "Animation/AnimationStateMachine.h"
#include "Animation/AnimationState.h"
class EnemyAnimationStateMachine;

class DuckIdlestate;
class DuckDeathState;

class EnemyAnimationStateMachine : public AnimationStateMachine
{
private:
	bool m_death = false;
	bool m_deathFinished = false;
public:
	EnemyAnimationStateMachine(AnimatedModel* model, AnimationInstance* instance)
		: AnimationStateMachine(model, instance) {
		// register all states
		RegisterState(new DuckIdlestate(this));
		RegisterState(new DuckDeathState(this));
		
		// init Idle state
		ChangeState("Idle");
	}

	void TriggerDeath()
	{
		ChangeState("Death");
	}

	bool IsDeath() const { return m_death; }
	bool IsDeathFinished() const { return m_deathFinished; }
private:
	friend class DuckIdlestate;
	friend class DuckDeathState;
};

