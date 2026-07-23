#pragma once
#include "../base/DirectXCommon.h"

namespace MyEngine {

/// <summary>
/// スプライト描画で共通使用するルートシグネチャとパイプラインを管理するクラス。
/// </summary>
class SpriteCommon
{
public:
	/// <summary>
	/// Instanceを取得します。
	/// </summary>
	/// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
	static SpriteCommon* GetInstance() {
		static SpriteCommon instance;
		return &instance;
	}

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);
	//共通描画設定
	/// <summary>
	/// Common Draw処理を実行します。
	/// </summary>
	void CommonDraw();

	/// <summary>
	/// Dx Commonを取得します。
	/// </summary>
	/// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
private:
	/// <summary>
	/// SpriteCommonのインスタンスを生成します。
	/// </summary>
	SpriteCommon() = default;
    /// <summary>
    /// SpriteCommonが保持するリソースを破棄します。
    /// </summary>
    ~SpriteCommon() = default;
    /// <summary>
    /// SpriteCommonのインスタンスを生成します。
    /// </summary>
    SpriteCommon(const SpriteCommon&) = delete;
    /// <summary>
    /// 演算子「=」による計算結果を生成します。
    /// </summary>
    /// <returns>保持している値への参照。</returns>
    SpriteCommon& operator=(const SpriteCommon&) = delete;
	HRESULT hr;
	//ルートシグネチャの作成
	/// <summary>
	/// Root Signature Initialize処理を実行します。
	/// </summary>
	void RootSignatureInitialize();

	//グラフィックスパイプライン
	/// <summary>
	/// Graphics Pipeline Initialize処理を実行します。
	/// </summary>
	void GraphicsPipelineInitialize();

	DirectXCommon* dxCommon_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
};

} // namespace MyEngine
