#include"Camera.h"
#include"MyMath.h"

Camera::Camera() : transform({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} }),fovY(0.45f),aspectRatio(float(WinApp::kClientWidth) / float(WinApp::kClientHeight)),nearClip(0.1f),farClip(100.0f),worldMatrix(Math::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate)),viewMatrix(Math::Inverse(worldMatrix)),projectionMatrix(Math::MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip)),viewProjectionMatrix(Math::Multiply(viewMatrix, projectionMatrix))
{}

void Camera::Update()
{
	 worldMatrix = Math::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	 viewMatrix = Math::Inverse(worldMatrix);
	 projectionMatrix = Math::MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	 viewProjectionMatrix = Math::Multiply(viewMatrix, projectionMatrix);
}
