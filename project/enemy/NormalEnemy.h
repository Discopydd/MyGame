#pragma once

#include "Enemy.h"

// ------------------ 普通敌人 ------------------
class NormalEnemy final : public Enemy {
public:
    void Initialize(Object3dCommon* common, Camera* camera, const Vector3& spawnPos, EnemyType type) override;
    void Update(float deltaTime, const MapChipField& map, const Player& player) override;
    void Draw() override;

    // 玩家踩头：一次死亡 + 死亡动画
    void OnStomp() override;

private:
    // ---------- 移动/重力/地面判定 ----------
    Vector3 velocity_{ 0.0f, 0.0f, 0.0f };
    bool    isOnGround_ = false;
    int     facing_ = 1;                 // 1 右 / -1 左
    float   gravity_ = -2.20f;           // 与 Boss 同量纲（每秒）
    float   moveSpeed_ = 0.12f;          // 与 Boss moveSpeed_ 同量纲（按 60fps 每帧位移）

    // 死亡动画时保持与地形的碰撞（避免“穿地”），玩家碰撞则用 width_/height_（会被置 0）
    float aliveWidth_  = 1.5f;
    float aliveHeight_ = 1.5f;

    // ---------- 死亡动画 ----------
    bool  isDying_ = false;
    float deathTimer_ = 0.0f;
    float deathDuration_ = 0.45f;
    float deathSpin_ = 0.0f;             // Z 轴旋转
    float deathSpinSpeed_ = 10.0f;       // rad/s
};
