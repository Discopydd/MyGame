#pragma once
#include "../math/Vector4.h"
#include <cstdint>
#include "../math/Matrix4x4.h"
namespace MyEngine {

/// <summary>
/// Materialで使用する関連データをまとめて保持する構造体です。
/// </summary>
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
	float shininess;
};

} // namespace MyEngine
