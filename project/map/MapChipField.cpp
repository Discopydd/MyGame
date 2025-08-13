#include "MapChipField.h"
namespace {

std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType ::kBlank},
    {"1", MapChipType::kBlock },
};

}

void MapChipField::ResetMapChipData() {
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
	for (auto& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
	}
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	// リセット
	ResetMapChipData();
	// ファイル開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open()); // Error Check
	// ファイルの内容を保存する
	std::stringstream mapChipCsv;
	mapChipCsv << file.rdbuf();
	file.close();

	for (uint32_t i = 0; i < kNumBlockVirtical; i++) {
		std::string line;
		std::getline(mapChipCsv, line);
		std::istringstream line_stream(line);
		for (uint32_t j = 0; j < kNumBlockHorizontal; j++) {
			std::string word;
			std::getline(line_stream, word, ',');
			if (mapChipTable.contains(word)) {
				mapChipData_.data[i][j] = mapChipTable[word];
			}
		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex)const {
	// 範囲オーバーなら
	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex)
		return MapChipType::kBlank;
	if (yIndex < 0 || kNumBlockVirtical - 1 < yIndex)
		return MapChipType::kBlank;

	return mapChipData_.data[yIndex][xIndex];
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) const{ return Vector3(kBlockHeight * xIndex, kBlockWidth * (kNumBlockVirtical - 1 - yIndex), 0); }

MapChipField::IndexSet MapChipField::GetMapChipIndexByPosition(const Vector3& position)const
{
   IndexSet indexsSet{};
	 float adjustedX = position.x + (kBlockWidth / 2.0f);
    float adjustedY = position.y + (kBlockHeight / 2.0f);

    indexsSet.xIndex = uint32_t(adjustedX / kBlockWidth);
    indexsSet.yIndex = uint32_t(kNumBlockVirtical - adjustedY / kBlockHeight);
	return indexsSet;
}

MapChipField::Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex)const
{
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);
	Rect rect{};
	rect.left = center.x - kBlockWidth / 2;
	rect.right = center.x + kBlockWidth / 2;
	rect.bottom = center.y - kBlockHeight / 2;
	rect.top = center.y + kBlockHeight / 2;

	return rect;
}
