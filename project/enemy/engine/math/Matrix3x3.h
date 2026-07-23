#pragma once
namespace MyEngine {

/// <summary>
/// 2次元変換などに使用する3行3列の行列要素を保持する構造体。
/// </summary>
struct Matrix3x3 final {
	float m[3][3];
};

} // namespace MyEngine
