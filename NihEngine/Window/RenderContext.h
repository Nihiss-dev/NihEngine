#pragma once

#include "NihPCH.h"
#include "Core/Containers/Array.h"

class Camera;

struct RenderContext
{
	const Camera* m_Camera = nullptr;
	Array<DirectX::GeometricPrimitive*> m_Primitives;
};