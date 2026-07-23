#pragma once

#include "../base/DirectXCommon.h"
#include"Camera.h"
namespace MyEngine {

/// <summary>
/// 3Dオブジェクト描画で共通使用するパイプライン、カメラ、DirectXリソースを管理するクラス。
/// </summary>
class Object3dCommon
{
	public:

	// 初期化
	/// <summary>
	/// 動作に必要な参照とリソースを設定し、初期状態を構築します。
	/// </summary>
	/// <param name="dxCommon">処理対象のオブジェクトへのポインタ。</param>
	void Initialize(DirectXCommon* dxCommon);

	//共通描画設定
	/// <summary>
	/// Common Draw処理を実行します。
	/// </summary>
	void CommonDraw();

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

	//DXCommon
	/// <summary>
	/// Dx Commonを取得します。
	/// </summary>
	/// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	//setter
	/// <summary>
	/// Default Cameraを設定します。
	/// </summary>
	/// <param name="camera">描画および座標変換に使用するカメラ。</param>
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	//getter
	/// <summary>
	/// Default Cameraを取得します。
	/// </summary>
	/// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
	Camera* GetDefaultCamera()const { return defaultCamera; }
	private:

	DirectXCommon* dxCommon_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	Camera* defaultCamera = nullptr;
};

} // namespace MyEngine
