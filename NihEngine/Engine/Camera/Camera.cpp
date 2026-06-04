#include "Camera.h"

Camera::Camera()
	: m_Target(DirectX::SimpleMath::Vector3::Zero)
	, m_Radius(10.0f)
	, m_Theta(DirectX::XM_PIDIV4) // 45 degrees
	, m_Phi(DirectX::XM_PIDIV4)   // 45 degrees
	, m_Fov(DirectX::XM_PIDIV4)   // 45 degrees
	, m_AspectRatio(16.0f / 9.0f)
	, m_NearPlane(0.1f)
	, m_FarPlane(1000.0f)
	, m_IsDragging(false)
	, m_LastMouseX(0)
	, m_LastMouseY(0)
{
}

void Camera::Init(int viewportWidth, int viewportHeight)
{
	m_AspectRatio = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
	RebuildView();
	RebuildProjection();
}

void Camera::Update()
{
}

void Camera::OnMouseButtonDown(int x, int y)
{
	m_IsDragging = true;
	m_LastMouseX = x;
	m_LastMouseY = y;
}

void Camera::OnMouseButtonUp()
{
	m_IsDragging = false;
}

void Camera::OnMouseMove(int x, int y)
{
	if (!m_IsDragging)
		return;

	int deltaX = x - m_LastMouseX;
	int deltaY = y - m_LastMouseY;
	m_LastMouseX = x;
	m_LastMouseY = y;

	// Horizontal drag rotate around Y axis
	m_Theta -= static_cast<float>(deltaX) * cs_OrbitSensitivity;

	// Vertical drag rotate towards/away from target
	m_Phi -= static_cast<float>(deltaY) * cs_OrbitSensitivity;

	// Clamp phi to prevent flipping
	m_Phi = std::clamp(m_Phi, cs_MinPhi, cs_MaxPhi);

	RebuildView();
}

void Camera::OnMouseWheel(float delta)
{
	m_Radius -= delta * cs_ZoomSensitivity;
	m_Radius = std::clamp(m_Radius, cs_MinRadius, cs_MaxRadius);
	RebuildView();
}

void Camera::OnWindowSizeChanged(int width, int height)
{
	m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
	RebuildProjection();
}

void Camera::RebuildView()
{
	// Convert spherical coordinates to Cartesian
	const float sinPhi = std::sin(m_Phi);
	const float cosPhi = std::cos(m_Phi);
	const float sinTheta = std::sin(m_Theta);
	const float cosTheta = std::cos(m_Theta);

	const DirectX::SimpleMath::Vector3 eye = m_Target + DirectX::SimpleMath::Vector3(
		m_Radius * sinPhi * cosTheta,
		m_Radius * cosPhi,
		m_Radius * sinPhi * sinTheta
	);

	m_ViewMatrix = DirectX::SimpleMath::Matrix::CreateLookAt(eye, m_Target, DirectX::SimpleMath::Vector3::Up);
}

void Camera::RebuildProjection()
{
	m_ProjectionMatrix = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
}