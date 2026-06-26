#pragma once
#include "../2d/Sprite.h"
#include "MaterialData.h"
#include <vector>

namespace MyEngine {

struct ModelData {
	std::vector<VertexData>vertices;
	MaterialData material;
};

} // namespace MyEngine
