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
    MapChipField::IndexSet index;      // 転送ポータルのマス座標
    std::string targetMap;             // 遷移先マップのパス
    MyEngine::Vector3 targetStartPos;  // 遷移先マップでのプレイヤー開始位置
    MyEngine::Vector3 worldPos;        // 現在マップ上のポータル表示位置
};

// GameScene.cpp にある補助関数の宣言（実装は GameScene.cpp 側）
MyEngine::Vector3 WorldToScreen(const MyEngine::Vector3& worldPos, MyEngine::Camera* camera);

class PortalManager {
public:
    PortalManager() = default;
    ~PortalManager() = default;

    void Initialize(MyEngine::SpriteCommon* spriteCommon, MyEngine::Camera* camera);
    void Finalize();

    void ClearPortals();
    void AddPortal(const MapChipField::IndexSet& idx,
                   const std::string& targetMap,
                   const MyEngine::Vector3& startPos,
                   const MyEngine::Vector3& worldPos);

    // 現在登録されている全ポータルを返す（GameScene 側で配置に使用）
    const std::vector<PortalInfo>& GetPortals() const { return portals_; }

    // プレイヤーがいずれかのポータル上にいる場合、そのポータルへのポインタを返す。なければ nullptr
    const PortalInfo* GetPortalAt(const MapChipField::IndexSet& playerIndex) const;

    // 「E を押す」ヒントアイコンを更新（表示有無 + 位置）
    void UpdateHint(const MapChipField::IndexSet& playerIndex,
                    const MyEngine::Vector3& playerWorldPos,
                    bool canControl);

    // 描画ヒントアイコン（GameScene::Draw 内呼び出す）
    void DrawHint();

    // コイン未回収で使用不可の間、ポータル上に鍵アイコンを表示する
    void SetLockVisible(bool visible);
    void DrawLockIcons();

private:
    MyEngine::SpriteCommon* spriteCommon_ = nullptr;
    MyEngine::Camera*       camera_       = nullptr;

    std::unique_ptr<MyEngine::Sprite> hintSprite_;  // 「E を押す」アイコン
    std::unique_ptr<MyEngine::Sprite> lockSprite_;  // ポータルのロックアイコン
    std::vector<PortalInfo> portals_;
    bool showLockIcons_ = false;
    float lockPulseTimer_ = 0.0f;
    bool unlockBurstActive_ = false;
    float unlockBurstTimer_ = 0.0f;
};
