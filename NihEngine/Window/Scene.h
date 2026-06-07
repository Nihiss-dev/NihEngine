#pragma once

#include "NihPCH.h"

#include "Core/Containers/Array.h"
#include <d3d12.h>

#include "Core/Memory/UniquePtr.h"
#include "Engine/Camera/Camera.h"
#include "Window/RenderContext.h"

class Scene
{
public:
	Scene();

	void Init(int viewPortWidth, int viewPortHeight);
	void Update();

	RenderContext GetRenderContext() const;

	Camera& GetCamera() { return m_Camera; }
	const Camera& GetCamera() const { return m_Camera; }

	void OnWindowSizeChanged(int width, int height);

private:
	Camera m_Camera;
	Array<UniquePtr<DirectX::GeometricPrimitive>> m_Objects;
};
