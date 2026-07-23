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
namespace MyEngine {

class Object3dCommon;
/// <summary>
/// 3Dオブジェクトのモデル、ワールド変換、マテリアルを管理して描画するクラス。
/// </summary>
class Object3d {

public:

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="object3dCommon">3Dオブジェクト生成に使用する共通描画処理。</param>
    void Initialize(Object3dCommon* object3dCommon);
    // 更新
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    void Update();

    // 描画
    /// <summary>
    /// 現在の状態を画面へ描画します。
    /// </summary>
    void Draw();


    /// <summary>
    /// Modelを設定します。
    /// </summary>
    /// <param name="model">処理対象のオブジェクトへのポインタ。</param>
    void SetModel(Model* model) { model_ = model; }
    /// <summary>
    /// Modelを設定します。
    /// </summary>
    /// <param name="filepath">対象ファイルまたはリソースのパス。</param>
    void SetModel(const std::string& filepath);

    //transform
    /// <summary>
    /// Transformを設定します。
    /// </summary>
    /// <param name="transform">処理に使用する参照値。</param>
    void SetTransform(const Transform& transform) { this->transform = transform; }
    /// <summary>
    /// Transformを取得します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
    Transform GetTransform() { return transform; }

    //スケール
    /// <summary>
    /// Scaleを設定します。
    /// </summary>
    /// <param name="scale">処理に使用する参照値。</param>
    void SetScale(const Vector3& scale) { transform.scale = scale; }
    //回転
    /// <summary>
    /// Rotateを設定します。
    /// </summary>
    /// <param name="rotate">処理に使用する参照値。</param>
    void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
    //位置
    /// <summary>
    /// Translateを設定します。
    /// </summary>
    /// <param name="translate">処理に使用する参照値。</param>
    void SetTranslate(const Vector3& translate) { transform.translate = translate; }
    //setter
    /// <summary>
    /// Cameraを設定します。
    /// </summary>
    /// <param name="camera">描画および座標変換に使用するカメラ。</param>
    void SetCamera(Camera* camera) { this->camera = camera; };
    /// <summary>
    /// Directional Light Dataを取得します。
    /// </summary>
    /// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
    DirectionalLight* GetDirectionalLightData() const { return directionalLightData; }
    /// <summary>
    /// Modelを取得します。
    /// </summary>
    /// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
    Model* GetModel() const { return model_; }
    /// <summary>
    /// Point Light Dataを取得します。
    /// </summary>
    /// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
    PointLight* GetPointLightData() const { return pointLightData; }
    /// <summary>
    /// Spot Light Dataを取得します。
    /// </summary>
    /// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
    SpotLight* GetSpotLightData() const { return spotLightData; }
    /// <summary>
    /// Rotateを取得します。
    /// </summary>
    /// <returns>保持している値への参照。</returns>
    const Vector3& GetRotate() const { return transform.rotate; }
    /// <summary>
    /// Translateを取得します。
    /// </summary>
    /// <returns>保持している値への参照。</returns>
    const Vector3& GetTranslate() const { return transform.translate; }
    // 平行光源の強度を設定
    /// <summary>
    /// Directional Light Intensityを設定します。
    /// </summary>
    /// <param name="intensity">処理に使用するintensityの値。</param>
    void SetDirectionalLightIntensity(float intensity);
    // 点光源の強度を設定
    /// <summary>
    /// Point Light Intensityを設定します。
    /// </summary>
    /// <param name="intensity">処理に使用するintensityの値。</param>
    void SetPointLightIntensity(float intensity);
    // スポットライトの強度を設定
    /// <summary>
    /// Spot Light Intensityを設定します。
    /// </summary>
    /// <param name="intensity">処理に使用するintensityの値。</param>
    void SetSpotLightIntensity(float intensity);

    /// <summary>
    /// Colorを設定します。
    /// </summary>
    /// <param name="color">処理に使用する参照値。</param>
    void SetColor(const Vector4& color) {
        if (model_) {
            model_->SetColor(color);
        }
    }

    /// <summary>
    /// Colorを取得します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
    Vector4 GetColor() const {
        if (model_) {
            return model_->GetColor();
        }
        return {1.0f, 1.0f, 1.0f, 1.0f};
    }
    /// <summary>
    /// Enable Lightingを設定します。
    /// </summary>
    /// <param name="enable">機能を有効にする場合は true。</param>
    void SetEnableLighting(bool enable) {
        if (model_) {
            model_->SetEnableLighting(enable);
        }
    }
     /// <summary>
     /// Lighting Modeを設定します。
     /// </summary>
     /// <param name="mode">処理に使用するmodeの値。</param>
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

} // namespace MyEngine
