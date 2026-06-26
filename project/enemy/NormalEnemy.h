#pragma once

#include "Enemy.h"

// ------------------ 通常敵 ------------------
class NormalEnemy final : public Enemy {
public:
    void Initialize(MyEngine::Object3dCommon* common, MyEngine::Camera* camera, const MyEngine::Vector3& spawnPos, EnemyType type) override;
    void Update(float deltaTime, const MapChipField& map, const Player& player) override;
    void Draw() override;

    // プレイヤーの踏みつけ: 1回で死亡 + 死亡アニメ
    void OnStomp() override;

private:
    // ---------- 移動/重力/地面判定 ----------
    MyEngine::Vector3 velocity_{ 0.0f, 0.0f, 0.0f };
    bool    isOnGround_ = false;
    int     facing_ = 1;                 // 1 右 / -1 左
    float   gravity_ = -2.20f;           // Boss と同じスケール（毎秒）
    float   moveSpeed_ = 0.12f;          // 通常巡回速度

    // ---------- Type0（enemy0）: 警戒巡回 + 軽い段差対応 ----------
    float type0AlertRange_ = 5.5f;
    float type0AlertSpeed_ = 0.16f;
    float type0JumpVelocity_ = 0.42f;
    float type0JumpCooldown_ = 0.0f;
    float type0JumpCooldownTime_ = 0.90f;

    // ---------- Type1（enemy2）: 追跡 + 跳跃越障 ----------
    float chaseSpeed_ = 0.10f;           // Type1 横方向速度（比巡回やや快）
    float chaseBoostSpeed_ = 0.14f;      // 離れている時の追跡ブースト
    float smallJumpVelocity_ = 0.36f;    // 低ジャンプ: トゲ/小さな段差
    float highJumpVelocity_ = 0.52f;     // 高ジャンプ: 壁 / 高い段差

    float jumpCooldown_ = 0.0f;
    float jumpCooldownTime_ = 0.70f;

    float stopRange_ = 0.90f;
    float chaseMemoryTimer_ = 0.0f;
    float chaseMemoryTime_ = 1.15f;

    // 死亡アニメ中も地形との衝突を維持する（地面抜け防止）。プレイヤー衝突用の width_/height_ は 0 にされる
    float aliveWidth_ = 1.5f;
    float aliveHeight_ = 1.5f;

    // ---------- Type1（enemy2）: エリア制限（警戒 / リーシュ）+ 戦闘離脱後に戻る ----------
    enum class Type1State { Patrol, Chase, Return };
    Type1State type1State_ = Type1State::Patrol;

    MyEngine::Vector3 homePos_{};            // スポーン地点（「家」）
    float aggroRange_ = 12.0f;      // 警戒範囲: プレイヤー入るで初めて追う
    float leashRange_ = 20.0f;      // リーシュ範囲: 超えたら追跡をやめて戻る
    float patrolHalfWidth_ = 0.0f;  // 家付近巡回半幅；0=棒立ち
    float returnStopDist_ = 0.25f;  // 帰還判定距離

    // ---------- Type2（enemy3）: 間合い管理 + 飛びかかり ----------
    enum class Type2State { Patrol, Stalk, Pounce, Recover, Return };
    Type2State type2State_ = Type2State::Patrol;

    float stalkSpeed_ = 0.11f;
    float stalkPreferredMin_ = 1.9f;
    float stalkPreferredMax_ = 4.4f;
    float pounceSpeed_ = 0.22f;
    float pounceAirControl_ = 0.17f;
    float pounceJumpVelocity_ = 0.58f;
    float pounceCooldown_ = 0.0f;
    float pounceCooldownTime_ = 1.35f;
    float pounceMinRange_ = 2.8f;
    float pounceMaxRange_ = 8.5f;
    float pounceVerticalRange_ = 3.2f;
    float recoverTimer_ = 0.0f;
    float recoverDuration_ = 0.22f;

    // ---------- 死亡アニメ ----------
    bool  isDying_ = false;
    float deathTimer_ = 0.0f;
    float deathDuration_ = 0.45f;
    float deathSpin_ = 0.0f;             // Z 轴旋转
    float deathSpinSpeed_ = 10.0f;       // rad/s
};
