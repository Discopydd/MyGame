#pragma once
namespace MyEngine {

/// <summary>
/// 2次元空間の座標や方向を表す2成分ベクトル構造体。
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

} // namespace MyEngine
