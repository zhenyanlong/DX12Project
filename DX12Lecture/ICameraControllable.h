#pragma once
#include "Vec3.h"
#include "Core.h"
//#include "Window.h"
//// 相机可控接口：所有需要被摄像机控制的Actor必须实现此接口
//class ICameraControllable
//{
//public:
//	virtual ~ICameraControllable() = default;
//
//	// ===== 位置与旋转属性 =====
//	virtual Vec3 GetPosition() { return Vec3(0.f, 0.f, 0.f); }
//	virtual float GetYaw() { return 0; }       // 偏航角（Y轴旋转，左右）
//	virtual float GetPitch() {}      // 俯仰角（X轴旋转，上下）
//	virtual const Vec3& GetCameraUp() const {} // 相机上方向（默认世界Y轴）
//	Vec3 Position;
//
//	// ===== 相机方向计算 =====
//	virtual Vec3 GetCameraForward() const // 相机前方向（由欧拉角计算）
//	{
//		Vec3 forward;
//		forward.x = cosf(GetYaw()) * cosf(GetPitch());
//		forward.y = sinf(GetPitch());
//		forward.z = sinf(GetYaw()) * cosf(GetPitch());
//		return forward.normalize();
//	}
//
//	virtual Vec3 GetCameraRight() const // 相机右方向（前方向×上方向）
//	{
//		return Cross(GetCameraForward(), GetCameraUp()).normalize();
//	}
//
//	// ===== 观察矩阵更新 =====
//	virtual Matrix GetViewMatrix() const
//	{
//		return Matrix::lookAt(GetPosition(), GetPosition() + GetCameraForward(), GetCameraUp());
//	}
//};
//
//// 相机控制器：封装鼠标/键盘输入处理，控制指定的ICameraControllable实例
//class CameraController
//{
//public:
//	// 控制器参数（保留原有逻辑，可外部配置）
//	static const float CAMERA_MOVE_SPEED;    // 移动速度（单位/秒）
//	static const float MOUSE_SENSITIVITY;    // 鼠标灵敏度（弧度/像素）
//	static const bool LOCK_MOUSE_ON_START;   // 启动时锁定鼠标
//
//private:
//	ICameraControllable* m_controlledActor = nullptr; // 当前被控制的Actor
//	bool m_mouseLocked = false;                       // 鼠标锁定状态
//	POINT m_windowCenter;                             // 窗口中心（用于鼠标重置）
//	HWND m_hwnd = nullptr;                            // 窗口句柄（用于鼠标操作）
//
//	// 私有方法：重置鼠标到窗口中心
//	void ResetMouseToCenter()
//	{
//		if (m_hwnd)
//		{
//			SetCursorPos(m_windowCenter.x, m_windowCenter.y);
//		}
//	}
//
//public:
//	CameraController(HWND hwnd) : m_hwnd(hwnd)
//	{
//		// 初始化窗口中心和鼠标状态
//		UpdateWindowCenter();
//		m_mouseLocked = LOCK_MOUSE_ON_START;
//		if (m_mouseLocked)
//		{
//			LockMouse();
//			ResetMouseToCenter();
//		}
//		else
//		{
//			UnlockMouse();
//		}
//	}
//
//	// 更新窗口中心（窗口移动时调用）
//	void UpdateWindowCenter()
//	{
//		if (m_hwnd)
//		{
//			RECT windowRect;
//			GetWindowRect(m_hwnd, &windowRect);
//			m_windowCenter.x = windowRect.left + (windowRect.right - windowRect.left) / 2;
//			m_windowCenter.y = windowRect.top + (windowRect.bottom - windowRect.top) / 2;
//		}
//	}
//
//	// 锁定/解锁鼠标
//	void LockMouse()
//	{
//		if (m_hwnd)
//		{
//			RECT windowRect;
//			GetWindowRect(m_hwnd, &windowRect);
//			ClipCursor(&windowRect);
//			ShowCursor(FALSE);
//			m_mouseLocked = true;
//		}
//	}
//
//	void UnlockMouse()
//	{
//		ClipCursor(NULL);
//		ShowCursor(TRUE);
//		m_mouseLocked = false;
//	}
//
//	// 设置当前被控制的Actor
//	void SetControlledActor(ICameraControllable* actor)
//	{
//		m_controlledActor = actor;
//		// 若Actor为空，解锁鼠标（可选）
//		if (!m_controlledActor && m_mouseLocked)
//		{
//			UnlockMouse();
//		}
//		else if (m_controlledActor && !m_mouseLocked && LOCK_MOUSE_ON_START)
//		{
//			LockMouse();
//			ResetMouseToCenter();
//		}
//	}
//
//	// 获取当前被控制的Actor
//	ICameraControllable* GetControlledActor() const { return m_controlledActor; }
//
//	// 处理输入并更新Actor的位置和旋转
//	void HandleInput(Window& win, float dt)
//	{
//		if (!m_controlledActor) return;
//
//		// ===== 1. ESC键切换鼠标锁定状态 =====
//		if (win.keys[VK_ESCAPE] && win.keyJustPressed[VK_ESCAPE])
//		{
//			m_mouseLocked = !m_mouseLocked;
//			if (m_mouseLocked)
//			{
//				LockMouse();
//				ResetMouseToCenter();
//			}
//			else
//			{
//				UnlockMouse();
//			}
//			win.keyJustPressed[VK_ESCAPE] = false; // 防止连续触发
//		}
//
//		// ===== 2. 鼠标控制旋转（仅锁定时生效） =====
//		if (m_mouseLocked)
//		{
//			POINT currentMousePos;
//			GetCursorPos(&currentMousePos);
//
//			// 计算鼠标偏移
//			int mouseDeltaX = currentMousePos.x - m_windowCenter.x;
//			int mouseDeltaY = currentMousePos.y - m_windowCenter.y;
//
//			// 重置鼠标到中心
//			ResetMouseToCenter();
//
//			// 更新欧拉角（转弧度，保留原有灵敏度）
//			m_controlledActor->GetYaw() -= mouseDeltaX * MOUSE_SENSITIVITY * 0.0174533f;
//			m_controlledActor->GetPitch() -= mouseDeltaY * MOUSE_SENSITIVITY * 0.0174533f;
//
//			// 限制俯仰角（避免视角翻转）
//			m_controlledActor->GetPitch() = clamp(m_controlledActor->GetPitch(),
//				-1.56f, 1.56f); // ≈-89° ~ +89°（PI/2 - 0.01）
//		}
//
//		// ===== 3. 键盘控制移动（保留原有WASD+QE逻辑） =====
//		Vec3 forward = m_controlledActor->GetCameraForward();
//		Vec3 right = m_controlledActor->GetCameraRight();
//		Vec3 up = m_controlledActor->GetCameraUp();
//		Vec3& pos = m_controlledActor->GetPosition();
//
//		// W/S：前后移动
//		if (win.keys['W']) pos += forward * CAMERA_MOVE_SPEED * dt;
//		if (win.keys['S']) pos -= forward * CAMERA_MOVE_SPEED * dt;
//
//		// A/D：左右平移
//		if (win.keys['A']) pos += right * CAMERA_MOVE_SPEED * dt;
//		if (win.keys['D']) pos -= right * CAMERA_MOVE_SPEED * dt;
//
//		// Q/E：上下移动（世界Y轴）
//		if (win.keys['Q']) pos.y -= CAMERA_MOVE_SPEED * dt;
//		if (win.keys['E']) pos.y += CAMERA_MOVE_SPEED * dt;
//	}
//
//	// 更新观察矩阵到全局GeneralMatrix
//	void UpdateViewMatrix(Matrix projectionMatrix);
//	
//};

class CameraControllable
{
public:
	virtual void updatePos(Vec3 pos) = 0;

	virtual void updateRotation(Vec3 rot) = 0;
	
	virtual void updateRotation(Matrix rotMat) = 0;

	virtual void updateWorldMatrix(Vec3 pos, float yaw, float pitch) = 0;
};