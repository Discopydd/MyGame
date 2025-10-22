#include "ImGuiManager.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"

//終了
void ImGuiManager::Finalize() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
}

//初期化
void ImGuiManager::Initialize([[maybe_unused]]WinApp* windowsAPI,DirectXCommon* directXCommon,[[maybe_unused]] SrvManager* srvmanager){
#ifdef USE_IMGUI
	windowsAPI_ = windowsAPI;
	directXCommon_ = directXCommon;
	srvmanager_ = srvmanager;
    
	index = srvmanager_->Allocate();

	//ImGuiのコンテキストを生成
	ImGui::CreateContext();
	//ImGuiのスタイルを設定
	ImGui::StyleColorsDark();
	//win32用初期化
	ImGui_ImplWin32_Init(windowsAPI_->GetHwnd());
	//////DirectX12用初期化//////
	ImGui_ImplDX12_Init(directXCommon_->GetDevice().Get(),
		static_cast<int>(directXCommon_->GetSwapChainResourcesNum()),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,srvmanager_->GetDescriptorHeap(),
		srvmanager_->GetCPUDescriptorHandle(index),srvmanager_->GetGPUDescriptorHandle(index));
#endif
}

//ImGui受付開始
void ImGuiManager::Begin() {
#ifdef USE_IMGUI
	//ImGuiフレーム開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif
}

//ImGui受付終了
void ImGuiManager::End() {
#ifdef USE_IMGUI
	//描画前準備
	ImGui::Render();
#endif
}

//画面への描画
void ImGuiManager::Draw() {
#ifdef USE_IMGUI
	ID3D12GraphicsCommandList* commandList = directXCommon_->GetCommandList().Get();

    ImDrawData* dd = ImGui::GetDrawData();
    // 关键判空/判尺寸/判命令条数 — 没数据就直接跳过，避免崩溃
    if (dd == nullptr || dd->CmdListsCount == 0 || dd->DisplaySize.x <= 0.0f || dd->DisplaySize.y <= 0.0f) {
        return;
    }
	//デスクリプタヒープの配列をセットするコマンド
	srvmanager_->PreDraw();
	//描画コマンドを発行
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif
}