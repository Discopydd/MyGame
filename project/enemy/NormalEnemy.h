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

    // Type1（E1）用：追踪玩家 + 跳跃越障
    float chaseSpeed_ = 0.10f;         // Type1 横向速度（比巡逻略快）
    float smallJumpVelocity_ = 0.36f;   // 低跳：主要跨地刺/小台阶
    float highJumpVelocity_ = 0.52f;   // 高跳：跨 kBlock/kBlock2/移动平台等

    float jumpCooldown_ = 0.0f;
    float jumpCooldownTime_ = 0.70f;

    float stopRange_ = 0.90f;
    // 死亡动画时保持与地形的碰撞（避免“穿地”），玩家碰撞则用 width_/height_（会被置 0）
    float aliveWidth_ = 1.5f;
    float aliveHeight_ = 1.5f;

    // ---------- Type1（E1）：区域限制（警戒/牵引）+ 脱战返回 ----------
    enum class Type1State { Patrol, Chase, Return };
    Type1State type1State_ = Type1State::Patrol;

    Vector3 homePos_{};            // 出生点（“家”）
    float aggroRange_ = 12.0f;      // 警戒范围：玩家进入才追
    float leashRange_ = 20.0f;     // 牵引范围：超出就放弃追击返回（建议 >= aggroRange_）
    float patrolHalfWidth_ = 0.0f; // 家附近巡逻半宽；0=站桩
    float returnStopDist_ = 0.25f; // 回家判定距离

    // ---------- 死亡动画 ----------
    bool  isDying_ = false;
    float deathTimer_ = 0.0f;
    float deathDuration_ = 0.45f;
    float deathSpin_ = 0.0f;             // Z 轴旋转
    float deathSpinSpeed_ = 10.0f;       // rad/s
};
