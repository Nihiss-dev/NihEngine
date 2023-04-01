#pragma once

#include "Core/Memory/UniquePtr.h"
#include "Tasks/TaskManager.h"
#include "Window/Window.h"
#include "Engine/StepTimer.h"

class Engine
{
public:
    Engine();
    ~Engine();

    Engine(Engine&&) = default;
    Engine& operator= (Engine&&) = default;

    Engine(const Engine&) = delete;
    Engine& operator= (const Engine&) = delete;

    void SetWindowInit(Window::WindowInit&& windowInit);

    void Init();
    void Run();

private:
    void BeginSimulation();

    void Tick();
    void BeginFrame();
    void Update(float deltaTime);
    void EndFrame();

    void EndSimulation();

private:
    UniquePtr<TaskManager> m_TaskManager;
    UniquePtr<Window> m_Window;

    DX::StepTimer m_Timer;
    bool m_IsRunning{false};
};
