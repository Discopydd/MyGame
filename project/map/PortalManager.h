#pragma once
#include <vector>
#include <string>
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Camera.h"
#include <map/MapChipField.h>
#include "MyMath.h"
#include <memory>
// もともと GameScene にあった PortalInfo をここへ移動
struct PortalInfo {
    MapChipField::IndexSet index;  // 転送ポータルのマス座標
    std::string targetMap;         // 遷移先マップのパス
    Vector3 targetStartPos;        // 遷移先マップでのプレイヤー開始位置
};

// GameScene.cpp にある補助関数の宣言（実装は GameScene.cpp 側）
Vector3 WorldToScreen(const Vector3& worldPos, Camera* camera);

class PortalManager {
public:
    PortalManager() = default;
    ~PortalManager() = default;

    void Initialize(SpriteCommon* spriteCommon, Camera* camera);
    void Finalize();

    void ClearPortals();
    void AddPortal(const MapChipField::IndexSet& idx,
                   const std::string& targetMap,
                   const Vector3& startPos);

    // 現在登録されている全ポータルを返す（GameScene 側で配置に使用）
    const std::vector<PortalInfo>& GetPortals() const { return portals_; }

    // プレイヤーがいずれかのポータル上にいる場合、そのポータルへのポインタを返す。なければ nullptr
    const PortalInfo* GetPortalAt(const MapChipField::IndexSet& playerIndex) const;

    // 「E を押す」ヒントアイコンを更新（表示有無 + 位置）
    void UpdateHint(const MapChipField::IndexSet& playerIndex,
                    const Vector3& playerWorldPos,
                    bool canControl);

    // 描画ヒントアイコン（GameScene::Draw 内呼び出す）
    void DrawHint();

private:
    SpriteCommon* spriteCommon_ = nullptr;
    Camera*       camera_       = nullptr;

    std::unique_ptr<Sprite> hintSprite_;  // 「E を押す」アイコン
    std::vector<PortalInfo> portals_;
};
