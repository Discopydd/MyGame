#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include <wrl.h>
#include<cassert>
#include"WinApp.h"
#include <array>
#include <vector>
#include <dxcapi.h>
#include <chrono>
#include <thread>
#pragma comment(lib, "dxcompiler.lib")
#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
#include "../externals/DirectXTex/DirectXTex.h"
#include "../externals/DirectXTex/d3dx12.h"


/// <summary>
/// Im Gui Impl Win 32 Wnd Proc Handler処理を実行します。
/// </summary>
/// <param name="hWnd">処理に使用するhWndの値。</param>
/// <param name="msg">処理に使用するmsgの値。</param>
/// <param name="wParam">処理に使用するwParamの値。</param>
/// <param name="lParam">処理に使用するlParamの値。</param>
/// <returns>計算または取得した結果。</returns>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace MyEngine {

/// <summary>
/// DirectX 12の初期化、コマンド実行、描画前後処理、GPUリソース生成を管理するクラス。
/// </summary>
class DirectXCommon {
public: // メンバ関数
	/// <summary>
	/// Device Initialize処理を実行します。
	/// </summary>
	void DeviceInitialize();
	/// <summary>
	/// Command Initialize処理を実行します。
	/// </summary>
	void CommandInitialize();
	/// <summary>
	/// Swap Chain Initialize処理を実行します。
	/// </summary>
	void SwapChainInitialize();
	/// <summary>
	/// Depth Buffer Initialize処理を実行します。
	/// </summary>
	void DepthBufferInitialize();
	/// <summary>
	/// Descriptor Heap Initialize処理を実行します。
	/// </summary>
	void DescriptorHeapInitialize();
	/// <summary>
	/// RTV Initialize処理を実行します。
	/// </summary>
	void RTVInitialize();
	/// <summary>
	/// DSV Initialize処理を実行します。
	/// </summary>
	void DSVInitialize();
	/// <summary>
	/// Fence Initialize処理を実行します。
	/// </summary>
	void FenceInitialize();
	/// <summary>
	/// Viewport Initialize処理を実行します。
	/// </summary>
	void ViewportInitialize();
	/// <summary>
	/// Scissor Initialize処理を実行します。
	/// </summary>
	void ScissorInitialize();
	/// <summary>
	/// Dxc Compiler Initialize処理を実行します。
	/// </summary>
	void DxcCompilerInitialize();
	/// <summary>
	/// Imgui Initialize処理を実行します。
	/// </summary>
	void ImguiInitialize();
public:
	//初期化
	/// <summary>
	/// 動作に必要な参照とリソースを設定し、初期状態を構築します。
	/// </summary>
	/// <param name="winApp">処理対象のオブジェクトへのポインタ。</param>
	void Initialize(WinApp* winApp);
	//描画前処理
	/// <summary>
	/// Begin処理を実行します。
	/// </summary>
	void Begin();
	//描画後処理
	/// <summary>
	/// End処理を実行します。
	/// </summary>
	void End();
	/// <summary>
	/// Depth Bufferをクリアします。
	/// </summary>
	void ClearDepthBuffer();
	/// <summary>
	/// 使用しているリソースを解放し、終了処理を行います。
	/// </summary>
	void Finalize();


	/// <summary>
	/// Begin Im Gui処理を実行します。
	/// </summary>
	void BeginImGui();

    /// <summary>
    /// Render Im Gui処理を実行します。
    /// </summary>
    void RenderImGui();
	/// <summary>
	/// Finalize Im Gui処理を実行します。
	/// </summary>
	void FinalizeImGui();

	/// <summary>
	/// DirectXCommonのインスタンスを生成します。
	/// </summary>
	DirectXCommon(const DirectXCommon&) = delete;
    /// <summary>
    /// 演算子「=」による計算結果を生成します。
    /// </summary>
    /// <returns>保持している値への参照。</returns>
    DirectXCommon& operator=(const DirectXCommon&) = delete;

    /// <summary>
    /// Instanceを取得します。
    /// </summary>
    /// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
    static DirectXCommon* GetInstance();
	//SRVの指定番号のCPUデスクリプタハンドルを取得
	/// <summary>
	/// SRVCPU Descriptor Handleを取得します。
	/// </summary>
	/// <param name="index">対象を示すインデックス。</param>
	/// <returns>計算または取得した結果。</returns>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	//SRVの指定番号のGPUデスクリプタハンドルを取得
	/// <summary>
	/// SRVGPU Descriptor Handleを取得します。
	/// </summary>
	/// <param name="index">対象を示すインデックス。</param>
	/// <returns>計算または取得した結果。</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

	//RTVの指定番号のCPUデスクリプタハンドルを取得
	/// <summary>
	/// RTVCPU Descriptor Handleを取得します。
	/// </summary>
	/// <param name="index">対象を示すインデックス。</param>
	/// <returns>計算または取得した結果。</returns>
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUDescriptorHandle(uint32_t index);

	//RTVの指定番号のGPUデスクリプタハンドルを取得
	/// <summary>
	/// RTVGPU Descriptor Handleを取得します。
	/// </summary>
	/// <param name="index">対象を示すインデックス。</param>
	/// <returns>計算または取得した結果。</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGPUDescriptorHandle(uint32_t index);

	//DSVの指定番号のCPUデスクリプタハンドルを取得
	/// <summary>
	/// DSVCPU Descriptor Handleを取得します。
	/// </summary>
	/// <param name="index">対象を示すインデックス。</param>
	/// <returns>計算または取得した結果。</returns>
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);

	//DSVの指定番号のGPUデスクリプタハンドルを取得
	/// <summary>
	/// DSVGPU Descriptor Handleを取得します。
	/// </summary>
	/// <param name="index">対象を示すインデックス。</param>
	/// <returns>計算または取得した結果。</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);

	/// <summary>
	/// CPU Descriptor Handleを取得します。
	/// </summary>
	/// <param name="descriptorHeap">処理に使用するdescriptorHeapの値。</param>
	/// <param name="descriptorSize">処理に使用するdescriptorSizeの値。</param>
	/// <param name="index">対象を示すインデックス。</param>
	/// <returns>計算または取得した結果。</returns>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap,
		uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// GPU Descriptor Handleを取得します。
	/// </summary>
	/// <param name="descriptorHeap">処理に使用するdescriptorHeapの値。</param>
	/// <param name="descriptorSize">処理に使用するdescriptorSizeの値。</param>
	/// <param name="index">対象を示すインデックス。</param>
	/// <returns>計算または取得した結果。</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap,
		uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// Deviceを取得します。
	/// </summary>
	/// <returns>計算または取得した結果。</returns>
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return device.Get(); }
	/// <summary>
	/// Command Listを取得します。
	/// </summary>
	/// <returns>計算または取得した結果。</returns>
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList()const { return commandList.Get(); }

	//CompileShader関数の作成
	/// <summary>
	/// Shaderをコンパイルします。
	/// </summary>
	/// <param name="filePath">読み込むファイルのパス。</param>
	/// <param name="profile">処理対象のオブジェクトへのポインタ。</param>
	/// <returns>計算または取得した結果。</returns>
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile);


	/// <summary>
	/// Buffer Resourceを生成します。
	/// </summary>
	/// <param name="sizeInBytes">処理に使用するsizeInBytesの値。</param>
	/// <returns>計算または取得した結果。</returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	/// <summary>
	/// Texture Resourceを生成します。
	/// </summary>
	/// <param name="metadata">処理に使用する参照値。</param>
	/// <returns>計算または取得した結果。</returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	[[nodiscard]]
	/// <summary>
	/// Upload Texture Data処理を実行します。
	/// </summary>
	/// <param name="texture">処理に使用するtextureの値。</param>
	/// <param name="mipImages">処理に使用する参照値。</param>
	/// <returns>計算または取得した結果。</returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource>texture, const DirectX::ScratchImage& mipImages);

	//テクスチャファイルの読み込み
	/// <summary>
	/// Textureを読み込みます。
	/// </summary>
	/// <param name="filePath">読み込むファイルのパス。</param>
	/// <returns>計算または取得した結果。</returns>
	DirectX::ScratchImage LoadTexture(const std::string& filePath);

	//デスクリプタヒープを生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>
		/// <summary>
		/// CreateDescriptorHeapのインスタンスを生成します。
		/// </summary>
		/// <param name="heapType">処理に使用するheapTypeの値。</param>
		/// <param name="numDescriptrs">処理に使用するnumDescriptrsの値。</param>
		/// <param name="shaderVisible">処理に使用するshaderVisibleの値。</param>
		CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType,
			UINT numDescriptrs, bool shaderVisible);
	// 最大SRV数(最大テクスチャ枚数)
	static const uint32_t kMaxSRVCount;

	/// <summary>
	/// Swap Chain Resources Numを取得します。
	/// </summary>
	/// <returns>計算または取得した数値。</returns>
	size_t GetSwapChainResourcesNum()const { return swapChainDesc.BufferCount; }
 
private: // メンバ変数
	// ウィンドウズアプリケーション管理
	WinApp* winApp_ = nullptr;
	HRESULT hr;

	 /// <summary>
	 /// DirectXCommonのインスタンスを生成します。
	 /// </summary>
	 DirectXCommon() = default;
    /// <summary>
    /// DirectXCommonが保持するリソースを破棄します。
    /// </summary>
    ~DirectXCommon() = default;
	// Direct3D関連
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2>swapChainResources;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;

	uint32_t descriptorSizeRTV;
	uint32_t descriptorSizeDSV;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStarHandle;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> rtvHandles;


	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	HANDLE fenceEvent;
	uint64_t fenceValue = 0;

	//ビューポート
	D3D12_VIEWPORT viewport{};

	//シザー矩形
	D3D12_RECT scissorRect{};

	//DXC
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;

	//barrier
	D3D12_RESOURCE_BARRIER barrier{};

	std::chrono::steady_clock::time_point reference_;

private: // メンバ関数


	//FPS固定初期化
	/// <summary>
	/// Initialize Fix FPS処理を実行します。
	/// </summary>
	void InitializeFixFPS();
	//FPS固定更新
	/// <summary>
	/// Fix FPSを更新します。
	/// </summary>
	void UpdateFixFPS();
};

} // namespace MyEngine
