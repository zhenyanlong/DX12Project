#include "AnimationState.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/EnemyAnimationStateMachine.h"
AnimationStateMachine* AnimationState::GetStateMachine() const
{
	return m_stateMachine;
}

#include "Animation/FPSAnimationStateMachine.h"

void FPSIdleState::OnUpdate(float dt) {
	auto fpsSM = dynamic_cast<FPSAnimationStateMachine*>(GetStateMachine());
	if (fpsSM && fpsSM->m_isMoving) {
		fpsSM->ChangeState("Walk"); 
	}
}

// Walk
void FPSWalkState::OnUpdate(float dt) {
	auto fpsSM = dynamic_cast<FPSAnimationStateMachine*>(GetStateMachine());
	if (fpsSM && !fpsSM->m_isMoving) {
		fpsSM->ChangeState("Idle"); 
	}
}

// Reload
void FPSReloadState::OnEnter() {
	auto fpsSM = dynamic_cast<FPSAnimationStateMachine*>(GetStateMachine());
	fpsSM->m_animInstance->resetAnimationTime();
	if (fpsSM) {
		fpsSM->m_isReloading = true;
	}
}


void FPSReloadState::OnUpdate(float dt) {
	auto fpsSM = dynamic_cast<FPSAnimationStateMachine*>(GetStateMachine());

	if (fpsSM && fpsSM->m_animInstance->animationFinished()) {
		fpsSM->m_isReloading = false;
		
		fpsSM->ChangeState(fpsSM->m_prevState);
		
		fpsSM->m_animInstance->resetAnimationTime();
	}
}

// Fire
void FPSFireState::OnEnter() {
	auto fpsSM = dynamic_cast<FPSAnimationStateMachine*>(GetStateMachine());
	fpsSM->m_animInstance->resetAnimationTime();
	if (fpsSM) {
		fpsSM->m_isFiring = true;
	}
}


void FPSFireState::OnUpdate(float dt) {
	auto fpsSM = dynamic_cast<FPSAnimationStateMachine*>(GetStateMachine());
	if (fpsSM && fpsSM->m_animInstance->animationFinished()) {
		fpsSM->m_isFiring = false;
		
		fpsSM->ChangeState(fpsSM->m_prevState);
		
		fpsSM->m_animInstance->resetAnimationTime();
	}
}

void DuckIdlestate::OnUpdate(float dt)
{
	auto duckSM = dynamic_cast<EnemyAnimationStateMachine*>(GetStateMachine());
	if (duckSM) {
		duckSM->ChangeState("Idle"); 
	}
}

void DuckDeathState::OnEnter()
{
	auto duckSM = dynamic_cast<EnemyAnimationStateMachine*>(GetStateMachine());
	duckSM->m_animInstance->resetAnimationTime();
	if (duckSM) {
		duckSM->m_death = true;
	}
}

void DuckDeathState::OnUpdate(float dt)
{
	auto duckSM = dynamic_cast<EnemyAnimationStateMachine*>(GetStateMachine());
	if (duckSM && duckSM->m_animInstance->animationFinished()) {
		duckSM->m_death = false;
		duckSM->m_deathFinished = true;
		


	}
}
