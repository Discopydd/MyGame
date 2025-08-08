#pragma once
#include <Windows.h>
#include <cstdint>
#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
class WinApp {
private:
    HWND hwnd = nullptr;
    WNDCLASS wc{};
    WinApp() = default; // 禁止外部构造
    ~WinApp() = default;

public:
    WinApp(const WinApp&) = delete;
    WinApp& operator=(const WinApp&) = delete;
    static WinApp* GetInstance();
    static constexpr int32_t kClientWidth = 1280;
    static constexpr int32_t kClientHeight = 720;

    void Initialize();
    // 
    bool ProcessMessage();
    // 
    void Finalize();

    HWND GetHwnd() const { return hwnd; }
    HINSTANCE GetHInstance() const { return wc.hInstance; }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};