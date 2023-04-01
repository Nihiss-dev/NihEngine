#pragma once

#include "framework.h"
#include <string>
#include "NihEngine.h"

#include "Core/Memory/UniquePtr.h"
#include "Tasks/Task.h"
#include "Window/Renderer.h"

class Window
{
public:
	struct WindowInit
	{
		HINSTANCE m_hInstance;
		UINT m_Style;

		std::string m_WindowName;
		int m_NCmdShow;

		int m_Heigth;
		int m_Width;
	};

	Window();
	Window(WindowInit&&);
	~Window();

	Window(Window&&) = default;
	Window& operator= (Window&&) = default;

	Window(const Window&) = delete;
	Window& operator= (const Window&) = delete;

	void Init();
	void UpdateMessages();
	void Render();

	static LRESULT CALLBACK Update(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	UniquePtr<Renderer> m_Renderer;
	WindowInit m_WindowInit;
	HWND m_Hwnd;
	std::string m_WindowName;

	int m_Height = 480;
	int m_Width = 480;
};

