#pragma once
#include "string"

namespace MyEngine {

/// <summary>
/// モデルが参照するテクスチャファイルなどのマテリアル情報を保持する構造体。
/// </summary>
struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex = 0;
};

} // namespace MyEngine
