#include "AnimationState.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/EnemyAnimationStateMachine.h"
AnimationStateMachine* AnimationState::GetStateMachine() const
{
	return m_stateMachine;
}

#include "Animation/FPSAnimationStateMachine.h"
// -------------------- 状态实现 --------------------
// Idle状态更新：检测移动，切换到Walk
void FPSIdleState::OnUpdate(float dt) {
	auto fpsSM = dynamic_cast<FPSAnimationStateMachine*>(GetStateMachine());
	if (fpsSM && fpsSM->m_isMoving) {
		fpsSM->ChangeState("Walk"); // 混合过渡到Walk
	}
}

// Walk状态更新：检测停止，切换到Idle
void FPSWalkState::OnUpdate(float dt) {
	auto fpsSM = dynamic_cast<FPSAnimationStateMachine*>(GetStateMachine());
	if (fpsSM && !fpsSM->m_isMoving) {
		fpsSM->ChangeState("Idle"); // 混合过渡到Idle
	}
}

// Reload状态进入：标记换弹中
void FPSReloadState::OnEnter() {
	auto fpsSM = dynamic_cast<FPSAnimationStateMachine*>(GetStateMachine());
	fpsSM->m_animInstance->resetAnimationTime();
	if (fpsSM) {
		fpsSM->m_isReloading = true;
	}
}

// Reload状态更新：检测动画结束，恢复之前的状态
void FPSReloadState::OnUpdate(float dt) {
	auto fpsSM = dynamic_cast<FPSAnimationStateMachine*>(GetStateMachine());

	if (fpsSM && fpsSM->m_animInstance->animationFinished()) {
		fpsSM->m_isReloading = false;
		// 恢复到换弹前的状态（Idle/Walk）
		fpsSM->ChangeState(fpsSM->m_prevState);
		// 重置动画时间
		fpsSM->m_animInstance->resetAnimationTime();
	}
}

// Fire状态进入：标记射击中
void FPSFireState::OnEnter() {
	auto fpsSM = dynamic_cast<FPSAnimationStateMachine*>(GetStateMachine());
	fpsSM->m_animInstance->resetAnimationTime();
	if (fpsSM) {
		fpsSM->m_isFiring = true;
	}
}

// Fire状态更新：检测动画结束，恢复之前的状态
void FPSFireState::OnUpdate(float dt) {
	auto fpsSM = dynamic_cast<FPSAnimationStateMachine*>(GetStateMachine());
	if (fpsSM && fpsSM->m_animInstance->animationFinished()) {
		fpsSM->m_isFiring = false;
		// 恢复到射击前的状态（Idle/Walk）
		fpsSM->ChangeState(fpsSM->m_prevState);
		// 重置动画时间
		fpsSM->m_animInstance->resetAnimationTime();
	}
}

void DuckIdlestate::OnUpdate(float dt)
{
	auto duckSM = dynamic_cast<EnemyAnimationStateMachine*>(GetStateMachine());
	if (duckSM) {
		duckSM->ChangeState("Idle"); // 混合过渡到Idle
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
		
		// 重置动画时间
		//duckSM->m_animInstance->resetAnimationTime();

	}
}
