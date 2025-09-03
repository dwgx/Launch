#ifndef GUI_H
#define GUI_H

#include <windows.h>
#include <d3d11.h>
#include <string>

extern ID3D11Device* g_pDevice;
extern IDXGISwapChain* g_pSwapChain;
extern ID3D11RenderTargetView* g_pRenderTargetView;
extern ID3D11DeviceContext* g_pDeviceContext;
extern HWND g_hwnd;

void PrintHResultError(HRESULT hr, const char* operation);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool InitGUI();
void RenderFrame();
void RunGUILoop();
void ShutdownGUI();

#endif // GUI_H