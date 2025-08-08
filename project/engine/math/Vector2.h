#pragma once
/// <summary>
/// 2次元ベクトル
/// </summary>
struct Vector2 final {
	float x;
	float y;

	Vector2& operator+=(const Vector2& other) {
		this->x += other.x;
		this->y += other.y;
		return *this;
	}

	// スカラー加算 (float 型)
	Vector2& operator+=(float scalar) {
		this->x += scalar;
		this->y += scalar;
		return *this;
	}
};