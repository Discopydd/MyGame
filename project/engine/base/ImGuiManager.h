#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "SrvManager.h"

#ifdef USE_IMGUI
#include "../externals//imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
#endif

namespace MyEngine {

/// <summary>
/// ImGuiManagerに関する処理と状態を管理するクラスです。
/// </summary>
class ImGuiManager
{
public:
	//終了
	/// <summary>
	/// 使用しているリソースを解放し、終了処理を行います。
	/// </summary>
	void Finalize();
	//初期化
	/// <summary>
	/// 動作に必要な参照とリソースを設定し、初期状態を構築します。
	/// </summary>
	/// <param name="windowsAPI">処理対象のオブジェクトへのポインタ。</param>
	/// <param name="directXCommon">処理対象のオブジェクトへのポインタ。</param>
	/// <param name="srvmanager">処理対象のオブジェクトへのポインタ。</param>
	void Initialize(WinApp*windowsAPI,DirectXCommon*directXCommon,SrvManager*srvmanager);
	//ImGui受付開始
	/// <summary>
	/// Begin処理を実行します。
	/// </summary>
	void Begin();
	//ImGui受付終了
	/// <summary>
	/// End処理を実行します。
	/// </summary>
	void End();
	//画面への描画
	/// <summary>
	/// 現在の状態を画面へ描画します。
	/// </summary>
	void Draw();
private:
	WinApp* windowsAPI_;
	DirectXCommon* directXCommon_;
	SrvManager* srvmanager_;
	uint32_t index;
};

} // namespace MyEngine
