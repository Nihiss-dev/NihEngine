#pragma once

#include "resource.h"

#include "Core/Memory/UniquePtr.h"
#include "Core/NonCopyable.h"

class Engine;

class NihEngine : public NonCopyable
{
public:
	NihEngine();

	void Init(HINSTANCE hInstance, int nCmdShow);
	void Run();

private:
	UniquePtr<Engine> m_Engine;
};