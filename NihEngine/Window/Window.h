#pragma once

#include "framework.h"
#include <string>
#include "NihEngine.h"

#include "Core/Memory/UniquePtr.h"
#include "Window/Renderer.h"
#include "Window/IDeviceNotify.h"

class Window : public IDeviceNotify
//class Window
{
public:
	struct WindowInit
	{
		HINSTANCE m_hInstance{};
		UINT m_Style{};

		std::string m_WindowName{};
		int m_NCmdShow{};

		int m_Heigth{};
		int m_Width{};
	};

	Window();
	Window(WindowInit&&);
	virtual ~Window();

	Window(Window&&) = default;
	Window& operator= (Window&&) = default;

	Window(const Window&) = delete;
	Window& operator= (const Window&) = delete;

	void Init();
	void UpdateMessages();
	void Render();

	static LRESULT CALLBACK Update(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	void OnDeviceLost() override;
	void OnDeviceRestored() override;

private:
	UniquePtr<Renderer> m_Renderer;
	WindowInit m_WindowInit;
	HWND m_Hwnd;
	std::string m_WindowName;

	int m_Height = 480;
	int m_Width = 480;
};

