#pragma once

#include "framework.h"
#include "NihEngine.h"

#include "Tasks/Task.h"

class Window : public Task
{
public:
	struct WindowInit
	{
		HINSTANCE m_hInstance;
		UINT m_Style;
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

	void Init() override;
	void Run() override;

	static LRESULT CALLBACK Update(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	WindowInit m_WindowInit;
	HWND m_Hwnd;

	int m_Height = 480;
	int m_Width = 480;
};

