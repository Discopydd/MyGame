#pragma once
#include "../base/DirectXCommon.h"

class SpriteCommon
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);
	//共通描画設定
	void CommonDraw();

	DirectXCommon* GetDxCommon()const { return dxCommon_; }
private:

	HRESULT hr;
	//ルートシグネチャの作成
	void RootSignatureInitialize();

	//グラフィックスパイプライン
	void GraphicsPipelineInitialize();

	DirectXCommon* dxCommon_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
};