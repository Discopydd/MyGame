
#include"WinApp.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	  // ImGui 関連のメッセージ処理
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
        return true;
    }

    // メッセージに応じてゲーム固有の処理を行う
    switch (msg) {
    // ウィンドウが破棄された場合
    case WM_DESTROY:
        // OSに対して、アプリの終了を伝える
        PostQuitMessage(0);
        return 0;
    }

    // 標準のメッセージ処理を行う
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::Initialize()
{
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

    assert(SUCCEEDED(hr));

    // 设置窗口类
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = L"WindowClass";
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&wc);


RECT wrc = { 0, 0, kClientWidth, kClientHeight };
AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

hwnd = CreateWindow(
    wc.lpszClassName,
    L"ポータル・リープ",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    wrc.right - wrc.left,
    wrc.bottom - wrc.top,
    nullptr,
    nullptr,
    wc.hInstance,
    nullptr
);
ShowWindow(hwnd, SW_SHOW);


}

void WinApp::Finalize()
{
    CloseWindow(hwnd);
    CoUninitialize();
}

bool WinApp::ProcessMessage()
{
    MSG msg = {};

    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (msg.message == WM_QUIT)
    {
        return true;
    }
    return false;
}
WinApp* WinApp::GetInstance() {
    static WinApp instance;
    return &instance;
}