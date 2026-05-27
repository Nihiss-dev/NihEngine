// NihEngine.cpp : Définit le point d'entrée de l'application.
//

#include "framework.h"
#include "NihEngine.h"

//#include "Core/Memory/UniquePtr.h"
#include "Engine/Engine.h"
#include "Window/Window.h"

NihEngine::NihEngine()
{
}

void NihEngine::Init(HINSTANCE hInstance, int nCmdShow)
{
    m_Engine = std::make_unique<Engine>();

    Window::WindowInit windowInit;
    windowInit.m_hInstance = hInstance;
    windowInit.m_Style = CS_HREDRAW | CS_VREDRAW;
    windowInit.m_WindowName = "NihEngine";
    windowInit.m_NCmdShow = nCmdShow;
    windowInit.m_Heigth = 1920;
    windowInit.m_Width = 1080;

    m_Engine->SetWindowInit(std::move(windowInit));
    m_Engine->Init();
}

void NihEngine::Run()
{
    m_Engine->Run();
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    NihEngine NihEngine;

    NihEngine.Init(hInstance, nCmdShow);
    NihEngine.Run();

    return 0;
}
