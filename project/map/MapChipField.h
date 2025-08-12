#pragma once
#include "Vector3.h"
#include <cassert>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

class MapChipField {

	MapChipData mapChipData_;

public:
	// １マスのサイズ
	static inline const float kBlockWidth = 2;
	static inline const float kBlockHeight = 2;
	// 縦横幅
	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	void ResetMapChipData();
	void LoadMapChipCsv(const std::string& filePath);
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);
};