#include"Object3d.h"
#include "../math/MyMath.h"
#include "ModelManager.h"
#include <numbers>

void Object3d::Initialize(Object3dCommon* object3dCommon)
{
	this->object3dCommon_ = object3dCommon;
	//カメラ用のTransformを作る
	this->camera = object3dCommon_->GetDefaultCamera();

	#pragma region ModelTransform
	//ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource = object3dCommon_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	//単位行列を書き込む
	transformationMatrixData->WVP = Math::MakeIdentity4x4();
	transformationMatrixData->World = Math::MakeIdentity4x4();
#pragma endregion
	
	//リソースを作る
	cameraResource = object3dCommon_->GetDxCommon()->CreateBufferResource(sizeof(CameraForGPU));
	//書き込むためのアドレスの取得
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));
	cameraData->worldPosition = { 0.0f, 0.0f, -5.0f };
#pragma region 平行光源
	//平行光源用のResourceを作成
	directionalLightResource = object3dCommon_->GetDxCommon()->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = Math::Normalize(directionalLightData->direction);
	directionalLightData->intensity = 1.0f;
#pragma endregion
	pointLightResource = object3dCommon_->GetDxCommon()->CreateBufferResource(sizeof(PointLight));
	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));
	pointLightData->position = { 0.0f, 1.0f, -1.0f }; // 点光源的位置
	pointLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 点光源的颜色
	pointLightData->intensity = 1.0f;

	spotLightResource = object3dCommon_->GetDxCommon()->CreateBufferResource(sizeof(SpotLight));
	spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));
	spotLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	spotLightData->position = { 0.0f, 2.0f, 5.0f };
	spotLightData->distance = 7.0f;
	spotLightData->direction = Math::Normalize({ 0.0f, 0.0f, 1.0f });
	spotLightData->intensity = 4.0f;
	spotLightData->decay = 2.0f;
	spotLightData->cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	//カメラとモデルのTransform変数
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{1.0f,0.0f,0.0f} };

	
}

void Object3d::Update()
{

	worldMatrix = Math::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	if (camera) {
		const Matrix4x4& viewProjectionMatrix = camera->GetViewprojectionMatrix();
		worldViewProjectionMatrix = worldMatrix * viewProjectionMatrix;
		cameraData->worldPosition = camera->GetTransform().translate;
	}
	else {
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;
}

void Object3d::Draw()
{
	object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	//平行光源Cbufferの場所を設定
	object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
	object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLightResource->GetGPUVirtualAddress());
	object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, spotLightResource->GetGPUVirtualAddress());
	if (model_) {
		model_->Draw();
	}
}

void Object3d::SetModel(const std::string& filepath)
{
	model_ = ModelManager::GetInstants()->FindModel(filepath);
}

void Object3d::SetDirectionalLightIntensity(float intensity)
{
    if (!directionalLightData) { return; }
    directionalLightData->intensity = intensity;
}

void Object3d::SetPointLightIntensity(float intensity)
{
    if (!pointLightData) { return; }
    pointLightData->intensity = intensity;
}

void Object3d::SetSpotLightIntensity(float intensity)
{
    if (!spotLightData) { return; }
    spotLightData->intensity = intensity;
}
