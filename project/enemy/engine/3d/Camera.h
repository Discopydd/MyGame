#pragma once
#include <WinApp.h>
#include <Matrix4x4.h>
#include <Transform.h>
namespace MyEngine {

//カメラ
/// <summary>
/// 3D空間の視点、射影行列、ビュー行列を管理するカメラクラス。
/// </summary>
class Camera {
public:
	/// <summary>
	/// Cameraのインスタンスを生成します。
	/// </summary>
	Camera();

	// 更新
	/// <summary>
	/// 入力や経過時間に応じて、状態を1フレーム分更新します。
	/// </summary>
	void Update();

	/// <summary>
	/// Rotateを設定します。
	/// </summary>
	/// <param name="rotate">処理に使用する参照値。</param>
	void SetRotate(const Vector3& rotate) { this->transform.rotate = rotate; }
	/// <summary>
	/// Translateを設定します。
	/// </summary>
	/// <param name="translate">処理に使用する参照値。</param>
	void SetTranslate(const Vector3& translate) { this->transform.translate = translate; }
	/// <summary>
	/// Fov Yを設定します。
	/// </summary>
	/// <param name="fovy">処理に使用する参照値。</param>
	void SetFovY(const float& fovy) { this->fovY = fovy; }
	/// <summary>
	/// Aspect Ratioを設定します。
	/// </summary>
	/// <param name="aspectRation">処理に使用する参照値。</param>
	void SetAspectRatio(const float& aspectRation) { this->aspectRatio = aspectRation; }
	/// <summary>
	/// Near Clipを設定します。
	/// </summary>
	/// <param name="nearClip">処理に使用する参照値。</param>
	void SetNearClip(const float& nearClip) { this->nearClip = nearClip; }
	/// <summary>
	/// Far Clipを設定します。
	/// </summary>
	/// <param name="farClip">処理に使用する参照値。</param>
	void SetFarClip(const float& farClip) { this->farClip = farClip; }

	/// <summary>
	/// World Matrixを取得します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	const Matrix4x4& GetWorldMatrix()const { return worldMatrix; }
	/// <summary>
	/// View Matrixを取得します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	const Matrix4x4& GetViewMatrix()const { return viewMatrix; }
	/// <summary>
	/// Projection Matrixを取得します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	const Matrix4x4& GetProjectionMatrix()const { return projectionMatrix; }
	/// <summary>
	/// Viewprojection Matrixを取得します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	const Matrix4x4& GetViewprojectionMatrix()const { return viewProjectionMatrix; }
	/// <summary>
	/// Transformを取得します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	const Transform& GetTransform()const { return transform; }

	/// <summary>
	/// Fov Yを取得します。
	/// </summary>
	/// <returns>計算または取得した数値。</returns>
	float GetFovY() const { return fovY; }
    /// <summary>
    /// Aspect Ratioを取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    float GetAspectRatio() const { return aspectRatio; }
private:
	Transform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;
	Matrix4x4 projectionMatrix;
	float fovY = 0.45f;
	float aspectRatio = float(WinApp::kClientWidth) / float(WinApp::kClientHeight);
	float nearClip = 0.1f;
	float farClip = 100.0f;
	Matrix4x4 viewProjectionMatrix;
};

} // namespace MyEngine
