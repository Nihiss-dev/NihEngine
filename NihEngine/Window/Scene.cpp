#include "Scene.h"

Scene::Scene()
{
}

void Scene::Init()
{
	m_Objects.push_back(DirectX::GeometricPrimitive::CreateSphere());
}

void Scene::Render(ID3D12GraphicsCommandList* commandList)
{
	for (UniquePtr<DirectX::GeometricPrimitive>& primitive : m_Objects)
	{
		primitive->Draw(commandList);
	}
}