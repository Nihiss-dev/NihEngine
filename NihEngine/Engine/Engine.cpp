
#include "Engine.h"

void Engine::SetWindowInit(Window::WindowInit&& windowInit)
{
	m_Window = std::make_unique<Window>(std::move(windowInit));
}

void Engine::Init()
{
	m_Window->Init();
	m_TaskManager = std::make_unique<TaskManager>();
	m_TaskManager->Init();
}

void Engine::Run()
{
	BeginSimulation();
	while (m_IsRunning)
	{
		m_Timer.Tick([&]() {
			Tick();
		});
	}
	EndSimulation();
}

void Engine::BeginSimulation()
{
	m_IsRunning = true;
	m_TaskManager->BeginSimulation();
}

void Engine::Tick()
{
	float deltaTime = float(m_Timer.GetElapsedSeconds());
	// Make sure we consume all messages coming from the window first
	m_Window->UpdateMessages();
	BeginFrame();
	Update(deltaTime);
	EndFrame();

	// now that all logic has been updated, update rendering
	// don't try to render anything before the first update
	if (m_Timer.GetFrameCount() == 0)
	{
		return;
	}
	m_Window->Render();
}

void Engine::BeginFrame()
{
	m_TaskManager->BeginFrame();
}

void Engine::Update(const float deltaTime)
{
	m_TaskManager->Update(deltaTime);
}

void Engine::EndFrame()
{
	m_TaskManager->EndFrame();
}

void Engine::EndSimulation()
{
	m_TaskManager->EndSimulation();
}
