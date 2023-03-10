
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
		// Make sure we consume all messages coming from the window first
		m_Window->UpdateMessages();
		BeginFrame();
		Update(0.0f);
		EndFrame();
	}
	EndSimulation();
}

void Engine::BeginSimulation()
{
	m_IsRunning = true;
	m_TaskManager->BeginSimulation();
}

void Engine::BeginFrame()
{
	m_TaskManager->BeginFrame();
}

void Engine::Update(float deltaTime)
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
