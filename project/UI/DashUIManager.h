#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include <player/Player.h>

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

    Sprite* icon_      = nullptr; // 技能图标
    Sprite* overlay_   = nullptr; // 冷却灰罩
};
