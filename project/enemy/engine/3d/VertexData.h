#pragma once
#include "Vector4.h"
#include "Vector2.h"
#include "Vector3.h"

namespace MyEngine {

/// <summary>
/// 3D描画に使用する頂点の座標、テクスチャ座標、法線をまとめて保持する構造体。
/// </summary>
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

} // namespace MyEngine
