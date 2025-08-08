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


class SpriteCommon;
class Sprite
{
public:
	//初期化
	void Initialize(SpriteCommon*spriteCommon, std::string textureFilePath);
    //更新
	void Update();
	//描画
	void Draw();
	// サイズ
	const Vector2& GetSize()const { return size; }
	void SetSize(const Vector2& size) { this->size = size; }

	// ポジション
	const Vector2& GetPosition()const { return position; }
	void SetPosition(const Vector2& position) { this->position = position; }

	// 回転
	const float& GetRotation()const { return rotation; }
	void SetRotation(const float& rotation) { this->rotation = rotation; }

	// 色
	const Vector4& GetColor()const { return materialData->color; }
	void setColor(const Vector4& color) { materialData->color = color; }

	// アンカー
	const Vector2& GetAnchorPoint()const { return anchorPoint_; }
	void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }

	// 左右フリップ
	const bool& GetIsFlipX()const { return isFlipX_; }
	void SetIsFlipX(const bool& isFlipX) { isFlipX_ = isFlipX; }

	// 上下フリップ
	const bool& GetIsFlipY()const { return isFlipY_; }
	void SetIsFlipY(const bool& isFlipY) { isFlipY_ = isFlipY; }

	// テクスチャ左上
	const Vector2& GetTextureLeftTop()const { return textureLeftTop_; }
	void SetTextureLeftTop(const Vector2& textureLeftTop) { textureLeftTop_ = textureLeftTop; }

	// テクスチャサイズ
	const Vector2& GetTextureSize()const { return textureSize_; }
	void SetTextureSize(const Vector2& textureSize) { textureSize_ = textureSize; }
private:

	//テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();

	SpriteCommon* spriteCommon = nullptr;
	SrvManager* srvManager = nullptr;
	//頂点データ作成
	void VertexDataCreate();
	//index作成
	void IndexCreate();
	//マテリアル作成
	void MaterialCreate();
	//座標変換行列データ作成
	void TransformationCreate();

	void CameraCreate();
	void DirectionalLightCreate();
	void PointLightCreate();
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
	//テクスチャ左上座標
	Vector2 textureLeftTop_ = { 0.0f,0.0f };
	//テクスチャ切り出しサイズ
	Vector2 textureSize_ = { 512.0f,512.0f };
};
