#include "MapChipField.h"
namespace {

std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType ::kBlank},
    {"1", MapChipType::kBlock },
    {"2", MapChipType::kPortal },
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
    ResetMapChipData();
    std::ifstream file;
    file.open(filePath);
    assert(file.is_open());

    std::stringstream mapChipCsv;
    mapChipCsv << file.rdbuf();
    file.close();

    std::vector<std::vector<MapChipType>> tempData;
    tempData.resize(kNumBlockVirtical);

    for (uint32_t i = 0; i < kNumBlockVirtical; i++) {
        std::string line;
        std::getline(mapChipCsv, line);
        std::istringstream line_stream(line);
        tempData[i].resize(kNumBlockHorizontal);

        for (uint32_t j = 0; j < kNumBlockHorizontal; j++) {
            std::string word;
            std::getline(line_stream, word, ',');
            if (mapChipTable.contains(word)) {
                tempData[i][j] = mapChipTable[word];
            }
        }
    }

    for (uint32_t i = 0; i < kNumBlockVirtical; i++) {
        uint32_t reversedIndex = kNumBlockVirtical - 1 - i;
        mapChipData_.data[reversedIndex] = tempData[i];
    }
}
MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex)const {
	if (xIndex >= kNumBlockHorizontal || yIndex >= kNumBlockVirtical) {
        return MapChipType::kBlank;
    }
    return mapChipData_.data[yIndex][xIndex];
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) const{  return Vector3(
        kBlockWidth * xIndex + kBlockWidth / 2.0f, // X 位置
        kBlockHeight * yIndex + kBlockHeight / 2.0f, // Y 位置
        0.0f
    ); }

MapChipField::IndexSet MapChipField::GetMapChipIndexByPosition(const Vector3& position)const
{
   IndexSet indexSet{};
    indexSet.xIndex = static_cast<uint32_t>(position.x / kBlockWidth);
    indexSet.yIndex = static_cast<uint32_t>(position.y / kBlockHeight);
    return indexSet;
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
