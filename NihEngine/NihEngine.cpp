// NihEngine.cpp : Définit le point d'entrée de l'application.
//

#include <memory>

#include "framework.h"
#include "NihEngine.h"

#include "Engine/Engine.h"
#include "Window/Window.h"
#include "Tasks/TaskManager.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    std::shared_ptr<TaskManager> taskManager = TaskManager::GetInstance();

    Window::WindowInit windowInit;
    windowInit.m_hInstance = hInstance;
    windowInit.m_Style = CS_HREDRAW | CS_VREDRAW;
    windowInit.m_NCmdShow = nCmdShow;
    windowInit.m_Heigth = 1920;
    windowInit.m_Width = 1080;

    Window window = Window(std::move(windowInit));
    taskManager->AddTask(&window);

    taskManager->Init();
    taskManager->Run();

    return 0;
}
