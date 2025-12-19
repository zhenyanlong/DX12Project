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
		// ×¢²áËùÓÐ×´Ì¬
		RegisterState(new DuckIdlestate(this));
		RegisterState(new DuckDeathState(this));
		
		// ³õÊ¼×´Ì¬£ºIdle
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

