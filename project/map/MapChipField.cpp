#include "MapChipField.h"
namespace {

std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType ::kBlank},
    {"1", MapChipType::kBlock },
    {"2", MapChipType::kPortal },
    {"3", MapChipType::kItem},
    {"4", MapChipType::kSpike},
};

}

void MapChipField::ResetMapChipData() {
	mapChipData_.data.clear();
	numBlockVertical_ = 0;
    numBlockHorizontal_ = 0;
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
    ResetMapChipData();
   std::ifstream file(filePath);
    assert(file.is_open());

    std::string line;
    std::vector<std::vector<MapChipType>> tempData;

       while (std::getline(file, line)) {
        if (line.empty()) continue; // 跳过空行
        std::istringstream line_stream(line);
        std::vector<MapChipType> row;
        std::string word;
        while (std::getline(line_stream, word, ',')) {
            if (mapChipTable.contains(word)) {
                row.push_back(mapChipTable[word]);
            } else {
                row.push_back(MapChipType::kBlank); // 兜底：未知字符 = 空白
            }
        }
        if (!row.empty()) {
            tempData.push_back(row);
        }
    }
    file.close();

    if (tempData.empty()) {
        numBlockVertical_ = 0;
        numBlockHorizontal_ = 0;
        return; // 空地图
    }

    // 统一每行的长度（避免列数不一致）
    numBlockHorizontal_ = 0;
    for (auto& row : tempData) {
        if (row.size() > numBlockHorizontal_) {
            numBlockHorizontal_ = static_cast<uint32_t>(row.size());
        }
    }
    numBlockVertical_ = static_cast<uint32_t>(tempData.size());

    // 重新分配并填充
    mapChipData_.data.resize(numBlockVertical_);
    for (uint32_t i = 0; i < numBlockVertical_; i++) {
        uint32_t reversedIndex = numBlockVertical_ - 1 - i;
        mapChipData_.data[reversedIndex].resize(numBlockHorizontal_, MapChipType::kBlank);
        for (uint32_t j = 0; j < tempData[i].size(); j++) {
            mapChipData_.data[reversedIndex][j] = tempData[i][j];
        }
    }
}
MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex)const {
	if (xIndex >= numBlockHorizontal_  || yIndex >= numBlockVertical_) {
        return MapChipType::kBlank;
    }
    return mapChipData_.data[yIndex][xIndex];
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) const{  return Vector3(
        kBlockWidth * xIndex + kBlockWidth / 2.0f, // X 位置
        kBlockHeight * yIndex + kBlockHeight / 2.0f, // Y 位置
        0.0f
    ); }

MapChipField::IndexSet MapChipField::GetMapChipIndexByPosition(const Vector3& position) const {
    IndexSet idx{};

    // 以格子左下为 0 开始的网格索引：先 floor，再夹到范围内
    int xi = static_cast<int>(std::floor(position.x / kBlockWidth));
    int yi = static_cast<int>(std::floor(position.y / kBlockHeight));

    if (numBlockHorizontal_ == 0 || numBlockVertical_ == 0) {
        idx.xIndex = 0; idx.yIndex = 0; 
        return idx; // 空地图保护
    }

    xi = (std::max)(0, (std::min)(xi, static_cast<int>(numBlockHorizontal_ - 1)));
    yi = (std::max)(0, (std::min)(yi, static_cast<int>(numBlockVertical_  - 1)));

    idx.xIndex = static_cast<uint32_t>(xi);
    idx.yIndex = static_cast<uint32_t>(yi);
    return idx;
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
