#pragma once
#include <DirectXCommon.h>
namespace MyEngine {

/// <summary>
/// CBV、SRV、UAV用ディスクリプタヒープの生成と割り当てを管理するクラス。
/// </summary>
class SrvManager
{
public://初期化
	/// <summary>
	/// 動作に必要な参照とリソースを設定し、初期状態を構築します。
	/// </summary>
	/// <param name="dxCommon">処理対象のオブジェクトへのポインタ。</param>
	void Initialize(DirectXCommon* dxCommon);
	//SRV生成(テクスチャ用)
	/// <summary>
	/// SR Vfor Texture 2 Dを生成します。
	/// </summary>
	/// <param name="srvIndex">対象を示すインデックス。</param>
	/// <param name="pResource">処理対象のオブジェクトへのポインタ。</param>
	/// <param name="Format">処理に使用するFormatの値。</param>
	/// <param name="MipLevels">処理に使用するMipLevelsの値。</param>
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);
	//SRV生成(Structured Buffer用)
	/// <summary>
	/// SR Vfor Structured Bufferを生成します。
	/// </summary>
	/// <param name="srvIndex">対象を示すインデックス。</param>
	/// <param name="pResource">処理対象のオブジェクトへのポインタ。</param>
	/// <param name="numElements">処理に使用するnumElementsの値。</param>
	/// <param name="structureByteStride">処理に使用するstructureByteStrideの値。</param>
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);
	//ヒープセットコマンド
	/// <summary>
	/// Pre Draw処理を実行します。
	/// </summary>
	void PreDraw();
	//SRVセットコマンド
	/// <summary>
	/// Graphics Root Descriptor Tableを設定します。
	/// </summary>
	/// <param name="RootParameterIndex">対象を示すインデックス。</param>
	/// <param name="srvIndex">対象を示すインデックス。</param>
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);
	//確保可能かチェック
	/// <summary>
	/// Securedcheck処理を実行します。
	/// </summary>
	/// <returns>判定結果。</returns>
	bool Securedcheck();
	/// <summary>
	/// Allocate処理を実行します。
	/// </summary>
	/// <returns>計算または取得した数値。</returns>
	uint32_t Allocate();
	//CPUハンドル計算
	/// <summary>
	/// CPU Descriptor Handleを取得します。
	/// </summary>
	/// <param name="index">対象を示すインデックス。</param>
	/// <returns>計算または取得した結果。</returns>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	//GPUハンドル計算
	/// <summary>
	/// GPU Descriptor Handleを取得します。
	/// </summary>
	/// <param name="index">対象を示すインデックス。</param>
	/// <returns>計算または取得した結果。</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);
	//最大SRV数(最大テクスチャ数)
	static const uint32_t kMaxSRVCount;
	//デスクリプタハンドル取得
 	/// <summary>
 	/// Descriptor Heapを取得します。
 	/// </summary>
 	/// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
 	ID3D12DescriptorHeap* GetDescriptorHeap() { return descriptorHeap.Get(); }

	  /// <summary>
	  /// SrvManagerのインスタンスを生成します。
	  /// </summary>
	  SrvManager(const SrvManager&) = delete;
    /// <summary>
    /// 演算子「=」による計算結果を生成します。
    /// </summary>
    /// <returns>保持している値への参照。</returns>
    SrvManager& operator=(const SrvManager&) = delete;

    /// <summary>
    /// Instanceを取得します。
    /// </summary>
    /// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
    static SrvManager* GetInstance();

	/// <summary>
	/// 使用しているリソースを解放し、終了処理を行います。
	/// </summary>
	void Finalize();
private:
	DirectXCommon* directXCommon = nullptr;
	//SRV用DescriptorSizeを取得
	uint32_t descriptorSize;
	//次に使用するSRVindex
	uint32_t useIndex = 0;
	//SRV用デスクリプターヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>descriptorHeap;

	/// <summary>
	/// SrvManagerのインスタンスを生成します。
	/// </summary>
	SrvManager() = default;
    /// <summary>
    /// SrvManagerが保持するリソースを破棄します。
    /// </summary>
    ~SrvManager() = default;
};


} // namespace MyEngine
