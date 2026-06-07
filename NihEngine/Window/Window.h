#pragma once

#include "framework.h"
#include <string>

#include "Core/Memory/UniquePtr.h"
#include "Window/Renderer.h"
#include "Window/RenderContext.h"
#include "Window/IDeviceNotify.h"

class Scene;

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
	void Render(const RenderContext& context);

	void SetScene(Scene* scene);
	RECT GetOutputSize() const;

	static LRESULT CALLBACK Update(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	void OnDeviceLost() override;
	void OnDeviceRestored() override;

private:
	UniquePtr<Renderer> m_Renderer;
	Scene* m_Scene{ nullptr };
	WindowInit m_WindowInit;
	HWND m_Hwnd;
	std::string m_WindowName;

	int m_Height = 480;
	int m_Width = 480;
};

