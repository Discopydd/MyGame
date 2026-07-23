#pragma once
#include "../2d/Sprite.h"
#include "MaterialData.h"
#include <vector>

namespace MyEngine {

/// <summary>
/// ModelDataで使用する関連データをまとめて保持する構造体です。
/// </summary>
struct ModelData {
	std::vector<VertexData>vertices;
	MaterialData material;
};

} // namespace MyEngine
