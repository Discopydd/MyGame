#pragma once
#include "string"

namespace MyEngine {

/// <summary>
/// MaterialDataで使用する関連データをまとめて保持する構造体です。
/// </summary>
struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex = 0;
};

} // namespace MyEngine
