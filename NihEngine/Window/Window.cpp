#include "Window.h"

#include "Tasks/TaskManager.h"

Window::Window()
{
	m_WindowInit = {};
}

Window::Window(WindowInit&& windowInit)
	: m_WindowInit(windowInit)
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
	wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wcex.lpszClassName = L"Test";
	wcex.hIconSm = LoadIconW(wcex.hInstance, L"IDI_ICON");

	if (!RegisterClassExW(&wcex))
		return;

	HWND hwnd = CreateWindowExW(0, L"Test", L"TOTO", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, m_Height, m_Width, nullptr, nullptr, wcex.hInstance, nullptr);
	if (!hwnd)
		return;

	ShowWindow(hwnd, m_WindowInit.m_NCmdShow);

	RECT rc = { 0, 0, static_cast<LONG>(m_Height), static_cast<LONG>(m_Width) };
	GetClientRect(hwnd, &rc);
}

void Window::Run()
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
	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		TaskManager::GetInstance()->Stop();
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}