#pragma once
#include <Windows.h>
#include <cstdint>
#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
namespace MyEngine {

/// <summary>
/// WinAppに関する処理と状態を管理するクラスです。
/// </summary>
class WinApp {
private:
    HWND hwnd = nullptr;
    WNDCLASS wc{};
    /// <summary>
    /// WinAppのインスタンスを生成します。
    /// </summary>
    WinApp() = default; // 外部からの生成を禁止
    /// <summary>
    /// WinAppが保持するリソースを破棄します。
    /// </summary>
    ~WinApp() = default;

public:
    /// <summary>
    /// WinAppのインスタンスを生成します。
    /// </summary>
    WinApp(const WinApp&) = delete;
    /// <summary>
    /// 演算子「=」による計算結果を生成します。
    /// </summary>
    /// <returns>保持している値への参照。</returns>
    WinApp& operator=(const WinApp&) = delete;
    /// <summary>
    /// Instanceを取得します。
    /// </summary>
    /// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
    static WinApp* GetInstance();
    static constexpr int32_t kClientWidth = 1280;
    static constexpr int32_t kClientHeight = 720;

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    void Initialize();
    //
    /// <summary>
    /// Process Message処理を実行します。
    /// </summary>
    /// <returns>判定結果。</returns>
    bool ProcessMessage();
    //
    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize();

    /// <summary>
    /// Hwndを取得します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
    HWND GetHwnd() const { return hwnd; }
    /// <summary>
    /// H Instanceを取得します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
    HINSTANCE GetHInstance() const { return wc.hInstance; }

private:
    /// <summary>
    /// Window Proc処理を実行します。
    /// </summary>
    /// <param name="hwnd">処理に使用するhwndの値。</param>
    /// <param name="msg">処理に使用するmsgの値。</param>
    /// <param name="wparam">処理に使用するwparamの値。</param>
    /// <param name="lparam">処理に使用するlparamの値。</param>
    /// <returns>計算または取得した結果。</returns>
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

} // namespace MyEngine
