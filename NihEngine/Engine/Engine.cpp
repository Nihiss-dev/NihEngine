
#include "Engine.h"

Engine::Engine()
{

}

Engine::~Engine()
{
}

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
		BeginFrame();
		Update(0.0f);
		EndFrame();
		m_Window->Run();
	}
	EndSimulation();
}

void Engine::BeginSimulation()
{
	m_IsRunning = true;
}

void Engine::BeginFrame()
{
}

void Engine::Update(float deltaTime)
{
	m_TaskManager->Update(deltaTime);
}

void Engine::EndFrame()
{
}

void Engine::EndSimulation()
{
}
