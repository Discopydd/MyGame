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
	/// <summary>
	/// 演算子「+」による計算結果を生成します。
	/// </summary>
	/// <param name="other">処理に使用するotherの値。</param>
	/// <returns>計算または取得した結果。</returns>
	Vector3 operator+(const Vector3 other) const { return Vector3{x + other.x, y + other.y, z + other.z}; }
	/// <summary>
	/// 演算子「-」による計算結果を生成します。
	/// </summary>
	/// <param name="other">処理に使用するotherの値。</param>
	/// <returns>計算または取得した結果。</returns>
	Vector3 operator-(const Vector3 other) const { return Vector3{x - other.x, y - other.y, z - other.z}; }
	/// <summary>
	/// 演算子「*」による計算結果を生成します。
	/// </summary>
	/// <param name="other">処理に使用するotherの値。</param>
	/// <returns>計算または取得した結果。</returns>
	Vector3 operator*(const Vector3 other) const { return Vector3{x * other.x, y * other.y, z * other.z}; }
	/// <summary>
	/// 演算子「*」による計算結果を生成します。
	/// </summary>
	/// <param name="scalar">乗算または除算に使用するスカラー値。</param>
	/// <returns>計算または取得した結果。</returns>
	Vector3 operator*(const float scalar) const { return Vector3{x * scalar, y * scalar, z * scalar}; }
};



} // namespace MyEngine
