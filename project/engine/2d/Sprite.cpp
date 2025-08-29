#include "Sprite.h"
#include "SpriteCommon.h"
#include "../math/MyMath.h"
#include "../math/Transform.h"
#include "../base/TextureManager.h"
#include <numbers>

//初期化
void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath) {
	this->spriteCommon = spriteCommon;
	TextureManager::GetInstance()->LoadTexture(textureFilePath);
	filePath = textureFilePath;
	VertexDataCreate();
	IndexCreate();
	MaterialCreate();
	CameraCreate();
	DirectionalLightCreate();
	PointLightCreate();
	SpotLightCreate();
	TransformationCreate();
	AdjustTextureSize();
}
//更新
void Sprite::Update() {
	// CPUで動かす用のTransformを作る
	transform.rotate = { 0.0f,0.0f,rotation };
	transform.translate = { position.x,position.y,0.0f };
	transform.scale = { size.x,size.y,1.0f, };

	//アンカーポイント
	float left = 0.0f - anchorPoint_.x;
	float right = 1.0f - anchorPoint_.x;
	float top = 0.0f - anchorPoint_.y;
	float bottom = 1.0f - anchorPoint_.y;

	//左右反転
	if (isFlipX_) {
		left = -left;
		right = -right;
	}
	//上下反転
	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
	}

	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(filePath);
	float tex_left = textureLeftTop_.x / metadata.width;
	float tex_right = (textureLeftTop_.x + textureSize_.x) / metadata.width;
	float tex_top = textureLeftTop_.y / metadata.height;
	float tex_bottom = (textureLeftTop_.y + textureSize_.y) / metadata.height;

	//VertexResourceにデータを書き込むためのアドレス取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 頂点リソースにデータを書き込む
	vertexData[0].position = { left,bottom,0.0f,1.0f };	 //左下
	vertexData[1].position = { left,top,0.0f,1.0f };	 //左上
	vertexData[2].position = { right,bottom,0.0f,1.0f }; //右下
	vertexData[3].position = { right,top,0.0f,1.0f };    //右上

	vertexData[0].texcoord = { tex_left,tex_bottom };    //左下
	vertexData[1].texcoord = { tex_left,tex_top };		 //左上
	vertexData[2].texcoord = { tex_right,tex_bottom };   //右下
	vertexData[3].texcoord = { tex_right,tex_top };	     //右上

	vertexData[0].normal = { 0.0f,0.0f,-1.0f };          //左下	
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };          //左上
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };          //右下
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };          //右上

	// Sprite用のWorldViewProjectionMatrixを作る
	Matrix4x4 worldMatrixSprite = Math::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrixSprite = Math::MakeIdentity4x4();
	Matrix4x4 projectionMatrixSprite = Math::MakeOrthographicMatrix(0.0f, float(WinApp::kClientWidth), 0.0f, float(WinApp::kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrixSprite = Math::Multiply(worldMatrixSprite, Math::Multiply(viewMatrixSprite, projectionMatrixSprite));
	transformationMatrixData->WVP = worldViewProjectionMatrixSprite;
	transformationMatrixData->World = worldMatrixSprite;
}
//描画
void Sprite::Draw() {
	if (!isVisible_) return;
	//VertexBufferViewを設定
	spriteCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	//IndexBufferViewを設定
	spriteCommon->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	//マテリアルCBufferの場所を設定
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	//座標変換行列CBufferの場所を設定
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(filePath));
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLightResource->GetGPUVirtualAddress());
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, spotLightResource->GetGPUVirtualAddress());
	//DrawCall(描画)
	spriteCommon->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
//頂点データ作成
void Sprite::VertexDataCreate() {
	//リソースを作る
	vertexResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * 4);
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

}
//index作成
void Sprite::IndexCreate() {
	//リソースを作る
	indexResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * 6);
	//リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	//IndexResourceにデータを書き込むためのアドレスを取得
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	//インデックスリソースにデータを書き込む
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;
}
//マテリアル作成
void Sprite::MaterialCreate() {
	//リソースを作る
	materialResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(Material));
	// 書き込むためのアドレスと取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//マテリアルデータの初期値を書き込む
	materialData->color = Vector4{ 1.0f,1.0f,1.0f,1.0f };
	//Lighting
	materialData->enableLighting = false;
	//UVTransform行列を単位行列で初期化
	materialData->uvTransform = Math::MakeIdentity4x4();
	materialData->shininess = 70;
}
//座標変換行列データ作成
void Sprite::TransformationCreate() {
	//リソースを作る
	transformationMatrixResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	//書き込むためのアドレスの取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	// 単位行列を書き込んでおく
	transformationMatrixData->WVP = Math::MakeIdentity4x4();
	transformationMatrixData->World = Math::MakeIdentity4x4();
}
void Sprite::CameraCreate()
{
	//リソースを作る
	cameraResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(CameraForGPU));
	//書き込むためのアドレスの取得
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));
	cameraData->worldPosition = { 0.0f, 0.0f, -5.0f };

}
void Sprite::DirectionalLightCreate()
{   directionalLightResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = Math::Normalize(directionalLightData->direction);
	directionalLightData->intensity = 1.0f;
}
void Sprite::PointLightCreate()
{
	pointLightResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(PointLight));
	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));
	pointLightData->position = { 0.0f, 1.0f, -1.0f }; // 点光源的位置
	pointLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 点光源的颜色
	pointLightData->intensity = 1.0f;
}
void Sprite::SpotLightCreate()
{
	spotLightResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(SpotLight));
	spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));
	spotLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	spotLightData->position = { 0.0f, 2.0f, 5.0f };
	spotLightData->distance = 7.0f;
	spotLightData->direction = Math::Normalize({ 0.0f, 0.0f, 1.0f });
	spotLightData->intensity = 4.0f;
	spotLightData->decay = 2.0f;
	spotLightData->cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
}
void Sprite::AdjustTextureSize()
{
	//テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(filePath);
	//テクスチャ切り出しサイズ
	textureSize_ = { static_cast<float>(metadata.width),static_cast<float>(metadata.height) };
	//画像サイズをテクスチャサイズに合わせる
	size = textureSize_;
}