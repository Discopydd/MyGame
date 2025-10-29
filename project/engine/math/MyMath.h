#pragma once
#include "Matrix4x4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <cassert>
#include <Quaternion.h>
namespace Math {
	const float PI = 3.141592654f;

	Matrix4x4 MakeScaleMatrix(const Vector3& scale);

	Matrix4x4 MakeRotateZMatrix(float radian);

	Matrix4x4 MakeRotateXMatrix(float radian);

	Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

	Matrix4x4 MakeRotateYMatrix(float radian);

	Matrix4x4 Multiply(const Matrix4x4& matrix1, const Matrix4x4& matrix2);

	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotation, const Vector3& translation);

	Matrix4x4 Inverse(const Matrix4x4& m);

	Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

	Matrix4x4 MakeOrthographicMatrix(float left, float right, float top, float bottom, float nearClip, float farClip);

	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

	Matrix4x4 MakeIdentity4x4();

	float Length(const Vector3& vec);

	Vector3 Normalize(const Vector3& vec);

	Vector3 Add(const Vector3& v1, const Vector3& v2);

	Vector3 Subtract(const Vector3& v1, const Vector3& v2);
	Vector3 Multiply(const Vector3& vec, float scalar);
	Vector3 TransformCoordLocal(const Vector3& v, const Matrix4x4& m);

	float   Lerp(float a, float b, float t);
	Vector3 Lerp(const Vector3& a, const Vector3& b, float t);
	Vector4 Lerp(const Vector4& a, const Vector4& b, float t);
	// 角度(度) -> 弧度(rad)
	float ToRadian(float degrees);

	// 由轴角(弧度)构造四元数；返回值为 Vector4(quat.x, quat.y, quat.z, quat.w)
	Quaternion MakeAxisAngleQuaternion(const Vector3& axis, float angleRad);

	// 线性插值（Nlerp）：默认走最短弧，并做单位化
	Quaternion Lerp(const Quaternion& a, const Quaternion& b, float t, bool shortestPath = true);

	// 球面线性插值（Slerp）：默认走最短弧，t∈[0,1]
	Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t, float eps = 1e-6f);

	Vector3 QuaternionToEuler(const Quaternion& q);

	// （可选）度数版
	Vector3 QuaternionToEulerDeg(const Quaternion& q);
}