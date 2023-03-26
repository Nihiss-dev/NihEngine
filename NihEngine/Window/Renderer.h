#pragma once

#include "NihPCH.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

class Renderer
{
public:
	Renderer();
	~Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	void Initialize(HWND window, int width, int heigth);
	void Render();

	// messages
	void OnActivated();
	void OnDeactivated();
	void OnSuspending();
	void OnResuming();
	void OnWindowSizeChanged(int width, int heigth);
	void GetDefaultSize(int &width, int &height);
private:
	void CreateDevice();
	void CreateResources();
	void Clear();
	void Present();

	void GetAdapter(IDXGIAdapter** ppAdapter);
	void WaitForGPU();

	void OnDeviceLost();
	void MoveToNextFrame();

private:
	HWND m_Window;
	int m_OutputWidth;
	int m_OutputHeigth;
	D3D_FEATURE_LEVEL m_FeatureLevel;
	static const UINT m_SwapBufferCount = 2;
	UINT m_BackBufferIndex;
	UINT m_RtvDescriptorSize;
	Microsoft::WRL::ComPtr<ID3D12Device> m_D3dDevice;
	Microsoft::WRL::ComPtr<IDXGIFactory4> m_DxgiFactory;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocators[m_SwapBufferCount];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_FenceValues[m_SwapBufferCount];
	Microsoft::WRL::Wrappers::Event m_FenceEvent;

	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_RenderTargets[m_SwapBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencil;

	std::unique_ptr<DirectX::GraphicsMemory> m_GraphicsMemory;
 };