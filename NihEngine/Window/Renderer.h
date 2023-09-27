#pragma once

#include "NihPCH.h"

#include <d3d12.h>
#include <Window/d3dx12.h>
#include <dxgi1_4.h>
#include <wrl.h>

#include "Core/Memory/UniquePtr.h"

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

class IDeviceNotify;

class Renderer
{
public:
	static constexpr unsigned int c_AllowTearing = 0x1;
	static constexpr unsigned int c_EnableHDR = 0x2;
	static constexpr unsigned int c_ReverseDepth = 0x4;

public:
	//Renderer();
	Renderer(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
			 DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT,
			 UINT backBufferCount = 2,
			 D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0,
			 unsigned int flags = 0) noexcept(false);
	~Renderer();

	Renderer(Renderer&&) = default;
	Renderer& operator=(Renderer&&) = default;

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	void CreateDeviceResources();
	void CreateWindowSizeDependentResources();

	void SetWindow(const HWND window, const int width, const int height);
	bool OnWindowSizeChanged(const int width, const int height);
	void HandleDeviceLost();
	void RegisterDeviceNotify(IDeviceNotify* deviceNotify) { m_DeviceNotify = deviceNotify; }
	void Prepare(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_PRESENT,
				 D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_RENDER_TARGET);
	void Present(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RENDER_TARGET);
	void WaitForGPU();
	void UpdateColorSpace();

	RECT GetOutputSize() const { return m_OutputSize; }

	ID3D12Device* GetD3DDevice() const { return m_D3dDevice.Get(); }
	IDXGISwapChain3* GetSwapChain() const { return m_SwapChain.Get(); }
	IDXGIFactory4* GetDXGIFactory() const { return m_DxgiFactory.Get(); }
	HWND GetWindow() const { return m_Window; }
	D3D_FEATURE_LEVEL GetDeviceFeatureLevel() const { return m_D3dFeatureLevel; }
	ID3D12Resource* GetRenderTarget() const { return m_RenderTargets[m_BackBufferIndex].Get(); }
	ID3D12Resource* GetDepthStencil() const { return m_DepthStencil.Get(); }
	ID3D12CommandQueue* GetCommandQueue() const { return m_CommandQueue.Get(); }
	ID3D12CommandAllocator* GetCommandAllocator() const { return m_CommandAllocators[m_BackBufferIndex].Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return m_CommandList.Get(); }
	DXGI_FORMAT GetBackBufferFormat() const { return m_BackBufferFormat; }
	DXGI_FORMAT GetDepthBufferFormat() const { return m_DepthBufferFormat; }
	D3D12_VIEWPORT GetScreenViewport() const { return m_ScreenViewport; }
	D3D12_RECT GetScissorRect() const { return m_ScissorRect; }
	UINT GetCurrentFrameIndex() const { return m_BackBufferIndex; }
	UINT GetBackBufferCount() const { return m_BackBufferCount; }
	DXGI_COLOR_SPACE_TYPE GetColorSpace() const { return m_ColorSpace; }
	unsigned int GetDeviceOptions() const { return m_Options; }

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const
	{
		const auto cpuHandle = m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuHandle, static_cast<INT>(m_BackBufferIndex), m_RtvDescriptorSize);
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const
	{
		const auto cpuHandle = m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuHandle);
	}

	void Render();

	// messages
	void OnActivated();
	void OnDeactivated();
	void OnSuspending();
	void OnResuming();
private:
	void Clear();

	void MoveToNextFrame();
	void GetAdapter(IDXGIAdapter** ppAdapter);

	static constexpr size_t MAX_BACK_BUFFER_COUNT = 3;

	UINT m_BackBufferIndex;

	//Direct3D objects
	Microsoft::WRL::ComPtr<ID3D12Device> m_D3dDevice;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocators[MAX_BACK_BUFFER_COUNT];

	//Swap chain objects
	Microsoft::WRL::ComPtr<IDXGIFactory4> m_DxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_RenderTargets[MAX_BACK_BUFFER_COUNT];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencil;

	//Presentation fence objects
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_FenceValues[MAX_BACK_BUFFER_COUNT];
	Microsoft::WRL::Wrappers::Event m_FenceEvent;

	//Direct3D rendering objects
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvDescriptorHeap;
	UINT m_RtvDescriptorSize;
	D3D12_VIEWPORT m_ScreenViewport;
	D3D12_RECT m_ScissorRect;

	//Direct3D properties
	DXGI_FORMAT m_BackBufferFormat;
	DXGI_FORMAT m_DepthBufferFormat;
	UINT m_BackBufferCount;
	D3D_FEATURE_LEVEL m_D3DMinFeatureLevel;

	//Cached device properties
	HWND m_Window;
	D3D_FEATURE_LEVEL m_D3dFeatureLevel;
	DWORD m_DxgiFactoryFlags;
	RECT m_OutputSize;

	//HDR support
	DXGI_COLOR_SPACE_TYPE m_ColorSpace;

	//Renderer options (see flags above)
	unsigned int m_Options;

	//The IDeviceNotify can be held directly as it owns the Renderer
	IDeviceNotify* m_DeviceNotify;

	UniquePtr<DirectX::GraphicsMemory> m_GraphicsMemory;

	using VertexType = DirectX::VertexPositionColor;
	UniquePtr<DirectX::BasicEffect> m_Effect;
	UniquePtr<DirectX::PrimitiveBatch<VertexType>> m_Batch;
};