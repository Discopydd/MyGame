#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "SrvManager.h"

#ifdef USE_IMGUI
#include "../externals//imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
#endif

class ImGuiManager
{
public:
	//終了
	void Finalize();
	//初期化
	void Initialize(WinApp*windowsAPI,DirectXCommon*directXCommon,SrvManager*srvmanager);
	//ImGui受付開始
	void Begin();
	//ImGui受付終了
	void End();
	//画面への描画
	void Draw();
private:
	WinApp* windowsAPI_;
	DirectXCommon* directXCommon_;
	SrvManager* srvmanager_;
	uint32_t index;
};