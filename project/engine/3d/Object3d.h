#pragma once
#include "Object3dCommon.h"
#include "../math/Matrix4x4.h"
#include "../math/Vector4.h"
#include "../math/Vector3.h"
#include "../math/Vector2.h"
#include"DirectionalLight.h"
#include"TransformationMatrix.h"
#include"../math/Transform.h"
#include "Model.h"
#include"Camera.h"
class Object3dCommon;
class Object3d {

public:

	void Initialize(Object3dCommon* object3dCommon);
	// 更新
	void Update();

	// 描画
	void Draw();


	void SetModel(Model* model) { model_ = model; }
	void SetModel(const std::string& filepath);

	//transform
	void SetTransform(const Transform& transform) { this->transform = transform; }
	Transform GetTransform() { return transform; }

	//スケール
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	//回転
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	//位置
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }
	//setter
	void SetCamera(Camera* camera) { this->camera = camera; };
    DirectionalLight* GetDirectionalLightData() const { return directionalLightData; }
	Model* GetModel() const { return model_; }
	PointLight* GetPointLightData() const { return pointLightData; }
    SpotLight* GetSpotLightData() const { return spotLightData; }
	const Vector3& GetRotate() const { return transform.rotate; }

	// 设置平行光强度
    void SetDirectionalLightIntensity(float intensity);
    // 设置点光源强度
    void SetPointLightIntensity(float intensity);
    // 设置聚光灯强度
    void SetSpotLightIntensity(float intensity);

	void SetColor(const Vector4& color) {
        if (model_) {
            model_->SetColor(color);
        }
    }

    Vector4 GetColor() const {
        if (model_) {
            return model_->GetColor();
        }
        return {1.0f, 1.0f, 1.0f, 1.0f};
    }
	void SetEnableLighting(bool enable) {
		if (model_) {
			model_->SetEnableLighting(enable);
		}
	}
	 void SetLightingMode(int mode);
private:

	Object3dCommon* object3dCommon_ = nullptr;
	// モデル
	Model* model_ = nullptr;

	// ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	// データを書き込む
	TransformationMatrix* transformationMatrixData = nullptr;

	// 平行光源用のResourceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	DirectionalLight* directionalLightData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;
	CameraForGPU* cameraData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource;
	PointLight* pointLightData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource;
	SpotLight* spotLightData = nullptr;
	// SRT
	Transform transform;
    Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;

	Camera* camera = nullptr;
};