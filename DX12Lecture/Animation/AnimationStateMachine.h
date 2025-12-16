#pragma once
#include "Mesh.h"
#include <string>
#include <unordered_map>
#include <functional>
#include "Animation/AnimationState.h"
// 前置声明


// ===================== 动画状态机基类 =====================
class AnimationStateMachine {
protected:
	AnimatedModel* m_animatedModel;        // 关联的动画模型
	AnimationInstance* m_animInstance;     // 关联的动画实例
	std::unordered_map<std::string, AnimationState*> m_states; // 状态列表（状态名→状态对象）
	AnimationState* m_currentState;        // 当前状态
	AnimationState* m_targetState;         // 目标状态（用于混合过渡）
	float m_blendTime;                     // 混合过渡总时间（如0.2f）
	float m_currentBlendTime;              // 当前混合进度（0~blendTime）
	float m_weightCurrent;                 // 当前动画权重（1→0）
	float m_weightTarget;                  // 目标动画权重（0→1）

	// 动画混合类型（此处实现线性混合，可扩展）
	enum class BlendType {
		Linear, // 线性混合
		EaseInOut // 缓入缓出（可选）
	};
	BlendType m_blendType;

public:
	AnimationStateMachine(AnimatedModel* model, AnimationInstance* instance)
		: m_animatedModel(model), m_animInstance(instance), m_currentState(nullptr), m_targetState(nullptr),
		m_blendTime(0.2f), m_currentBlendTime(0.0f), m_weightCurrent(1.0f), m_weightTarget(0.0f),
		m_blendType(BlendType::Linear) {}
	virtual ~AnimationStateMachine() {
		// 释放所有状态
		for (auto& pair : m_states) {
			delete pair.second;
		}
		m_states.clear();
	}

	// 初始化：注册状态（子类调用）
	void RegisterState(AnimationState* state) {
		m_states[state->GetStateName()] = state;
	}

	// 切换状态（支持混合过渡）
	void ChangeState(const std::string& targetStateName, float blendTime = 0.2f) {
		auto it = m_states.find(targetStateName);
		if (it == m_states.end()) {
			return; // 状态不存在
		}
		// 如果当前无状态，直接设置为目标状态
		if (m_currentState == nullptr) {
			m_currentState = it->second;
			m_currentState->OnEnter();
			m_targetState = nullptr;
			m_currentBlendTime = 0.0f;
			m_weightCurrent = 1.0f;
			m_weightTarget = 0.0f;
			return;
		}
		// 如果目标状态是当前状态，忽略
		if (m_currentState->GetStateName() == targetStateName) {
			return;
		}
		// 设置目标状态，开始混合
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

	// 每帧更新（核心：状态更新+动画混合+骨骼矩阵计算）
	virtual void Update(float dt) {
		if (m_currentState == nullptr) {
			return;
		}

		// 1. 更新当前状态
		m_currentState->OnUpdate(dt);

		// 2. 处理动画混合
		//if (m_targetState != nullptr) {
		//	m_targetState->OnUpdate(dt);
		//	UpdateBlend(dt);
		//}
		//else {
		//	// 无混合：直接更新当前动画
		//	UpdateSingleAnim(m_currentState, dt);
		//}
		UpdateSingleAnim(m_currentState, dt);
		// 3. 计算骨骼矩阵（传递给AnimationInstance）
		CalculateBoneMatrices();
	}

	// 获取属性
	AnimatedModel* GetAnimatedModel() const { return m_animatedModel; }
	AnimationInstance* GetAnimInstance() const { return m_animInstance; }
	AnimationState* GetCurrentState() const { return m_currentState; }
	float GetWeightCurrent() const { return m_weightCurrent; }
	float GetWeightTarget() const { return m_weightTarget; }

protected:
	// 更新单个动画的播放进度
	void UpdateSingleAnim(AnimationState* state, float dt) {
		if (m_animInstance == nullptr || state == nullptr) {
			return;
		}
		// 推进动画时间（考虑播放速度）
		float animTime = m_animInstance->t + dt * state->GetAnimSpeed();
		// 处理循环
		if (state->IsLoop()) {
			float animDuration = m_animatedModel->animation.animations[state->GetAnimName()].duration();
			animTime = fmod(animTime, animDuration);
		}
		m_animInstance->t = animTime;
		// 标记当前动画名
		m_animInstance->usingAnimation = state->GetAnimName();
	}

	// 更新动画混合进度
	void UpdateBlend(float dt) {
		m_currentBlendTime += dt;
		float blendRatio = std::min(m_currentBlendTime / m_blendTime, 1.0f);
		// 根据混合类型计算权重
		if (m_blendType == BlendType::Linear) {
			m_weightCurrent = 1.0f - blendRatio;
			m_weightTarget = blendRatio;
		}
		// 混合完成：切换到目标状态
		if (blendRatio >= 1.0f) {
			m_currentState->OnExit();
			m_currentState = m_targetState;
			m_targetState = nullptr;
			m_currentBlendTime = 0.0f;
			m_weightCurrent = 1.0f;
			m_weightTarget = 0.0f;
		}
	}

	// 计算骨骼矩阵（核心：处理混合或单个动画）
	virtual void CalculateBoneMatrices() {
		if (m_animInstance == nullptr || m_animatedModel == nullptr) {
			return;
		}
		Animation* anim = &m_animatedModel->animation;
		// 1. 单个动画：直接使用AnimationInstance的更新逻辑
		if (m_targetState == nullptr) {
			int frame = 0;
			float interpolationFact = 0.0f;
			anim->calcFrame(m_currentState->GetAnimName(), m_animInstance->t, frame, interpolationFact);
			// 计算每个骨骼的矩阵
			for (int i = 0; i < anim->bonesSize(); i++) {
				m_animInstance->matrices[i] = anim->interpolateBoneToGlobal(m_currentState->GetAnimName(),
					m_animInstance->matrices, frame, interpolationFact, i);
			}
			anim->calcTransforms(m_animInstance->matrices, m_animInstance->coordTransform);
		}
		else {
			// 2. 动画混合：线性混合两个动画的骨骼矩阵（核心逻辑）
			// 2.1 计算当前动画的骨骼矩阵
			Matrix matCurrent[256];
			int frameCurrent = 0;
			float interpCurrent = 0.0f;
			anim->calcFrame(m_currentState->GetAnimName(), m_animInstance->t, frameCurrent, interpCurrent);
			for (int i = 0; i < anim->bonesSize(); i++) {
				matCurrent[i] = anim->interpolateBoneToGlobal(m_currentState->GetAnimName(),
					matCurrent, frameCurrent, interpCurrent, i);
			}
			// 2.2 计算目标动画的骨骼矩阵
			Matrix matTarget[256];
			int frameTarget = 0;
			float interpTarget = 0.0f;
			anim->calcFrame(m_targetState->GetAnimName(), m_animInstance->t, frameTarget, interpTarget);
			for (int i = 0; i < anim->bonesSize(); i++) {
				matTarget[i] = anim->interpolateBoneToGlobal(m_targetState->GetAnimName(),
					matTarget, frameTarget, interpTarget, i);
			}
			// 2.3 线性混合两个矩阵（权重加权）
			for (int i = 0; i < anim->bonesSize(); i++) {
				m_animInstance->matrices[i] = Matrix::lerp(matCurrent[i], matTarget[i], m_weightTarget);
			}
			anim->calcTransforms(m_animInstance->matrices, m_animInstance->coordTransform);
		}
	}
};