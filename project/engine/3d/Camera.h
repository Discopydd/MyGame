#pragma once
#include <WinApp.h>
#include <Matrix4x4.h>
//カメラ
#include <Transform.h>
class Camera {
public:
	Camera();

	// 更新
	void Update();

	void SetRotate(const Vector3& rotate) { this->transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { this->transform.translate = translate; }
	void SetFovY(const float& fovy) { this->fovY = fovy; }
	void SetAspectRatio(const float& aspectRation) { this->aspectRatio = aspectRation; }
	void SetNearClip(const float& nearClip) { this->nearClip = nearClip; }
	void SetFarClip(const float& farClip) { this->farClip = farClip; }

	const Matrix4x4& GetWorldMatrix()const { return worldMatrix; }
	const Matrix4x4& GetViewMatrix()const { return viewMatrix; }
	const Matrix4x4& GetProjectionMatrix()const { return projectionMatrix; }
	const Matrix4x4& GetViewprojectionMatrix()const { return viewProjectionMatrix; }
	const Transform& GetTransform()const { return transform; }
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