#include "GUI/gui.h"
#include <iostream>
#include <Windows.h>
#include <d3d11.h>
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "GUI/guimain.h"
#include "GUI/guisetting.h"
#include "MemoryManager/MemoryManager.h"
#define SECURITY_WIN32
#include <security.h>

extern ID3D11Device* g_pDevice;
extern IDXGISwapChain* g_pSwapChain;
extern ID3D11RenderTargetView* g_pRenderTargetView;
extern ID3D11DeviceContext* g_pDeviceContext;
extern HWND g_hwnd;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void PrintHResultError(HRESULT hr, const char* operation) {
    char errorMsg[256];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, hr, 0, errorMsg, sizeof(errorMsg), nullptr);
    std::cerr << operation << " 失败，错误码: 0x" << std::hex << hr << " - " << errorMsg << std::endl;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;
    switch (uMsg) {
        case WM_SIZE:
            if (g_pDevice && wParam != SIZE_MINIMIZED) {
                if (g_pRenderTargetView) {
                    g_pRenderTargetView->Release();
                    g_pRenderTargetView = nullptr;
                }
                HRESULT hr = g_pSwapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_R8G8B8A8_UNORM, 0);
                if (FAILED(hr)) {
                    PrintHResultError(hr, "调整交换链");
                    return 0;
                }
                ID3D11Texture2D* pBackBuffer = nullptr;
                hr = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
                if (FAILED(hr)) {
                    PrintHResultError(hr, "获取后缓冲区");
                    return 0;
                }
                hr = g_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
                pBackBuffer->Release();
                if (FAILED(hr)) {
                    PrintHResultError(hr, "创建渲染目标");
                    return 0;
                }
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
            break;
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool IsAdmin() {
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY ntAuth = SECURITY_NT_AUTHORITY;
    PSID adminGroup;
    AllocateAndInitializeSid(&ntAuth, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup);
    CheckTokenMembership(NULL, adminGroup, &isAdmin);
    FreeSid(adminGroup);
    return isAdmin;
}

int main() {
    if (!IsAdmin()) {
        std::cerr << "警告: 请以管理员运行以附加进程！" << std::endl;
    }
    if (!InitGUI()) {
        std::cerr << "GUI 初始化失败，程序退出！" << std::endl;
        return 1;
    }

    RunGUILoop();
    ShutdownGUI();

    return 0;
}