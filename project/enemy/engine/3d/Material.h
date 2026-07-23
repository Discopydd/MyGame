#pragma once
#include "../math/Vector4.h"
#include <cstdint>
#include "../math/Matrix4x4.h"
namespace MyEngine {

/// <summary>
/// 3Dモデル描画に使用する色、ライティング設定、UV変換行列を保持する構造体。
/// </summary>
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
	float shininess;
};

} // namespace MyEngine
