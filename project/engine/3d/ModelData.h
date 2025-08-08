#pragma once
#include "../2d/Sprite.h"
#include "MaterialData.h"
#include <vector>

struct ModelData {
	std::vector<VertexData>vertices;
	MaterialData material;
};