#pragma once

#include "Core/Memory/UniquePtr.h"
#include "Tasks/TaskManager.h"
#include "Window/Window.h"
#include "Engine/StepTimer.h"
#include "Core/NonCopyable.h"

class Engine : private NonCopyable
{
public:
    Engine() = default;
    ~Engine() = default;

    void SetWindowInit(Window::WindowInit&& windowInit);

    void Init();
    void Run();

private:
    void BeginSimulation();

    void Tick();
    void BeginFrame();
    void Update(const float deltaTime);
    void EndFrame();

    void EndSimulation();

private:
    UniquePtr<TaskManager> m_TaskManager{};
    UniquePtr<Window> m_Window{};

    DX::StepTimer m_Timer;
    bool m_IsRunning{false};
};
