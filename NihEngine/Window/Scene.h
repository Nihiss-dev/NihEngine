#pragma once

#include "NihPCH.h"

#include "Core/Containers/Array.h"
#include <d3d12.h>

#include "Core/Memory/UniquePtr.h"

class Scene
{
public:
	Scene();

	void Init();
	void Render(ID3D12GraphicsCommandList* commandList);

private:
	Array<UniquePtr<DirectX::GeometricPrimitive>> m_Objects;
};
