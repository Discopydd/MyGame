#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include <player/Player.h>
#include <memory>
class DashUIManager {
public:
    DashUIManager() = default;
    ~DashUIManager() = default;

    void Initialize(SpriteCommon* spriteCommon, Player* player);
    void Finalize();

    void Update(float dt);
    void Draw();

private:
    SpriteCommon* spriteCommon_ = nullptr;
    Player*       player_       = nullptr;

    std::unique_ptr<Sprite> icon_;    // 技能图标
    std::unique_ptr<Sprite> overlay_; // 冷却灰罩
};
