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
/// MapChipDataUnitで使用する関連データをまとめて保持する構造体です。
/// </summary>
struct MapChipDataUnit {
    MapChipType type = MapChipType::kBlank;
    uint8_t     subID = 0;              // 種別ごとのサブID (0〜9 想定)
};

/// <summary>
/// MapChipDataで使用する関連データをまとめて保持する構造体です。
/// </summary>
struct MapChipData {
    std::vector<std::vector<MapChipDataUnit>> data;
};

/// <summary>
/// MapChipFieldに関する処理と状態を管理するクラスです。
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

    /// <summary>
    /// Map Chip Dataを初期状態へ戻します。
    /// </summary>
    void ResetMapChipData();
    /// <summary>
    /// Map Chip Csvを読み込みます。
    /// </summary>
    /// <param name="filePath">読み込むファイルのパス。</param>
    void LoadMapChipCsv(const std::string& filePath);

    /// <summary>
    /// Map Chip Type By Indexを取得します。
    /// </summary>
    /// <param name="xIndex">マップ上のXインデックス。</param>
    /// <param name="yIndex">マップ上のYインデックス。</param>
    /// <returns>計算または取得した結果。</returns>
    MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) const;
    // ★ 追加: サブIDを取得
    /// <summary>
    /// Map Chip Sub ID By Indexを取得します。
    /// </summary>
    /// <param name="xIndex">マップ上のXインデックス。</param>
    /// <param name="yIndex">マップ上のYインデックス。</param>
    /// <returns>計算または取得した数値。</returns>
    uint8_t     GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex) const;

    /// <summary>
    /// Map Chip Position By Indexを取得します。
    /// </summary>
    /// <param name="xIndex">マップ上のXインデックス。</param>
    /// <param name="yIndex">マップ上のYインデックス。</param>
    /// <returns>計算または取得した結果。</returns>
    MyEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) const;

    /// <summary>
    /// IndexSetで使用する関連データをまとめて保持する構造体です。
    /// </summary>
    struct IndexSet {
        uint32_t xIndex;
        uint32_t yIndex;
    };
    /// <summary>
    /// Map Chip Index By Positionを取得します。
    /// </summary>
    /// <param name="position">対象のワールド座標。</param>
    /// <returns>計算または取得した結果。</returns>
    IndexSet GetMapChipIndexByPosition(const MyEngine::Vector3& position) const;

    /// <summary>
    /// Rectで使用する関連データをまとめて保持する構造体です。
    /// </summary>
    struct Rect {
        float left;
        float right;
        float bottom;
        float top;
    };
    /// <summary>
    /// Rect By Indexを取得します。
    /// </summary>
    /// <param name="xIndex">マップ上のXインデックス。</param>
    /// <param name="yIndex">マップ上のYインデックス。</param>
    /// <returns>計算または取得した結果。</returns>
    Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex) const;

    /// <summary>
    /// Map Min Positionを取得します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
    MyEngine::Vector3 GetMapMinPosition() const {
        return MyEngine::Vector3(0, 0, 0);
    }
    /// <summary>
    /// Map Max Positionを取得します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
    MyEngine::Vector3 GetMapMaxPosition() const {
        return MyEngine::Vector3(
            numBlockHorizontal_ * kBlockWidth,
            numBlockVertical_   * kBlockHeight,
            0.0f);
    }
};
