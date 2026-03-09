#pragma once

#include "Enemy.h"

// ------------------ 通常敵 ------------------
class NormalEnemy final : public Enemy {
public:
    void Initialize(Object3dCommon* common, Camera* camera, const Vector3& spawnPos, EnemyType type) override;
    void Update(float deltaTime, const MapChipField& map, const Player& player) override;
    void Draw() override;

    // プレイヤーの踏みつけ: 1回で死亡 + 死亡アニメ
    void OnStomp() override;

private:
    // ---------- 移動/重力/地面判定 ----------
    Vector3 velocity_{ 0.0f, 0.0f, 0.0f };
    bool    isOnGround_ = false;
    int     facing_ = 1;                 // 1 右 / -1 左
    float   gravity_ = -2.20f;           // Boss と同じスケール（毎秒）
    float   moveSpeed_ = 0.12f;          // Boss の moveSpeed_ と同じスケール（60fps 基準の 1 フレーム移動量）

    // Type1（E1）用: 追踪プレイヤー + 跳跃越障
    float chaseSpeed_ = 0.10f;         // Type1 横方向速度（比巡回やや快）
    float smallJumpVelocity_ = 0.36f;   // 低ジャンプ: 主要跨トゲ/小さな段差
    float highJumpVelocity_ = 0.52f;   // 高ジャンプ: kBlock / kBlock2 / 移動床などを越える

    float jumpCooldown_ = 0.0f;
    float jumpCooldownTime_ = 0.70f;

    float stopRange_ = 0.90f;
    // 死亡アニメ中も地形との衝突を維持する（地面抜け防止）。プレイヤー衝突用の width_/height_ は 0 にされる
    float aliveWidth_ = 1.5f;
    float aliveHeight_ = 1.5f;

    // ---------- Type1（E1）: エリア制限（警戒 / リーシュ）+ 戦闘離脱後に戻る ----------
    enum class Type1State { Patrol, Chase, Return };
    Type1State type1State_ = Type1State::Patrol;

    Vector3 homePos_{};            // スポーン地点（「家」）
    float aggroRange_ = 12.0f;      // 警戒範囲: プレイヤー入るで初めて追
    float leashRange_ = 20.0f;     // リーシュ範囲: 超えたら追跡をやめて戻る（推奨: >= aggroRange_）
    float patrolHalfWidth_ = 0.0f; // 家付近巡回半幅；0=棒立ち
    float returnStopDist_ = 0.25f; // 帰還判定距離

    // ---------- 死亡アニメ ----------
    bool  isDying_ = false;
    float deathTimer_ = 0.0f;
    float deathDuration_ = 0.45f;
    float deathSpin_ = 0.0f;             // Z 轴旋转
    float deathSpinSpeed_ = 10.0f;       // rad/s
};
