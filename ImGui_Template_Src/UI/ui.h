#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

#include "imgui_internal.h"

#include <string>
#include <string_view>
#include <fstream>
#include <vector>
#include <functional>

#include "Hook.h"

namespace GlobalStruct {
    struct FrameContext {
        ID3D12CommandAllocator* CommandAllocator;
        UINT64                  FenceValue;
    };
}

namespace GlobalData {
    static GlobalStruct::FrameContext   FrameContext[3] = {};
    static UINT                         FrameIndex = 0;
    static ID3D12Device*                D12Device = nullptr;
    static ID3D12DescriptorHeap*        RtvDescHeap = nullptr;
    static ID3D12DescriptorHeap*        SrvDescHeap = nullptr;
    static ID3D12CommandQueue*          CommandQueue = nullptr;
    static ID3D12GraphicsCommandList*   CommandList = nullptr;
    static ID3D12Fence*                 Fence = nullptr;
    static HANDLE                       FenceEvent = nullptr;
    static UINT64                       FenceLastSignaledValue = 0;
    static IDXGISwapChain3*             SwapChain = nullptr;
    static bool                         SwapChainOccluded = false;
    static HANDLE                       SwapChainWaitableObject = nullptr;
    static ID3D12Resource*              RenderTargetResource[3] = {};
    static D3D12_CPU_DESCRIPTOR_HANDLE  RenderTargetDescriptor[3] = {};
    static UINT                         ResizeWidth = 0, ResizeHeight = 0;

    extern LPWNDCLASSEXW  WindowClass;
    extern HWND           WindowHwnd;
    
    extern UINT ScreenWidth;
    extern UINT ScreenHeight;

    static const ImVec4 DefaultColor = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

    extern Hook::HookInit   HookInit;
    extern Hook::HookRender HookRender;

    extern ImVec2 WindowPos;

    extern std::vector<std::function<void()>> UIFInterface;

    extern bool Init;
}

namespace ImGui_Cfg {
    namespace Switch {
        extern bool Enable_UTPT;
    }
}

#define InitUIBegin namespace InitUI {
#define InitUIEnd };

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace InitUI {
    void CreateFatherWindow();
    void CreateDevice12();
    void ShowWindow();
    void ImGui();
};

namespace ReleaseUI {
    void Release();
}

namespace Render {
    void Render();
    void UserInterface();
    
    template <class T>
    void Start(const T&& Address) {
        bool MainLoop = true;
        while (MainLoop) {
            MSG msg;
            while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                if (msg.message == WM_QUIT)
                    MainLoop = false;
            }
            if (!MainLoop)
                break;

            if (GlobalData::SwapChainOccluded && GlobalData::SwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
                ::Sleep(10);
                continue;
            }
            GlobalData::SwapChainOccluded = false;

            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            GlobalData::HookRender.ThreadCall(1);
            
            if (ImGui_Cfg::Switch::Enable_UTPT) {
                std::thread UsethreadingToProcessTask(Address);
                UsethreadingToProcessTask.join();
            }
            else
                Address();

            GlobalData::HookRender.ThreadCall(0);
        }

        ReleaseUI::Release();
    }

    template <class T>
    void AddUI(const T&& Function) {
        return GlobalData::UIFInterface.push_back(Function);
    }
}

namespace InitHook {
    void SetUIInitHook();
    void SetRenderHook();
}