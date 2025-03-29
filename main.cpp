#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <string>
#include <vector>
#include <chrono>
#include <pdh.h>
#include <PdhMsg.h>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "pdh.lib")

struct SystemMetrics {
    float cpuUsage = 0.0f;
    float ramUsagePercent = 0.0f;
    ULONGLONG totalRamMB = 0;
    ULONGLONG usedRamMB = 0;
    float diskUsagePercent = 0.0f;
    ULONGLONG totalDiskGB = 0;
    ULONGLONG freeDiskGB = 0;
    float gpuUsage = 0.0f;
    float gpuTemperature = 0.0f;
};

PDH_HQUERY cpuQuery;
PDH_HCOUNTER cpuTotal;

void InitPdhQuery() {
    PdhOpenQuery(NULL, NULL, &cpuQuery);
    PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);
}

float GetCurrentCpuUsage() {
    PDH_FMT_COUNTERVALUE counterVal;
    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    return max(0.0, min(100.0, (float)counterVal.doubleValue));
}

void GetMemoryInfo(SystemMetrics& metrics) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        metrics.totalRamMB = memInfo.ullTotalPhys / (1024 * 1024);
        metrics.usedRamMB = (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024);
        metrics.ramUsagePercent = memInfo.dwMemoryLoad;
    }
}

void GetDiskInfo(SystemMetrics& metrics, const wchar_t* drive = L"C:\\") {
    ULARGE_INTEGER freeBytesAvailableToCaller;
    ULARGE_INTEGER totalNumberOfBytes;
    ULARGE_INTEGER totalNumberOfFreeBytes;

    if (GetDiskFreeSpaceExW(drive, &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        metrics.totalDiskGB = totalNumberOfBytes.QuadPart / (1024 * 1024 * 1024);
        metrics.freeDiskGB = totalNumberOfFreeBytes.QuadPart / (1024 * 1024 * 1024);
        ULONGLONG usedDiskBytes = totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart;
        if (totalNumberOfBytes.QuadPart > 0) {
            metrics.diskUsagePercent = (float)((double)usedDiskBytes / totalNumberOfBytes.QuadPart * 100.0);
        }
        else {
            metrics.diskUsagePercent = 0.0f;
        }
    }
}

void GetGpuInfo(SystemMetrics& metrics) {
    metrics.gpuUsage = 0.0f;
    metrics.gpuTemperature = 0.0f;
}

void UpdateSystemMetrics(SystemMetrics& metrics) {
    metrics.cpuUsage = GetCurrentCpuUsage();
    GetMemoryInfo(metrics);
    GetDiskInfo(metrics, L"C:\\");
    GetGpuInfo(metrics);
}

ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"SystemWidget", NULL };
    ::RegisterClassEx(&wc);

    HWND hwnd = ::CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        wc.lpszClassName, L"System Widget",
        WS_POPUP,
        50, 50, 250, 200,
        NULL, NULL, wc.hInstance, NULL);

    ::SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 240, LWA_ALPHA);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsClassic();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(10, 10);
    style.WindowRounding = 4.0f;
    style.FramePadding = ImVec2(5, 3);
    style.FrameRounding = 4.0f;
    style.ItemSpacing = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(4, 4);
    style.IndentSpacing = 21.0f;
    style.ScrollbarSize = 10.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 5.0f;
    style.GrabRounding = 3.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;

    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.85f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.54f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    SystemMetrics metrics;
    InitPdhQuery();
    auto lastUpdateTime = std::chrono::high_resolution_clock::now();
    const auto updateInterval = std::chrono::seconds(1);

    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        auto currentTime = std::chrono::high_resolution_clock::now();
        if (currentTime - lastUpdateTime >= updateInterval) {
            UpdateSystemMetrics(metrics);
            lastUpdateTime = currentTime;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoBackground;

            ImGui::Begin("SystemInfoOverlay", nullptr, window_flags);

            ImGui::Text("Veezy Stats");
            ImGui::Separator();

            char cpuText[64];
            sprintf_s(cpuText, "CPU: %.1f %%", metrics.cpuUsage);
            ImGui::Text(cpuText);
            ImGui::ProgressBar(metrics.cpuUsage / 100.0f, ImVec2(-1.0f, 0.0f), cpuText);

            char ramText[64];
            sprintf_s(ramText, "RAM: %.1f %% (%llu / %llu MB)", metrics.ramUsagePercent, metrics.usedRamMB, metrics.totalRamMB);
            ImGui::Text(ramText);
            ImGui::ProgressBar(metrics.ramUsagePercent / 100.0f, ImVec2(-1.0f, 0.0f), "");

            char diskText[64];
            sprintf_s(diskText, "Disk (C:): %.1f %% (%llu GB Free)", metrics.diskUsagePercent, metrics.freeDiskGB);
            ImGui::Text(diskText);
            ImGui::ProgressBar(metrics.diskUsagePercent / 100.0f, ImVec2(-1.0f, 0.0f), "");

            ImGui::Separator();
            ImGui::Text("GPU Usage: %.1f %% (Placeholder)", metrics.gpuUsage);
            ImGui::Text("GPU Temp:  %.1f C (Placeholder)", metrics.gpuTemperature);
            ImGui::TextDisabled("(GPU requires vendor SDKs)");

            ImGui::End();
        }

        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    PdhCloseQuery(cpuQuery);
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}