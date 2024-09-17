#include "ui.h"

#define GlobalFunctionsBegin namespace GlobalFunctions {
#define GlobalFunctionsEnd };

#define UseGlobalFunctions using namespace GlobalFunctions;

GlobalFunctionsBegin

void CreateRenderTarget() {
    for (UINT i = 0; i < 3; i++)
    {
        ID3D12Resource* pBackBuffer = nullptr;
        GlobalData::SwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        GlobalData::D12Device->CreateRenderTargetView(pBackBuffer, nullptr, GlobalData::RenderTargetDescriptor[i]);
        GlobalData::RenderTargetResource[i] = pBackBuffer;
    }
}

bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC1 sd;
    {
        ZeroMemory(&sd, sizeof(DXGI_SWAP_CHAIN_DESC1));
        sd.BufferCount = 3;
        sd.Width = 0;
        sd.Height = 0;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        sd.Scaling = DXGI_SCALING_STRETCH;
        sd.Stereo = FALSE;
    }

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&GlobalData::D12Device)) != S_OK)
        return false;

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = 3;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        if (GlobalData::D12Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&GlobalData::RtvDescHeap)) != S_OK)
            return false;

        SIZE_T rtvDescriptorSize = GlobalData::D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GlobalData::RtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT i = 0; i < 3; i++)
        {
            GlobalData::RenderTargetDescriptor[i] = rtvHandle;
            rtvHandle.ptr += rtvDescriptorSize;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (GlobalData::D12Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&GlobalData::SrvDescHeap)) != S_OK)
            return false;
    }

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 1;
        if (GlobalData::D12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&GlobalData::CommandQueue)) != S_OK)
            return false;
    }

    for (UINT i = 0; i < 3; i++)
        if (GlobalData::D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&GlobalData::FrameContext[i].CommandAllocator)) != S_OK)
            return false;

    if (GlobalData::D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, GlobalData::FrameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&GlobalData::CommandList)) != S_OK ||
        GlobalData::CommandList->Close() != S_OK)
        return false;

    if (GlobalData::D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&GlobalData::Fence)) != S_OK)
        return false;

    GlobalData::FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (GlobalData::FenceEvent == nullptr)
        return false;

    {
        IDXGIFactory4* dxgiFactory = nullptr;
        IDXGISwapChain1* swapChain1 = nullptr;
        if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
            return false;
        if (dxgiFactory->CreateSwapChainForHwnd(GlobalData::CommandQueue, hWnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
            return false;
        if (swapChain1->QueryInterface(IID_PPV_ARGS(&GlobalData::SwapChain)) != S_OK)
            return false;
        swapChain1->Release();
        dxgiFactory->Release();
        GlobalData::SwapChain->SetMaximumFrameLatency(3);
        GlobalData::SwapChainWaitableObject = GlobalData::SwapChain->GetFrameLatencyWaitableObject();
    }

    CreateRenderTarget();
    return true;
}

void WaitForLastSubmittedFrame() {
    GlobalStruct::FrameContext* frameCtx = &GlobalData::FrameContext[GlobalData::FrameIndex % 3];

    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue == 0)
        return;

    frameCtx->FenceValue = 0;
    if (GlobalData::Fence->GetCompletedValue() >= fenceValue)
        return;

    GlobalData::Fence->SetEventOnCompletion(fenceValue, GlobalData::FenceEvent);
    WaitForSingleObject(GlobalData::FenceEvent, INFINITE);
}

void CleanupRenderTarget() {
    WaitForLastSubmittedFrame();

    for (UINT i = 0; i < 3; i++)
        if (GlobalData::RenderTargetResource[i]) { GlobalData::RenderTargetResource[i]->Release(); GlobalData::RenderTargetResource[i] = nullptr; }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (GlobalData::D12Device != nullptr && wParam != SIZE_MINIMIZED)
        {
            WaitForLastSubmittedFrame();
            CleanupRenderTarget();
            HRESULT result = GlobalData::SwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
            assert(SUCCEEDED(result) && "Failed to resize swapchain.");
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

WNDCLASSEXW* ConstructWindowClass(WNDCLASSEXW&& Entry) {
    return new WNDCLASSEXW((WNDCLASSEXW&&)Entry);
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (GlobalData::SwapChain) { GlobalData::SwapChain->SetFullscreenState(false, nullptr); GlobalData::SwapChain->Release(); GlobalData::SwapChain = nullptr; }
    if (GlobalData::SwapChainWaitableObject != nullptr) { CloseHandle(GlobalData::SwapChainWaitableObject); }
    for (UINT i = 0; i < 3; i++)
        if (GlobalData::FrameContext[i].CommandAllocator) { GlobalData::FrameContext[i].CommandAllocator->Release(); GlobalData::FrameContext[i].CommandAllocator = nullptr; }
    if (GlobalData::CommandQueue) { GlobalData::CommandQueue->Release(); GlobalData::CommandQueue = nullptr; }
    if (GlobalData::CommandList) { GlobalData::CommandList->Release(); GlobalData::CommandList = nullptr; }
    if (GlobalData::RtvDescHeap) { GlobalData::RtvDescHeap->Release(); GlobalData::RtvDescHeap = nullptr; }
    if (GlobalData::SrvDescHeap) { GlobalData::SrvDescHeap->Release(); GlobalData::SrvDescHeap = nullptr; }
    if (GlobalData::Fence) { GlobalData::Fence->Release(); GlobalData::Fence = nullptr; }
    if (GlobalData::FenceEvent) { CloseHandle(GlobalData::FenceEvent); GlobalData::FenceEvent = nullptr; }
    if (GlobalData::D12Device) { GlobalData::D12Device->Release(); GlobalData::D12Device = nullptr; }
}

GlobalStruct::FrameContext* WaitForNextFrameResources()
{
    UINT nextFrameIndex = GlobalData::FrameIndex + 1;
    GlobalData::FrameIndex = nextFrameIndex;

    HANDLE waitableObjects[] = { GlobalData::SwapChainWaitableObject, nullptr };
    DWORD numWaitableObjects = 1;

    GlobalStruct::FrameContext* frameCtx = &GlobalData::FrameContext[nextFrameIndex % 3];
    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue != 0)
    {
        frameCtx->FenceValue = 0;
        GlobalData::Fence->SetEventOnCompletion(fenceValue, GlobalData::FenceEvent);
        waitableObjects[1] = GlobalData::FenceEvent;
        numWaitableObjects = 2;
    }

    WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

    return frameCtx;
}

void ReleaseWindowsClass() {
    delete GlobalData::WindowClass;
}

GlobalFunctionsEnd

UseGlobalFunctions

void InitUI::CreateFatherWindow() {
    GlobalData::ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    GlobalData::ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    GlobalData::WindowClass = ConstructWindowClass(
        {
        sizeof(WNDCLASSEXW),
        CS_CLASSDC, WndProc,
        0L, 0L,
        GetModuleHandle(nullptr),
        nullptr, nullptr, nullptr,
        nullptr, L"ImGui Class",
        nullptr
        }
    );

    RegisterClassExW(GlobalData::WindowClass);

    GlobalData::WindowHwnd = ::CreateWindowW(
        GlobalData::WindowClass->lpszClassName,
        L"Code Viewer", WS_OVERLAPPEDWINDOW,
        100, 100,
        800, 600,
        nullptr, nullptr,
        GlobalData::WindowClass->hInstance, nullptr
    );
}

void InitUI::CreateDevice12() {
    if (CreateDeviceD3D(GlobalData::WindowHwnd))
        return;

    CleanupDeviceD3D();
    UnregisterClassW(GlobalData::WindowClass->lpszClassName, GlobalData::WindowClass->hInstance);
    MessageBox(
        NULL, L"Can't create device12.",
        L"Try to faile.", MB_OK
    );
    exit(-1);
}

void InitUI::ShowWindow() {
    ShowWindow(GlobalData::WindowHwnd, SW_SHOWDEFAULT);
    UpdateWindow(GlobalData::WindowHwnd);
}

void InitUI::ImGui() {
    printf("ImGui\n");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    static ImGuiIO& ImGuiMainIO = ImGui::GetIO(); (void)ImGuiMainIO;
    ImGuiMainIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGuiMainIO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(GlobalData::WindowHwnd);
    ImGui_ImplDX12_Init(
        GlobalData::D12Device, 3,
        DXGI_FORMAT_R8G8B8A8_UNORM, 
        GlobalData::SrvDescHeap,
        GlobalData::SrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        GlobalData::SrvDescHeap->GetGPUDescriptorHandleForHeapStart()
    );
}

void ReleaseUI::Release() {
    WaitForLastSubmittedFrame();

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(GlobalData::WindowHwnd);
    ::UnregisterClassW(GlobalData::WindowClass->lpszClassName, GlobalData::WindowClass->hInstance);

    ReleaseWindowsClass();
}

void Render::Render() {
    ImGui::Render();

    GlobalStruct::FrameContext* frameCtx = WaitForNextFrameResources();
    UINT backBufferIdx = GlobalData::SwapChain->GetCurrentBackBufferIndex();
    frameCtx->CommandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = GlobalData::RenderTargetResource[backBufferIdx];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    GlobalData::CommandList->Reset(frameCtx->CommandAllocator, nullptr);
    GlobalData::CommandList->ResourceBarrier(1, &barrier);

    const float clear_color_with_alpha[4] = { 
        GlobalData::DefaultColor.x * GlobalData::DefaultColor.w,
        GlobalData::DefaultColor.y * GlobalData::DefaultColor.w,
        GlobalData::DefaultColor.z * GlobalData::DefaultColor.w,
        GlobalData::DefaultColor.w 
    };
    GlobalData::CommandList->ClearRenderTargetView(GlobalData::RenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);
    GlobalData::CommandList->OMSetRenderTargets(1, &GlobalData::RenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
    GlobalData::CommandList->SetDescriptorHeaps(1, &GlobalData::SrvDescHeap);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), GlobalData::CommandList);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    GlobalData::CommandList->ResourceBarrier(1, &barrier);
    GlobalData::CommandList->Close();

    GlobalData::CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&GlobalData::CommandList);

    HRESULT hr = GlobalData::SwapChain->Present(1, 0);
    GlobalData::SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

    UINT64 fenceValue = GlobalData::FenceLastSignaledValue + 1;
    GlobalData::CommandQueue->Signal(GlobalData::Fence, fenceValue);
    GlobalData::FenceLastSignaledValue = fenceValue;
    frameCtx->FenceValue = fenceValue;
}