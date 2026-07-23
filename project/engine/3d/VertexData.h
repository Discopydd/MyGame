#pragma once
#include "Vector4.h"
#include "Vector2.h"
#include "Vector3.h"

namespace MyEngine {

/// <summary>
/// 3Dモデルの頂点位置、テクスチャ座標、法線情報を保持する構造体です。
/// </summary>
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

} // namespace MyEngine
