#pragma once


/// <summary>
/// 3次元ベクトル
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

