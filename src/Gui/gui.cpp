#include "gui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "MemoryManager/MemoryManager.h"
#include "guimain.h"
#include "guisetting.h"
#include <iostream>

// 全局变量定义
ID3D11Device* g_pDevice = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11DeviceContext* g_pDeviceContext = nullptr;
HWND g_hwnd = nullptr;

// ImGui_ImplWin32_WndProcHandler 声明
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool InitGUI() {
    // 注册窗口类
    const char* wndClassName = "MemoryEditor";
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, wndClassName, NULL };
    if (!RegisterClassEx(&wc)) {
        std::cerr << "注册窗口类失败: " << GetLastError() << std::endl;
        return false;
    }

    // 创建窗口
    g_hwnd = CreateWindow(wndClassName, "Launch", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, wc.hInstance, NULL);
    if (!g_hwnd) {
        std::cerr << "创建窗口失败: " << GetLastError() << std::endl;
        return false;
    }
    ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hwnd);

    // DirectX 初始化
    D3D_FEATURE_LEVEL featureLevel;
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = g_hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &swapChainDesc, &g_pSwapChain, &g_pDevice, &featureLevel, &g_pDeviceContext);
    if (FAILED(hr)) {
        PrintHResultError(hr, "创建 DirectX 设备");
        return false;
    }

    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (FAILED(hr)) {
        PrintHResultError(hr, "获取后缓冲区");
        return false;
    }
    hr = g_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr)) {
        PrintHResultError(hr, "创建渲染目标");
        return false;
    }

    // ImGui 初始化
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // 加载字体
    io.Fonts->Clear();
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    if (!font) {
        font = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/simsun.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
        if (!font) {
            font = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/meiryo.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
            if (!font) {
                io.Fonts->AddFontDefault();
            }
        }
    }

    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(g_hwnd) || !ImGui_ImplDX11_Init(g_pDevice, g_pDeviceContext)) {
        std::cerr << "ImGui 初始化失败！" << std::endl;
        return false;
    }

    return true;
}

void RenderFrame() {
    ImGui::Render();
    g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
    const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };  // 黑色背景
    g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView, clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    g_pSwapChain->Present(1, 0);
}

void RunGUILoop() {
    MemoryManager* memoryManager = nullptr;
    bool processAttached = false;
    std::string statusMsg = "";

    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // 注册窗口（注册式，便于扩展）
            RegisterMainWindow(memoryManager, processAttached, statusMsg);
            RegisterSettingWindow();

            RenderFrame();
        }
    }

    if (memoryManager) delete memoryManager;
}

void ShutdownGUI() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pDeviceContext) g_pDeviceContext->Release();
    if (g_pDevice) g_pDevice->Release();
    DestroyWindow(g_hwnd);
    UnregisterClass("MemoryEditor", GetModuleHandle(NULL));
}