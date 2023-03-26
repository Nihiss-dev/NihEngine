#include <array>

#include "NihPCH.h"
#include "Window/Renderer.h"
#include "Window/d3dx12.h"

#include <DirectXColors.h>

using Microsoft::WRL::ComPtr;

Renderer::Renderer()
	: m_FeatureLevel(D3D_FEATURE_LEVEL_11_0)
	, m_BackBufferIndex(0)
	, m_RtvDescriptorSize(0)
	, m_FenceValues{}
{

}

Renderer::~Renderer()
{
	WaitForGPU();
}

void Renderer::Initialize(HWND window, int width, int heigth)
{
	m_Window = window;
	m_OutputWidth = width;
	m_OutputHeigth = heigth;
	CreateDevice();
	CreateResources();
}

void Renderer::CreateDevice()
{
	DWORD dxgiFactoryFlags = 0;

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

	CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(m_DxgiFactory.ReleaseAndGetAddressOf()));

	ComPtr<IDXGIAdapter> adapter;
	GetAdapter(adapter.GetAddressOf());

	D3D12CreateDevice(adapter.Get(), m_FeatureLevel, IID_PPV_ARGS(m_D3dDevice.ReleaseAndGetAddressOf()));

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

	// Create the command queue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	m_D3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.ReleaseAndGetAddressOf()));

	// Create descriptor heaps for render target views and depth stencil views
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
	rtvDescriptorHeapDesc.NumDescriptors = m_SwapBufferCount;
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeadDesc = {};
	dsvDescriptorHeadDesc.NumDescriptors = 1;
	dsvDescriptorHeadDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	m_D3dDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(m_RtvDescriptorHeap.ReleaseAndGetAddressOf()));
	m_D3dDevice->CreateDescriptorHeap(&dsvDescriptorHeadDesc, IID_PPV_ARGS(m_DsvDescriptorHeap.ReleaseAndGetAddressOf()));

	m_RtvDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create a command allocator for each back buffer that will be rendered to
	for (UINT n = 0; n < m_SwapBufferCount; n++)
	{
		m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocators[n].ReleaseAndGetAddressOf()));
	}

	// Create a command list for recording graphics commands
	m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[0].Get(), nullptr, IID_PPV_ARGS(m_CommandList.ReleaseAndGetAddressOf()));
	m_CommandList->Close();

	// Create a fence for tracking GPU execution process
	m_D3dDevice->CreateFence(m_FenceValues[m_BackBufferIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.ReleaseAndGetAddressOf()));
	m_FenceValues[m_BackBufferIndex]++;

	m_FenceEvent.Attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
	if (!m_FenceEvent.IsValid())
	{
		// we should assert
	}

	// Check shader model 6 support
	D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_0 };
	if (FAILED(m_D3dDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))) || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
	{
#ifdef _DEBUG
		OutputDebugStringA("ERROR: Shader Model 6.0 is not supported\n");
#endif // _DEBUG
		// we should assert
	}
}

void Renderer::CreateResources()
{
	WaitForGPU();

	for (UINT n = 0; n < m_SwapBufferCount; n++)
	{
		m_RenderTargets[n].Reset();
		m_FenceValues[n] = m_FenceValues[m_BackBufferIndex];
	}

	constexpr DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	constexpr DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;
	const UINT backBufferWidth = static_cast<UINT>(m_OutputWidth);
	const UINT backBufferHeigth = static_cast<UINT>(m_OutputHeigth);

	if (m_SwapChain)
	{
		HRESULT hr = m_SwapChain->ResizeBuffers(m_SwapBufferCount, backBufferWidth, backBufferHeigth, backBufferFormat, 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			OnDeviceLost();
			return;
		}
	}
	else
	{
		// Create a descriptor for the swap chain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = backBufferWidth;
		swapChainDesc.Height = backBufferHeigth;
		swapChainDesc.Format = backBufferFormat;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = m_SwapBufferCount;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
		fsSwapChainDesc.Windowed = TRUE;

		// Create a swap chain for the window
		ComPtr<IDXGISwapChain1> swapChain;
		m_DxgiFactory->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_Window, &swapChainDesc, &fsSwapChainDesc, nullptr, swapChain.GetAddressOf());

		swapChain.As(&m_SwapChain);

		// Prevent from responding to ALT+ENTER shortcut
		m_DxgiFactory->MakeWindowAssociation(m_Window, DXGI_MWA_NO_ALT_ENTER);
	}

	// Obtain the back buffers for this window which will be the final render targets
	// and create render target views for each of them
	for (UINT n = 0; n < m_SwapBufferCount; n++)
	{
		m_SwapChain->GetBuffer(n, IID_PPV_ARGS(m_RenderTargets[n].GetAddressOf()));

		wchar_t name[25] = {};
		swprintf_s(name, L"Render target %u", n);
		m_RenderTargets[n]->SetName(name);

		auto cpuHandle = m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(cpuHandle, static_cast<INT>(n), m_RtvDescriptorSize);
		m_D3dDevice->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, rtvDescriptor);
	}

	// Reset the index to the current back buffer
	m_BackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	// Allocate a 2D surface as the depth/stencil buffer and create a depth/stencil view
	// on this surface
	const CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(depthBufferFormat, backBufferWidth, backBufferHeigth,
		1, /* This depth stencil view has only one texture */
		1 /* Use a single mipmap level */
	);
	depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	const CD3DX12_CLEAR_VALUE depthOptimizedClearValue(depthBufferFormat, 1.0f, 0u);

	m_D3dDevice->CreateCommittedResource(&depthHeapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthOptimizedClearValue, IID_PPV_ARGS(m_DepthStencil.ReleaseAndGetAddressOf()));

	m_DepthStencil->SetName(L"Depth stencil");

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = depthBufferFormat;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	auto cpuHandle = m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_D3dDevice->CreateDepthStencilView(m_DepthStencil.Get(), &dsvDesc, cpuHandle);
}

void Renderer::Render()
{
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

void Renderer::OnWindowSizeChanged(int width, int height)
{

}

void Renderer::GetDefaultSize(int& width, int& heigth)
{
	width = m_OutputWidth;
	heigth = m_OutputHeigth;
}

void Renderer::Clear()
{
	// Reset command list and allocator
	m_CommandAllocators[m_BackBufferIndex]->Reset();
	m_CommandList->Reset(m_CommandAllocators[m_BackBufferIndex].Get(), nullptr);

	// Transition the render target into the correct state to allow for drawing into it
	const D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_BackBufferIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_CommandList->ResourceBarrier(1, &barrier);

	// Clear the views
	auto cpuHandle = m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	auto cpuHandleDSV = m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(cpuHandle, static_cast<INT>(m_BackBufferIndex), m_RtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvDescriptor(cpuHandleDSV);
	m_CommandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
	m_CommandList->ClearRenderTargetView(rtvDescriptor, DirectX::Colors::CornflowerBlue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// Set the viewport and scissor rect
	const D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(m_OutputWidth), static_cast<float>(m_OutputHeigth), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
	const D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(m_OutputWidth), static_cast<LONG>(m_OutputHeigth) };
	m_CommandList->RSSetViewports(1, &viewport);
	m_CommandList->RSSetScissorRects(1, & scissorRect);
}

void Renderer::Present()
{
	// Transition the render target to the state that allows it to be presented to the display
	const D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_BackBufferIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_CommandList->ResourceBarrier(1, &barrier);

	// Send the command list off to the GPU for processing
	m_CommandList->Close();
	m_CommandQueue->ExecuteCommandLists(1, CommandListCast(m_CommandList.GetAddressOf()));

	// The first argument instruct DXGI to block until VSync, putting the application
	// to sleep until the next VSync. The ensure we don't waste any cycles rendering
	// frames that will never be displayed to the screen
	HRESULT hr = m_SwapChain->Present(1, 0);

	// If the device was reset we must completely reinitialize the renderer
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		OnDeviceLost();
	}
	else
	{
		MoveToNextFrame();
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

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), m_FeatureLevel, __uuidof(ID3D12Device), nullptr)))
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
		UINT64 fenceValue = m_FenceValues[m_BackBufferIndex];
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

void Renderer::OnDeviceLost()
{
	for (UINT n = 0; n < m_SwapBufferCount; n++)
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

	CreateDevice();
	CreateResources();
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