#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include <player/Player.h>
#include <memory>

/// <summary>
/// プレイヤーのダッシュ使用状態とクールタイムを画面上に表示するUI管理クラス。
/// </summary>
class DashUIManager {
public:
    DashUIManager() = default;
    ~DashUIManager() = default;

    void Initialize(MyEngine::SpriteCommon* spriteCommon, Player* player);
    void Finalize();

    void Update(float dt);
    void Draw();

private:
    MyEngine::SpriteCommon* spriteCommon_ = nullptr;
    Player*       player_       = nullptr;

    std::unique_ptr<MyEngine::Sprite> icon_;    // スキルアイコン
    std::unique_ptr<MyEngine::Sprite> overlay_; // クールダウンのグレーオーバーレイ

    MyEngine::Vector2 overlayFullTexSize_{ 0.0f, 0.0f };
};
