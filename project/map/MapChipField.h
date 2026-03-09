#pragma once
#include "Vector3.h"
#include <cassert>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>

enum class MapChipType {
    kBlank,          // 空白
    kBlock,          // ブロック
    kBlock2,
    kPortal,
    kItem,
    kSpike,
    kWater,
    kMoveHorizontal, // 左右往復
    kMoveVertical,   // 上下往復

    // 以降、マップ上にプレイヤー / 敵を配置する際に使用
    kPlayer,
    kEnemy,
};

// 1マス分のデータ
struct MapChipDataUnit {
    MapChipType type = MapChipType::kBlank;
    uint8_t     subID = 0;              // 種別ごとのサブID (0〜9 想定)
};

struct MapChipData {
    std::vector<std::vector<MapChipDataUnit>> data;
};

class MapChipField {

    MapChipData mapChipData_;

public:
    // １マスのサイズ
    static inline const float kBlockWidth  = 2;
    static inline const float kBlockHeight = 2;
    // 縦横幅
    uint32_t numBlockVertical_   = 0;
    uint32_t numBlockHorizontal_ = 0;

    void ResetMapChipData();
    void LoadMapChipCsv(const std::string& filePath);

    MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) const;
    // ★ 追加: サブIDを取得
    uint8_t     GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex) const;

    Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) const;

    struct IndexSet {
        uint32_t xIndex;
        uint32_t yIndex;
    };
    IndexSet GetMapChipIndexByPosition(const Vector3& position) const;

    struct Rect {
        float left;
        float right;
        float bottom;
        float top;
    };
    Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex) const;

    Vector3 GetMapMinPosition() const {
        return Vector3(0, 0, 0);
    }
    Vector3 GetMapMaxPosition() const {
        return Vector3(
            numBlockHorizontal_ * kBlockWidth,
            numBlockVertical_   * kBlockHeight,
            0.0f);
    }
};
