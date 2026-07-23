#pragma once


namespace MyEngine {

/// <summary>
/// 3次元空間の座標、方向、移動量を表し、基本的なベクトル演算を提供する構造体。
/// </summary>
struct Vector3 final {
	float x;
	float y;
	float z;
	public:
	Vector3& operator+=(const Vector3& other) {
		this->x += other.x;
		this->y += other.y;
		this->z += other.z;
		return *this;
	}
	Vector3 operator+(const Vector3 other) const { return Vector3{x + other.x, y + other.y, z + other.z}; }
	Vector3 operator-(const Vector3 other) const { return Vector3{x - other.x, y - other.y, z - other.z}; }
	Vector3 operator*(const Vector3 other) const { return Vector3{x * other.x, y * other.y, z * other.z}; }
	Vector3 operator*(const float scalar) const { return Vector3{x * scalar, y * scalar, z * scalar}; }
};



} // namespace MyEngine
