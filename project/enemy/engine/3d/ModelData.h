#pragma once
#include "../2d/Sprite.h"
#include "MaterialData.h"
#include <vector>

namespace MyEngine {

/// <summary>
/// 頂点情報、マテリアル情報など、読み込んだモデル全体のデータを保持する構造体。
/// </summary>
struct ModelData {
	std::vector<VertexData>vertices;
	MaterialData material;
};

} // namespace MyEngine
