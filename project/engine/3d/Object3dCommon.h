#pragma once

#include "../base/DirectXCommon.h"
#include"Camera.h"
class Object3dCommon
{
	public:

	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	//共通描画設定
	void CommonDraw();

	//ルートシグネチャの作成
	void RootSignatureInitialize();
	//グラフィックスパイプライン
	void GraphicsPipelineInitialize();

	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	//setter
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	//getter
	Camera* GetDefaultCamera()const { return defaultCamera; }
	private:

	DirectXCommon* dxCommon_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	Camera* defaultCamera = nullptr;
};