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

    std::unique_ptr<Engine> engine = std::make_unique<Engine>();

    Window::WindowInit windowInit;
    windowInit.m_hInstance = hInstance;
    windowInit.m_Style = CS_HREDRAW | CS_VREDRAW;
    windowInit.m_NCmdShow = nCmdShow;
    windowInit.m_Heigth = 1920;
    windowInit.m_Width = 1080;

    engine->SetWindowInit(std::move(windowInit));

    engine->Init();
    engine->Run();

    return 0;
}
