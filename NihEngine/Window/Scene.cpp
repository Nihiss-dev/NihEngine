#include "Scene.h"

Scene::Scene()
{
}

void Scene::Init(int viewPortHeight, int viewPortWidth)
{
	m_Camera.Init(viewPortHeight, viewPortWidth);
	m_Objects.push_back(DirectX::GeometricPrimitive::CreateSphere());
}

void Scene::Update()
{
	m_Camera.Update();
}

RenderContext Scene::GetRenderContext() const
{
	RenderContext context;
	context.m_Camera = &m_Camera;
	for (const UniquePtr<DirectX::GeometricPrimitive>& primitive : m_Objects)
	{
		context.m_Primitives.push_back(primitive.get());
	}
	return context;
}

void Scene::OnWindowSizeChanged(int width, int height)
{
	m_Camera.OnWindowSizeChanged(width, height);
}
