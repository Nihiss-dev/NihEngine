#pragma once

#include <memory>
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
    std::unique_ptr<TaskManager> m_TaskManager;
    std::unique_ptr<Window> m_Window;

    DX::StepTimer m_Timer;
    bool m_IsRunning{false};
};
