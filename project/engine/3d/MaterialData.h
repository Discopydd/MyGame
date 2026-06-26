#pragma once
#include "string"

namespace MyEngine {

struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex = 0;
};

} // namespace MyEngine
