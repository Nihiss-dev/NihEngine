#include "Window.h"

#include "Tasks/TaskManager.h"

Window::Window()
{
	m_WindowInit = {};
}

Window::Window(WindowInit&& windowInit)
	: m_WindowInit(windowInit)
	, m_WindowName(windowInit.m_WindowName)
	, m_Height(windowInit.m_Heigth)
	, m_Width(windowInit.m_Width)
{

}

Window::~Window()
{

}

void Window::Init()
{
	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.style = m_WindowInit.m_Style;
	wcex.lpfnWndProc = (WNDPROC)Update;
	wcex.hInstance = m_WindowInit.m_hInstance;
	wcex.hIcon = LoadIconW(wcex.hInstance, L"IDI_ICON");
	wcex.hCursor = LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW);
	wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wcex.lpszClassName = L"Test";
	wcex.hIconSm = LoadIconW(wcex.hInstance, L"IDI_ICON");

	if (!RegisterClassExW(&wcex))
		return;

	m_Renderer = std::make_unique<Renderer>();

	std::wstring windowName = std::wstring(m_WindowName.begin(), m_WindowName.end());
	LPCWSTR windowNameStr = windowName.c_str();
	m_Hwnd = CreateWindowExW(0, L"Test", windowNameStr, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, m_Height, m_Width, HWND(), HMENU(), wcex.hInstance, m_Renderer.get());
	if (!m_Hwnd)
		return;

	ShowWindow(m_Hwnd, m_WindowInit.m_NCmdShow);

	RECT rc = { 0, 0, static_cast<LONG>(m_Height), static_cast<LONG>(m_Width) };
	GetClientRect(m_Hwnd, &rc);

	m_Renderer->Initialize(m_Hwnd, m_Height, m_Width);
}

void Window::UpdateMessages()
{
	MSG msg = {};

	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK Window::Update(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool sInSizeMove{ false };
	static bool sInSuspend{ false };
	static bool sMinimized{ false };
	static bool sFullscreen{ false };

	Renderer* renderer = reinterpret_cast<Renderer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_CREATE:
	{
		if (lParam)
		{
			auto params = reinterpret_cast<LPCREATESTRUCTW>(lParam);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(params->lpCreateParams));
		}
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		std::ignore = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_SIZE:
	{
		if (wParam == SIZE_MINIMIZED)
		{
			if (!sMinimized)
			{
				sMinimized = true;
				if (!sInSuspend)
				{
					renderer->OnSuspending();
				}
				sInSuspend = true;
			}
		}
		else if (sMinimized)
		{
			sMinimized = false;
			if (sInSuspend)
			{
				renderer->OnResuming();
			}
			sInSuspend = false;
		}
		else if (sInSizeMove)
		{
			renderer->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
		}
		break;
	}
	case WM_ENTERSIZEMOVE:
	{
		sInSizeMove = true;
		break;
	}
	case WM_EXITSIZEMOVE:
	{
		sInSizeMove = false;
		RECT rc;
		GetClientRect(hwnd, &rc);
		renderer->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
		break;
	}
	case WM_GETMINMAXINFO:
	{
		if (lParam)
		{
			MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(lParam);
			info->ptMinTrackSize.x = 320;
			info->ptMinTrackSize.y = 200;
		}
		break;
	}
	case WM_ACTIVATEAPP:
	{
		if (wParam)
		{
			renderer->OnActivated();
		}
		else
		{
			renderer->OnDeactivated();
		}
		break;
	}
	case WM_POWERBROADCAST:
	{
		switch (wParam)
		{
		case PBT_APMQUERYSUSPEND:
		{
			if (!sInSuspend)
			{
				renderer->OnSuspending();
			}
			sInSuspend = true;
			return TRUE;
		}
		case PBT_APMRESUMESUSPEND:
		{
			if (!sMinimized)
			{
				if (sInSuspend)
				{
					renderer->OnResuming();
				}
				sInSuspend = false;
			}
			return TRUE;
		}
		break;
		}
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		//TaskManager::GetInstance()->Stop();
		break;
	}
	case WM_SYSKEYDOWN:
	{
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		{
			// Implement the ALT+ENTER fullscreen toggle
			if (sFullscreen)
			{
				SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
				SetWindowLongPtr(hwnd, GWL_EXSTYLE, 0);

				int width = 800;
				int height = 600;
				renderer->GetDefaultSize(width, height);

				ShowWindow(hwnd, SW_SHOWNORMAL);
				SetWindowPos(hwnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			else
			{
				SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP);
				SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);
				SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
				ShowWindow(hwnd, SW_SHOWMAXIMIZED);
			}
			sFullscreen = !sFullscreen;
		}
		break;
	}
	default:
		return DefWindowProcW(hwnd, message, wParam, lParam);
	}
	return 0;
}

void Window::Render()
{
	m_Renderer->Render();
}