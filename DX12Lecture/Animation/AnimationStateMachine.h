#pragma once
#include "Mesh.h"
#include <string>
#include <unordered_map>
#include <functional>
#include "Animation/AnimationState.h"



// **** Animation State machine Base Class ****
class AnimationStateMachine {
protected:
	AnimatedModel* m_animatedModel;       
	AnimationInstance* m_animInstance;     
	std::unordered_map<std::string, AnimationState*> m_states; 
	AnimationState* m_currentState;        
	AnimationState* m_targetState;        
	float m_blendTime;                     
	float m_currentBlendTime;              
	float m_weightCurrent;                
	float m_weightTarget;                 

	
	enum class BlendType {
		Linear, 
		EaseInOut 
	};
	BlendType m_blendType;

public:
	AnimationStateMachine(AnimatedModel* model, AnimationInstance* instance)
		: m_animatedModel(model), m_animInstance(instance), m_currentState(nullptr), m_targetState(nullptr),
		m_blendTime(0.2f), m_currentBlendTime(0.0f), m_weightCurrent(1.0f), m_weightTarget(0.0f),
		m_blendType(BlendType::Linear) {}
	virtual ~AnimationStateMachine() {
		
		for (auto& pair : m_states) {
			delete pair.second;
		}
		m_states.clear();
	}

	// init	states
	void RegisterState(AnimationState* state) {
		m_states[state->GetStateName()] = state;
	}

	// change state (not support blend animation)
	void ChangeState(const std::string& targetStateName, float blendTime = 0.2f) {
		auto it = m_states.find(targetStateName);
		if (it == m_states.end()) {
			return; 
		}
		// If the current state is not in the target state, directly set it to the target state
		if (m_currentState == nullptr) {
			m_currentState = it->second;
			m_currentState->OnEnter();
			m_targetState = nullptr;
			m_currentBlendTime = 0.0f;
			m_weightCurrent = 1.0f;
			m_weightTarget = 0.0f;
			return;
		}
		// If the target state is the current state, ignore
		if (m_currentState->GetStateName() == targetStateName) {
			return;
		}
		
		m_currentState = it->second;
		m_currentState->OnEnter();
		//m_targetState = nullptr;
		m_targetState = it->second;
		m_targetState->OnEnter();
		m_blendTime = blendTime;
		m_currentBlendTime = 0.0f;
		m_weightCurrent = 1.0f;
		m_weightTarget = 0.0f;
	}

	
	virtual void Update(float dt) {
		if (m_currentState == nullptr) {
			return;
		}

		
		m_currentState->OnUpdate(dt);

		
		//if (m_targetState != nullptr) {
		//	m_targetState->OnUpdate(dt);
		//	UpdateBlend(dt);
		//}
		//else {
		//	UpdateSingleAnim(m_currentState, dt);
		//}
		UpdateSingleAnim(m_currentState, dt);
		
		CalculateBoneMatrices();
	}

	
	AnimatedModel* GetAnimatedModel() const { return m_animatedModel; }
	AnimationInstance* GetAnimInstance() const { return m_animInstance; }
	AnimationState* GetCurrentState() const { return m_currentState; }
	float GetWeightCurrent() const { return m_weightCurrent; }
	float GetWeightTarget() const { return m_weightTarget; }

protected:
	void UpdateSingleAnim(AnimationState* state, float dt) {
		if (m_animInstance == nullptr || state == nullptr) {
			return;
		}
		float animTime = m_animInstance->t + dt * state->GetAnimSpeed();
		if (state->IsLoop()) {
			float animDuration = m_animatedModel->animation.animations[state->GetAnimName()].duration();
			animTime = fmod(animTime, animDuration);
		}
		m_animInstance->t = animTime;

		m_animInstance->usingAnimation = state->GetAnimName();
	}

	void UpdateBlend(float dt) {
		m_currentBlendTime += dt;
		float blendRatio = std::min(m_currentBlendTime / m_blendTime, 1.0f);

		if (m_blendType == BlendType::Linear) {
			m_weightCurrent = 1.0f - blendRatio;
			m_weightTarget = blendRatio;
		}

		if (blendRatio >= 1.0f) {
			m_currentState->OnExit();
			m_currentState = m_targetState;
			m_targetState = nullptr;
			m_currentBlendTime = 0.0f;
			m_weightCurrent = 1.0f;
			m_weightTarget = 0.0f;
		}
	}

	// calculate bone matrices
	virtual void CalculateBoneMatrices() {
		if (m_animInstance == nullptr || m_animatedModel == nullptr) {
			return;
		}
		Animation* anim = &m_animatedModel->animation;
		// single animation
		if (m_targetState == nullptr) {
			int frame = 0;
			float interpolationFact = 0.0f;
			anim->calcFrame(m_currentState->GetAnimName(), m_animInstance->t, frame, interpolationFact);
			
			for (int i = 0; i < anim->bonesSize(); i++) {
				m_animInstance->matrices[i] = anim->interpolateBoneToGlobal(m_currentState->GetAnimName(),
					m_animInstance->matrices, frame, interpolationFact, i);
			}
			anim->calcTransforms(m_animInstance->matrices, m_animInstance->coordTransform);
		}
		else {
			// Animation blending (has been discontinued) 
			Matrix matCurrent[256];
			int frameCurrent = 0;
			float interpCurrent = 0.0f;
			anim->calcFrame(m_currentState->GetAnimName(), m_animInstance->t, frameCurrent, interpCurrent);
			for (int i = 0; i < anim->bonesSize(); i++) {
				matCurrent[i] = anim->interpolateBoneToGlobal(m_currentState->GetAnimName(),
					matCurrent, frameCurrent, interpCurrent, i);
			}
			
			Matrix matTarget[256];
			int frameTarget = 0;
			float interpTarget = 0.0f;
			anim->calcFrame(m_targetState->GetAnimName(), m_animInstance->t, frameTarget, interpTarget);
			for (int i = 0; i < anim->bonesSize(); i++) {
				matTarget[i] = anim->interpolateBoneToGlobal(m_targetState->GetAnimName(),
					matTarget, frameTarget, interpTarget, i);
			}
			
			for (int i = 0; i < anim->bonesSize(); i++) {
				m_animInstance->matrices[i] = Matrix::lerp(matCurrent[i], matTarget[i], m_weightTarget);
			}
			anim->calcTransforms(m_animInstance->matrices, m_animInstance->coordTransform);
		}
	}
};