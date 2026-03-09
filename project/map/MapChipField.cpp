#include "MapChipField.h"
#include <algorithm>
#include <cmath>

namespace {

enum MapChipCharIndex {
    kChipType = 0,  // 先頭1文字: 種別
    kChipSubID = 1, // 2文字目: サブID
};

// 新フォーマット: 先頭1文字で種別を判定
const std::map<char, MapChipType> kTypeTable = {
    // 数値も一応対応させておく
    { '0', MapChipType::kBlank },
    { '1', MapChipType::kBlock },
    { '2', MapChipType::kPortal },
    { '3', MapChipType::kItem },
    { '4', MapChipType::kSpike },
    { '5', MapChipType::kMoveHorizontal },
    { '6', MapChipType::kMoveVertical },
    { '7', MapChipType::kWater },

    { 'B', MapChipType::kBlock },
    { 'b', MapChipType::kBlock2 },
    { 'P', MapChipType::kPortal },
    { 'I', MapChipType::kItem },
    { 'S', MapChipType::kSpike },
    { 'H', MapChipType::kMoveHorizontal },
    { 'V', MapChipType::kMoveVertical },
    { 'W', MapChipType::kWater },

    { '@', MapChipType::kPlayer },
    { 'E', MapChipType::kEnemy },
};

// 旧フォーマット（"0","1","10"...）用: 互換性のため残す
const std::map<std::string, MapChipType> kLegacyTable = {
    { "0",  MapChipType::kBlank },
    { "1",  MapChipType::kBlock },
    { "10", MapChipType::kBlock2 },
    { "2",  MapChipType::kPortal },
    { "3",  MapChipType::kItem },
    { "4",  MapChipType::kSpike },
    { "5",  MapChipType::kMoveHorizontal },
    { "6",  MapChipType::kMoveVertical },
    { "7",  MapChipType::kWater },
};

MapChipDataUnit MakeUnit(MapChipType type = MapChipType::kBlank, uint8_t subID = 0)
{
    MapChipDataUnit u{};
    u.type  = type;
    u.subID = subID;
    return u;
}

} // namespace

void MapChipField::ResetMapChipData()
{
    mapChipData_.data.clear();
    numBlockVertical_   = 0;
    numBlockHorizontal_ = 0;
}

void MapChipField::LoadMapChipCsv(const std::string& filePath)
{
    ResetMapChipData();

    std::ifstream file(filePath);
    assert(file.is_open());

    std::string line;
    std::vector<std::vector<MapChipDataUnit>> tempData;

    while (std::getline(file, line)) {
        if (line.empty()) { continue; }

        std::istringstream lineStream(line);
        std::vector<MapChipDataUnit> row;
        std::string word;

        while (std::getline(lineStream, word, ',')) {
            MapChipDataUnit unit = MakeUnit();

            if (!word.empty()) {
                // 1) 旧フォーマット（"0","1","10"...）
                auto itLegacy = kLegacyTable.find(word);
                if (itLegacy != kLegacyTable.end()) {
                    unit.type = itLegacy->second;
                } else {
                    // 2) 新フォーマット（先頭1文字＋サブID）
                    char key = word[MapChipCharIndex::kChipType];
                    auto it = kTypeTable.find(key);
                    if (it != kTypeTable.end()) {
                        unit.type = it->second;

                        if (word.size() > static_cast<size_t>(MapChipCharIndex::kChipSubID)) {
                            char sub = word[MapChipCharIndex::kChipSubID];
                            if (sub >= '0' && sub <= '9') {
                                unit.subID = static_cast<uint8_t>(sub - '0');
                            }
                        }
                    }
                }
            }

            row.push_back(unit);
        }

        if (!row.empty()) {
            tempData.push_back(std::move(row));
        }
    }

    file.close();

    if (tempData.empty()) {
        numBlockVertical_   = 0;
        numBlockHorizontal_ = 0;
        return;
    }

    // 行ごとの最大列数を求める
    numBlockHorizontal_ = 0;
    for (const auto& row : tempData) {
        if (row.size() > numBlockHorizontal_) {
            numBlockHorizontal_ = static_cast<uint32_t>(row.size());
        }
    }
    numBlockVertical_ = static_cast<uint32_t>(tempData.size());

    MapChipDataUnit def = MakeUnit();
    mapChipData_.data.assign(
        numBlockVertical_,
        std::vector<MapChipDataUnit>(numBlockHorizontal_, def));

    // y 方向を反転して格納（元コードと同じ）
    for (uint32_t i = 0; i < numBlockVertical_; ++i) {
        uint32_t rev = numBlockVertical_ - 1 - i;
        for (uint32_t j = 0; j < tempData[i].size(); ++j) {
            mapChipData_.data[rev][j] = tempData[i][j];
        }
    }
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) const
{
    if (xIndex >= numBlockHorizontal_ || yIndex >= numBlockVertical_) {
        return MapChipType::kBlank;
    }
    return mapChipData_.data[yIndex][xIndex].type;
}

uint8_t MapChipField::GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex) const
{
    if (xIndex >= numBlockHorizontal_ || yIndex >= numBlockVertical_) {
        return 0;
    }
    return mapChipData_.data[yIndex][xIndex].subID;
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) const
{
    return Vector3(
        kBlockWidth * xIndex + kBlockWidth / 2.0f,
        kBlockHeight * yIndex + kBlockHeight / 2.0f,
        0.0f);
}

MapChipField::IndexSet MapChipField::GetMapChipIndexByPosition(const Vector3& position) const
{
    IndexSet idx{};

    int xi = static_cast<int>(std::floor(position.x / kBlockWidth));
    int yi = static_cast<int>(std::floor(position.y / kBlockHeight));

    if (numBlockHorizontal_ == 0 || numBlockVertical_ == 0) {
        idx.xIndex = 0;
        idx.yIndex = 0;
        return idx;
    }

    xi = (std::max)(0, (std::min)(xi, static_cast<int>(numBlockHorizontal_ - 1)));
    yi = (std::max)(0, (std::min)(yi, static_cast<int>(numBlockVertical_  - 1)));

    idx.xIndex = static_cast<uint32_t>(xi);
    idx.yIndex = static_cast<uint32_t>(yi);
    return idx;
}

MapChipField::Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) const
{
    Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);
    Rect rect{};
    rect.left   = center.x - kBlockWidth / 2;
    rect.right  = center.x + kBlockWidth / 2;
    rect.bottom = center.y - kBlockHeight / 2;
    rect.top    = center.y + kBlockHeight / 2;
    return rect;
}
