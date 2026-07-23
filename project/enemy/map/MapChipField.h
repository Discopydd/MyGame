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
/// <summary>
/// CSVから読み込んだ1マス分のマップチップ種類と補助情報を保持する構造体。
/// </summary>
struct MapChipDataUnit {
    MapChipType type = MapChipType::kBlank;
    uint8_t     subID = 0;              // 種別ごとのサブID (0〜9 想定)
};

/// <summary>
/// マップ全体のマス情報を二次元配列として保持する構造体。
/// </summary>
struct MapChipData {
    std::vector<std::vector<MapChipDataUnit>> data;
};

/// <summary>
/// CSVマップの読み込み、チップ位置変換、当たり判定用範囲取得を行うクラス。
/// </summary>
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

    MyEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) const;

    /// <summary>
    /// マップチップの横方向と縦方向のインデックスをまとめて保持する構造体。
    /// </summary>
    struct IndexSet {
        uint32_t xIndex;
        uint32_t yIndex;
    };
    IndexSet GetMapChipIndexByPosition(const MyEngine::Vector3& position) const;

    /// <summary>
    /// マップ上の矩形範囲を左、右、下、上の境界値で表す構造体。
    /// </summary>
    struct Rect {
        float left;
        float right;
        float bottom;
        float top;
    };
    Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex) const;

    MyEngine::Vector3 GetMapMinPosition() const {
        return MyEngine::Vector3(0, 0, 0);
    }
    MyEngine::Vector3 GetMapMaxPosition() const {
        return MyEngine::Vector3(
            numBlockHorizontal_ * kBlockWidth,
            numBlockVertical_   * kBlockHeight,
            0.0f);
    }
};
