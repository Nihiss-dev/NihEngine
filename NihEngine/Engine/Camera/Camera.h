#pragma once

#include "NihPCH.h"
#include "Core/NonCopyable.h"

class Camera : private NonCopyable
{
public:
	Camera();
	~Camera() = default;

	void Init(int viewportWidth, int viewportHeight);

	void Update();

	// Window message handler
	void OnMouseButtonDown(int x, int y);
	void OnMouseButtonUp();
	void OnMouseMove(int x, int y);
	void OnMouseWheel(float delta);
	void OnWindowSizeChanged(int width, int height);

	const DirectX::SimpleMath::Matrix& GetViewMatrix() const { return m_ViewMatrix; }
	const DirectX::SimpleMath::Matrix& GetProjectionMatrix() const { return m_ProjectionMatrix; }

private:
	void RebuildView();
	void RebuildProjection();

private:
	DirectX::SimpleMath::Matrix m_ViewMatrix;
	DirectX::SimpleMath::Matrix m_ProjectionMatrix;

	DirectX::SimpleMath::Vector3 m_Target;
	float m_Radius;
	float m_Theta;
	float m_Phi;

	// Projection parameters
	float m_Fov;
	float m_AspectRatio;
	float m_NearPlane;
	float m_FarPlane;

	// Mouse state
	bool m_IsDragging;
	int m_LastMouseX;
	int m_LastMouseY;

	// Sensitivity settings
	static constexpr float cs_OrbitSensitivity = 0.005f;
	static constexpr float cs_ZoomSensitivity = 0.5f;
	static constexpr float cs_MinRadius = 1.0f;
	static constexpr float cs_MaxRadius = 500.0f;
	static constexpr float cs_MinPhi = 0.1f; // Prevent looking directly up/down
	static constexpr float cs_MaxPhi = DirectX::XM_PI - 0.1f; // Prevent looking directly up/down
};

