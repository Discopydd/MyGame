#pragma once
#include "../math/Vector2.h"
#include "../math/Vector3.h"
#include "../math/Vector4.h"
#include "../math/Matrix4x4.h"
#include "../math/Transform.h"
#include "../externals/DirectXTex/d3dx12.h"
#include"../3d/Material.h"
#include"VertexData.h"
#include"TransformationMatrix.h"
#include "SrvManager.h"
#include <CameraForGPU.h>
#include <DirectionalLight.h>
#include <PointLight.h>
#include <SpotLight.h>

#include <string>
#include <algorithm>
namespace MyEngine {

class SpriteCommon;
/// <summary>
/// Spriteに関する処理と状態を管理するクラスです。
/// </summary>
class Sprite
{
public:
	//初期化
	/// <summary>
	/// 動作に必要な参照とリソースを設定し、初期状態を構築します。
	/// </summary>
	/// <param name="spriteCommon">スプライト生成に使用する共通描画処理。</param>
	/// <param name="textureFilePath">使用するテクスチャファイルのパス。</param>
	void Initialize(SpriteCommon*spriteCommon, std::string textureFilePath);
    //更新
	/// <summary>
	/// 入力や経過時間に応じて、状態を1フレーム分更新します。
	/// </summary>
	void Update();
	//描画
	/// <summary>
	/// 現在の状態を画面へ描画します。
	/// </summary>
	void Draw();
	// サイズ
	/// <summary>
	/// Sizeを取得します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	const Vector2& GetSize()const { return size; }
	/// <summary>
	/// Sizeを設定します。
	/// </summary>
	/// <param name="size">処理に使用する参照値。</param>
	void SetSize(const Vector2& size) { this->size = size; }

	// ポジション
	/// <summary>
	/// 現在のワールド座標を取得します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	const Vector2& GetPosition()const { return position; }
	/// <summary>
	/// プレイヤーのワールド座標を設定し、描画モデルへ反映します。
	/// </summary>
	/// <param name="position">対象のワールド座標。</param>
	void SetPosition(const Vector2& position) { this->position = position; }

	// 回転
	/// <summary>
	/// Rotationを取得します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	const float& GetRotation()const { return rotation; }
	/// <summary>
	/// Rotationを設定します。
	/// </summary>
	/// <param name="rotation">処理に使用する参照値。</param>
	void SetRotation(const float& rotation) { this->rotation = rotation; }

	// 色
	/// <summary>
	/// Colorを取得します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	const Vector4& GetColor()const { return materialData->color; }
	/// <summary>
	/// Colorを設定します。
	/// </summary>
	/// <param name="color">処理に使用する参照値。</param>
	void SetColor(const Vector4& color) { materialData->color = color; }

	// アンカー
	/// <summary>
	/// Anchor Pointを取得します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	const Vector2& GetAnchorPoint()const { return anchorPoint_; }
	/// <summary>
	/// Anchor Pointを設定します。
	/// </summary>
	/// <param name="anchorPoint">処理に使用する参照値。</param>
	void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }

	// 左右フリップ
	/// <summary>
	/// Is Flip Xを取得します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	const bool& GetIsFlipX()const { return isFlipX_; }
	/// <summary>
	/// Is Flip Xを設定します。
	/// </summary>
	/// <param name="isFlipX">条件を有効にする場合は true。</param>
	void SetIsFlipX(const bool& isFlipX) { isFlipX_ = isFlipX; }

	// 上下フリップ
	/// <summary>
	/// Is Flip Yを取得します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	const bool& GetIsFlipY()const { return isFlipY_; }
	/// <summary>
	/// Is Flip Yを設定します。
	/// </summary>
	/// <param name="isFlipY">条件を有効にする場合は true。</param>
	void SetIsFlipY(const bool& isFlipY) { isFlipY_ = isFlipY; }

	// テクスチャ左上
	/// <summary>
	/// Texture Left Topを取得します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	const Vector2& GetTextureLeftTop()const { return textureLeftTop_; }
	/// <summary>
	/// Texture Left Topを設定します。
	/// </summary>
	/// <param name="textureLeftTop">処理に使用する参照値。</param>
	void SetTextureLeftTop(const Vector2& textureLeftTop) { textureLeftTop_ = textureLeftTop; }

	// テクスチャサイズ
	/// <summary>
	/// Texture Sizeを取得します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	const Vector2& GetTextureSize()const { return textureSize_; }
	/// <summary>
	/// Texture Sizeを設定します。
	/// </summary>
	/// <param name="textureSize">処理に使用する参照値。</param>
	void SetTextureSize(const Vector2& textureSize) { textureSize_ = textureSize; }

	/// <summary>
	/// Visibleを設定します。
	/// </summary>
	/// <param name="visible">表示する場合は true。</param>
	void SetVisible(bool visible) { isVisible_ = visible; }
    /// <summary>
    /// Visibleかを判定します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool IsVisible() const { return isVisible_; }

private:

	//テクスチャサイズをイメージに合わせる
	/// <summary>
	/// Adjust Texture Size処理を実行します。
	/// </summary>
	void AdjustTextureSize();
	/// <summary>
	/// Ends With Gif処理を実行します。
	/// </summary>
	/// <param name="s">演算に使用するスカラー値。</param>
	/// <returns>判定結果。</returns>
	static bool EndsWithGif(const std::string& s) {
		auto pos = s.find_last_of('.');
		if (pos == std::string::npos) return false;
		std::string ext = s.substr(pos + 1);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		return ext == "gif";
	}
	SpriteCommon* spriteCommon = nullptr;
	SrvManager* srvManager = nullptr;
	//頂点データ作成
	/// <summary>
	/// Vertex Data Create処理を実行します。
	/// </summary>
	void VertexDataCreate();
	//index作成
	/// <summary>
	/// Index Create処理を実行します。
	/// </summary>
	void IndexCreate();
	//マテリアル作成
	/// <summary>
	/// Material Create処理を実行します。
	/// </summary>
	void MaterialCreate();
	//座標変換行列データ作成
	/// <summary>
	/// ation Createを変換します。
	/// </summary>
	void TransformationCreate();

	/// <summary>
	/// Camera Create処理を実行します。
	/// </summary>
	void CameraCreate();
	/// <summary>
	/// Directional Light Create処理を実行します。
	/// </summary>
	void DirectionalLightCreate();
	/// <summary>
	/// Point Light Create処理を実行します。
	/// </summary>
	void PointLightCreate();
	/// <summary>
	/// Spot Light Create処理を実行します。
	/// </summary>
	void SpotLightCreate();
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;
	//バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	Material* materialData = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;
	
	CameraForGPU* cameraData = nullptr;
	// 平行光源用のResourceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	DirectionalLight* directionalLightData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource;
	PointLight* pointLightData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource;
	SpotLight* spotLightData = nullptr;
	//バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	// 設定用SRT
	Vector2 size = { 640.0f,360.0f };
	Vector2 position = { 0.0f,0.0f };
	float rotation = 0.0f;


	uint32_t textureIndex = 0;

	// アンカーポイント 中心位置を変えれる
	Vector2 anchorPoint_ = { 0.0f,0.0f };
	// 左右フリップ
	bool isFlipX_ = false;
	// 上下フリップ
	bool isFlipY_ = false;
	//ファイルパス
 	std::string filePath;
	bool isGif_ = false;
	//テクスチャ左上座標
	Vector2 textureLeftTop_ = { 0.0f,0.0f };
	//テクスチャ切り出しサイズ
	Vector2 textureSize_ = { 512.0f,512.0f };

	bool isVisible_ = true;
};

} // namespace MyEngine
