#pragma once
#include "../base/DirectXCommon.h"

namespace MyEngine {

/// <summary>
/// スプライト描画で共通使用するルートシグネチャとパイプラインを管理するクラス。
/// </summary>
class SpriteCommon
{
public:
	static SpriteCommon* GetInstance() {
		static SpriteCommon instance;
		return &instance;
	}

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);
	//共通描画設定
	void CommonDraw();

	DirectXCommon* GetDxCommon()const { return dxCommon_; }
private:
	SpriteCommon() = default;
    ~SpriteCommon() = default;
    SpriteCommon(const SpriteCommon&) = delete;
    SpriteCommon& operator=(const SpriteCommon&) = delete;
	HRESULT hr;
	//ルートシグネチャの作成
	void RootSignatureInitialize();

	//グラフィックスパイプライン
	void GraphicsPipelineInitialize();

	DirectXCommon* dxCommon_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
};

} // namespace MyEngine
