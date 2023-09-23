#include "Window/Renderer.h"
#include "Window/d3dx12.h"

#include "System/Assert.h"
#include "Window/IDeviceNotify.h"
#include <DirectXColors.h>

#include <dxgi1_6.h>

using Microsoft::WRL::ComPtr;

namespace
{
	inline DXGI_FORMAT NoSRGB(DXGI_FORMAT fmt)
	{
		switch (fmt)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM;
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8X8_UNORM;
		default: return fmt;
		}
	}

	inline long ComputeIntersectionArea(
		long ax1, long ay1, long ax2, long ay2,
		long bx1, long by1, long bx2, long by2) noexcept
	{
		return std::max<long>(0l, std::min<long>(ax2, bx2) - std::max<long>(ax1, bx1))
		* std::max<long>(0l, std::min<long>(ay2, by2) - std::max<long>(ay1, by1));
	}
}

Renderer::Renderer(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, UINT backBufferCount,
	D3D_FEATURE_LEVEL minFeatureLevel, unsigned flags) noexcept(false)
	: m_BackBufferIndex(0)
	, m_FenceValues{}
	, m_RtvDescriptorSize(0)
	, m_ScreenViewport{}
	, m_ScissorRect{}
	, m_BackBufferFormat(backBufferFormat)
	, m_DepthBufferFormat(depthBufferFormat)
	, m_BackBufferCount(backBufferCount)
	, m_D3DMinFeatureLevel(minFeatureLevel)
	, m_Window(nullptr)
	, m_D3dFeatureLevel(D3D_FEATURE_LEVEL_11_0)
	, m_DxgiFactoryFlags(0)
	, m_OutputSize{0, 0, 1, 1}
	, m_ColorSpace(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709)
	, m_Options(flags)
	, m_DeviceNotify(nullptr)
{
	NIH_ASSERT(!(backBufferCount < 2 || backBufferCount > MAX_BACK_BUFFER_COUNT));
	NIH_ASSERT(!(minFeatureLevel < D3D_FEATURE_LEVEL_11_0));
}

Renderer::~Renderer()
{
	WaitForGPU();
}

void Renderer::SetWindow(const HWND window, const int width, const int height)
{
	m_Window = window;

	m_OutputSize.left = m_OutputSize.top = 0;
	m_OutputSize.right = static_cast<LONG>(width);
	m_OutputSize.bottom = static_cast<LONG>(height);
}

void Renderer::CreateDeviceResources()
{
#if defined (_DEBUG)
	// Enable the debug layer
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif // _DEBUG

	CreateDXGIFactory2(m_DxgiFactoryFlags, IID_PPV_ARGS(m_DxgiFactory.ReleaseAndGetAddressOf()));

	if (m_Options & c_AllowTearing)
	{
		BOOL allowTearing = FALSE;

		ComPtr<IDXGIFactory5> factory5;
		HRESULT hr = m_DxgiFactory.As(&factory5);
		if (SUCCEEDED(hr))
		{
			hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
		}

		if (FAILED(hr) || !allowTearing)
		{
			m_Options &= ~c_AllowTearing;
		}
	}

	ComPtr<IDXGIAdapter> adapter;
	GetAdapter(adapter.GetAddressOf());

	D3D12CreateDevice(adapter.Get(), m_D3DMinFeatureLevel, IID_PPV_ARGS(m_D3dDevice.ReleaseAndGetAddressOf()));
	m_D3dDevice->SetName(L"DeviceResources");

#ifndef NDEBUG
	// Configure debug device (if active)
	ComPtr<ID3D12InfoQueue> d3dInfoQueue;
	if (SUCCEEDED(m_D3dDevice.As(&d3dInfoQueue)))
	{
#ifdef _DEBUG
		d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif // _DEBUG

		D3D12_MESSAGE_ID hide[] =
		{
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
			// workaround for debug layer issues on hybrid-graphics systems
			D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
		};
		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
		filter.DenyList.pIDList = hide;
		d3dInfoQueue->AddStorageFilterEntries(&filter);
	}
#endif // NDEBUG

	static const D3D_FEATURE_LEVEL s_FeatureLevels[] =
	{
#if defined(NTDDI_WIN10_FE) || defined(USING_D3D12_AGILITY_SDK)
		D3D_FEATURE_LEVEL_12_2,
#endif
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels =
	{
		static_cast<UINT>(std::size(s_FeatureLevels)), s_FeatureLevels, D3D_FEATURE_LEVEL_11_0
	};

	HRESULT hr;
	hr = m_D3dDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));
	if (SUCCEEDED(hr))
	{
		m_D3dFeatureLevel = featLevels.MaxSupportedFeatureLevel;
	}
	else
	{
		m_D3dFeatureLevel = m_D3DMinFeatureLevel;
	}

	// Create the command queue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	m_D3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.ReleaseAndGetAddressOf()));
	m_CommandQueue->SetName(L"DeviceResources");

	// Create descriptor heaps for render target views and depth stencil views
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
	rtvDescriptorHeapDesc.NumDescriptors = m_BackBufferCount;
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	m_D3dDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(m_RtvDescriptorHeap.ReleaseAndGetAddressOf()));
	m_RtvDescriptorHeap->SetName(L"DeviceResources");

	m_RtvDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	if (m_DepthBufferFormat != DXGI_FORMAT_UNKNOWN)
	{
		D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeadDesc = {};
		dsvDescriptorHeadDesc.NumDescriptors = 1;
		dsvDescriptorHeadDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

		m_D3dDevice->CreateDescriptorHeap(&dsvDescriptorHeadDesc, IID_PPV_ARGS(m_DsvDescriptorHeap.ReleaseAndGetAddressOf()));
		m_DsvDescriptorHeap->SetName(L"DeviceResources");
	}

	// Create a command allocator for each back buffer that will be rendered to
	for (UINT n = 0; n < m_BackBufferCount; n++)
	{
		m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocators[n].ReleaseAndGetAddressOf()));
		wchar_t name[25] = {};
		swprintf_s(name, L"Render Target &u", n);
		m_CommandAllocators[n]->SetName(name);
	}

	// Create a command list for recording graphics commands
	m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[0].Get(), nullptr, IID_PPV_ARGS(m_CommandList.ReleaseAndGetAddressOf()));
	m_CommandList->Close();
	m_CommandList->SetName(L"DeviceResources");

	// Create a fence for tracking GPU execution process
	m_D3dDevice->CreateFence(m_FenceValues[m_BackBufferIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.ReleaseAndGetAddressOf()));
	m_FenceValues[m_BackBufferIndex]++;
	m_Fence->SetName(L"DeviceResources");

	m_FenceEvent.Attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
	if (!m_FenceEvent.IsValid())
	{
		NIH_ASSERT(false);
	}

	// Check shader model 6 support
	D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_0 };
	if (FAILED(m_D3dDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))) || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
	{
#ifdef _DEBUG
		OutputDebugStringA("ERROR: Shader Model 6.0 is not supported\n");
#endif // _DEBUG
		NIH_ASSERT(false);
	}
}

void Renderer::CreateWindowSizeDependentResources()
{
	NIH_ASSERT(m_Window != nullptr);

	WaitForGPU();

	for (UINT n = 0; n < m_BackBufferCount; n++)
	{
		m_RenderTargets[n].Reset();
		m_FenceValues[n] = m_FenceValues[m_BackBufferIndex];
	}
	const UINT backBufferWidth = std::max<UINT>(static_cast<UINT>(m_OutputSize.right - m_OutputSize.left), 1u);
	const UINT backBufferHeight = std::max<UINT>(static_cast<UINT>(m_OutputSize.bottom - m_OutputSize.top), 1u);
	const DXGI_FORMAT backBufferFormat = NoSRGB(m_BackBufferFormat);

	if (m_SwapChain)
	{
		const HRESULT hr = m_SwapChain->ResizeBuffers(
			m_BackBufferCount,
			backBufferWidth,
			backBufferHeight,
			backBufferFormat, 
			(m_Options & c_AllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			HandleDeviceLost();
			return;
		}
	}
	else
	{
		// Create a descriptor for the swap chain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = backBufferWidth;
		swapChainDesc.Height = backBufferHeight;
		swapChainDesc.Format = backBufferFormat;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = m_BackBufferCount;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		swapChainDesc.Flags = (m_Options & c_AllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
		fsSwapChainDesc.Windowed = TRUE;

		// Create a swap chain for the window
		ComPtr<IDXGISwapChain1> swapChain;
		m_DxgiFactory->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_Window, &swapChainDesc, &fsSwapChainDesc, nullptr, swapChain.GetAddressOf());

		swapChain.As(&m_SwapChain);

		// Prevent from responding to ALT+ENTER shortcut
		m_DxgiFactory->MakeWindowAssociation(m_Window, DXGI_MWA_NO_ALT_ENTER);
	}

	UpdateColorSpace();

	// Obtain the back buffers for this window which will be the final render targets
	// and create render target views for each of them
	for (UINT n = 0; n < m_BackBufferCount; n++)
	{
		m_SwapChain->GetBuffer(n, IID_PPV_ARGS(m_RenderTargets[n].GetAddressOf()));

		wchar_t name[25] = {};
		swprintf_s(name, L"Render target %u", n);
		m_RenderTargets[n]->SetName(name);

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = m_BackBufferFormat;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		auto cpuHandle = m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(cpuHandle, static_cast<INT>(n), m_RtvDescriptorSize);
		m_D3dDevice->CreateRenderTargetView(m_RenderTargets[n].Get(), &rtvDesc, rtvDescriptor);
	}

	// Reset the index to the current back buffer
	m_BackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	if (m_DepthBufferFormat != DXGI_FORMAT_UNKNOWN)
	{
		// Allocate a 2D surface as the depth/stencil buffer and create a depth/stencil view
		// on this surface
		const CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

		D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_DepthBufferFormat, backBufferWidth, backBufferHeight,
			1, /* This depth stencil view has only one texture */
			1 /* Use a single mipmap level */
		);
		depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		const CD3DX12_CLEAR_VALUE depthOptimizedClearValue(m_DepthBufferFormat, (m_Options & c_AllowTearing) ? 0.0f : 1.0f, 0u);

		m_D3dDevice->CreateCommittedResource(&depthHeapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthOptimizedClearValue, IID_PPV_ARGS(m_DepthStencil.ReleaseAndGetAddressOf()));

		m_DepthStencil->SetName(L"Depth stencil");

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = m_DepthBufferFormat;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

		const auto& cpuHandle = m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_D3dDevice->CreateDepthStencilView(m_DepthStencil.Get(), &dsvDesc, cpuHandle);
	}

	m_ScreenViewport.TopLeftX = m_ScreenViewport.TopLeftY = 0.0f;
	m_ScreenViewport.Width = static_cast<float>(backBufferWidth);
	m_ScreenViewport.Height = static_cast<float>(backBufferHeight);
	m_ScreenViewport.MinDepth = D3D12_MIN_DEPTH;
	m_ScreenViewport.MaxDepth = D3D12_MAX_DEPTH;

	m_ScissorRect.left = m_ScissorRect.top = 0;
	m_ScissorRect.right = static_cast<LONG>(backBufferWidth);
	m_ScissorRect.bottom = static_cast<LONG>(backBufferHeight);
}

void Renderer::Render()
{
	Prepare();
	Clear();
	Present();
}

void Renderer::OnActivated()
{

}

void Renderer::OnDeactivated()
{

}

void Renderer::OnSuspending()
{

}

void Renderer::OnResuming()
{

}

bool Renderer::OnWindowSizeChanged(const int width, const int height)
{
	if (m_Window)
		return false;

	RECT newRect;
	newRect.left = newRect.top = 0;
	newRect.right = static_cast<long>(width);
	newRect.bottom = static_cast<long>(height);

	if (newRect.right == m_OutputSize.right && newRect.bottom == m_OutputSize.bottom)
	{
		UpdateColorSpace();
		return false;
	}

	m_OutputSize = newRect;
	CreateWindowSizeDependentResources();
	return true;
}

void Renderer::Clear()
{
	auto const rtvDescriptor = GetRenderTargetView();
	auto const dsvDescriptor = GetDepthStencilView();
	m_CommandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
	m_CommandList->ClearRenderTargetView(rtvDescriptor, DirectX::Colors::CornflowerBlue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	auto const viewport = GetScreenViewport();
	auto const scissorRect = GetScissorRect();

	m_CommandList->RSSetViewports(1, &viewport);
	m_CommandList->RSSetScissorRects(1, &scissorRect);
}


void Renderer::Prepare(D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
	// Reset command list and allocator
	m_CommandAllocators[m_BackBufferIndex]->Reset();
	m_CommandList->Reset(m_CommandAllocators[m_BackBufferIndex].Get(), nullptr);

	if (beforeState != afterState)
	{
		// Transition the render target into the correct state to allow for drawing into it
		const D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_BackBufferIndex].Get(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->ResourceBarrier(1, &barrier);
	}
}


void Renderer::Present(D3D12_RESOURCE_STATES beforeState)
{
	if (beforeState != D3D12_RESOURCE_STATE_PRESENT)
	{
		// Transition the render target to the state that allows it to be presented to the display
		const D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_BackBufferIndex].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_CommandList->ResourceBarrier(1, &barrier);
	}

	// Send the command list off to the GPU for processing
	m_CommandList->Close();
	m_CommandQueue->ExecuteCommandLists(1, CommandListCast(m_CommandList.GetAddressOf()));

	HRESULT hr;
	if (m_Options & c_AllowTearing)
	{
		// Recommended to always use tearing if supported when using a sync interval of 0
		// Note this will fail in true 'fullscreen' mode
		hr = m_SwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	}
	else
	{
		// The first argument instruct DXGI to block until VSync, putting the application
		// to sleep until the next VSync. The ensure we don't waste any cycles rendering
		// frames that will never be displayed to the screen
		hr = m_SwapChain->Present(1, 0);
	}

	// If the device was reset we must completely reinitialize the renderer
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		HandleDeviceLost();
	}
	else
	{
		MoveToNextFrame();

		if (!m_DxgiFactory->IsCurrent())
		{
			UpdateColorSpace();
		}
	}
}

void Renderer::GetAdapter(IDXGIAdapter** ppAdapter)
{
	*ppAdapter = nullptr;

	ComPtr<IDXGIAdapter1> adapter;
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_DxgiFactory->EnumAdapters1(adapterIndex, adapter.ReleaseAndGetAddressOf()); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), m_D3DMinFeatureLevel, __uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

#if !defined(NDEBUG)
	if (!adapter)
	{
		if (FAILED(m_DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()))))
		{
			// we should assert
		}
	}
#endif // NDEBUG

	if (!adapter)
	{
		// we should assert
	}

	*ppAdapter = adapter.Detach();
}

void Renderer::WaitForGPU()
{
	if (m_CommandQueue && m_Fence && m_FenceEvent.IsValid())
	{
		// Schedule a signal command in the GPU queue
		const UINT64 fenceValue = m_FenceValues[m_BackBufferIndex];
		if (SUCCEEDED(m_CommandQueue->Signal(m_Fence.Get(), fenceValue)))
		{
			// Wait until the signal has been processed
			if (SUCCEEDED(m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent.Get())))
			{
				std::ignore = WaitForSingleObjectEx(m_FenceEvent.Get(), INFINITE, FALSE);

				// Increment the fence value for the current frame
				m_FenceValues[m_BackBufferIndex]++;
			}
		}
	}
}

void Renderer::HandleDeviceLost()
{
	for (UINT n = 0; n < m_BackBufferCount; n++)
	{
		m_CommandAllocators[n].Reset();
		m_RenderTargets[n].Reset();
	}

	m_DepthStencil.Reset();
	m_Fence.Reset();
	m_CommandList.Reset();
	m_SwapChain.Reset();
	m_RtvDescriptorHeap.Reset();
	m_DsvDescriptorHeap.Reset();
	m_CommandQueue.Reset();
	m_D3dDevice.Reset();
	m_DxgiFactory.Reset();

	CreateDeviceResources();
	CreateWindowSizeDependentResources();

	if (m_DeviceNotify)
	{
		m_DeviceNotify->OnDeviceRestored();
	}
}

void Renderer::MoveToNextFrame()
{
	// Schedule a signal command in the queue
	const UINT64 currentFenceValue = m_FenceValues[m_BackBufferIndex];
	m_CommandQueue->Signal(m_Fence.Get(), currentFenceValue);

	// Update the backbuffer index
	m_BackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready
	if (m_Fence->GetCompletedValue() < m_FenceValues[m_BackBufferIndex])
	{
		m_Fence->SetEventOnCompletion(m_FenceValues[m_BackBufferIndex], m_FenceEvent.Get());
		std::ignore = WaitForSingleObjectEx(m_FenceEvent.Get(), INFINITE, FALSE);
	}

	// Set the fence value for the next frame
	m_FenceValues[m_BackBufferIndex] = currentFenceValue + 1;
}

void Renderer::UpdateColorSpace()
{
	if (!m_DxgiFactory)
	{
		return;
	}

	if (!m_DxgiFactory->IsCurrent())
	{
		CreateDXGIFactory2(m_DxgiFactoryFlags, IID_PPV_ARGS(m_DxgiFactory.ReleaseAndGetAddressOf()));
	}

	DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
	bool isDisplayHDR10 = false;

	if (m_SwapChain)
	{
		RECT windowBounds;
		NIH_ASSERT(GetWindowRect(m_Window, &windowBounds));

		const long ax1 = windowBounds.left;
		const long ay1 = windowBounds.top;
		const long ax2 = windowBounds.right;
		const long ay2 = windowBounds.bottom;

		ComPtr<IDXGIOutput> bestOutput;
		long bestIntersectArea = -1;

		ComPtr<IDXGIAdapter> adapter;
		for (UINT adapterIndex = 0;
			SUCCEEDED(m_DxgiFactory->EnumAdapters(adapterIndex, adapter.ReleaseAndGetAddressOf()));
			++adapterIndex)
		{
			ComPtr<IDXGIOutput> output;
			for (UINT outputIndex = 0;
				SUCCEEDED(adapter->EnumOutputs(outputIndex, output.ReleaseAndGetAddressOf()));
				++outputIndex)
			{
				DXGI_OUTPUT_DESC desc;
				output->GetDesc(&desc);

				const auto& r = desc.DesktopCoordinates;

				const long intersectArea = ComputeIntersectionArea(ax1, ay1, ax2, ay2, r.left, r.top, r.right, r.bottom);
				if (intersectArea > bestIntersectArea)
				{
					bestOutput.Swap(output);
					bestIntersectArea = intersectArea;
				}
			}
		}

		if (bestOutput)
		{
			ComPtr<IDXGIOutput6> output6;
			if (SUCCEEDED(bestOutput.As(&output6)))
			{
				DXGI_OUTPUT_DESC1 desc;
				output6->GetDesc1(&desc);

				if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
				{
					isDisplayHDR10 = true;
				}
			}
		}
	}

	if ((m_Options & c_EnableHDR) && isDisplayHDR10)
	{
		switch (m_BackBufferFormat)
		{
		case DXGI_FORMAT_R10G10B10A2_UNORM:
			colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
			break;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
			break;
		default:
			break;
		}
	}

	m_ColorSpace = colorSpace;

	UINT colorSpaceSupport = 0;
if (m_SwapChain && SUCCEEDED(m_SwapChain->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport))
	&& (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
{
	m_SwapChain->SetColorSpace1(colorSpace);
}
}
