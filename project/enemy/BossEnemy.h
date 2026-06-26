#pragma once

#include "Enemy.h"

#include <vector>
#include <memory>

// ------------------ Boss ------------------
class BossEnemy final : public Enemy {
public:
    void Initialize(MyEngine::Object3dCommon* common, MyEngine::Camera* camera, const MyEngine::Vector3& spawnPos, EnemyType type) override;
    void Update(float deltaTime, const MapChipField& map, const Player& player) override;
    void Draw() override;

    void OnStomp() override;
    bool CheckBossProjectileHit(const Player& player) override;

    // ================== Boss battle トリガー / HPバー表示 ==================
    bool IsBattleTriggered() const { return battleTriggered_; }
    bool IsBattleTriggerReady(const Player& player, const MapChipField& map) const;
    void TriggerBattleNow();

    // GameScene 用: Boss HPバーを表示するかどうかを決める
    bool ShouldShowBossHp(const Player& player, const MapChipField& map) const;
    void SetBattleTriggerXIndex(uint32_t xIndex) { battleTriggerXIndex_ = xIndex; }


private:
    // ================== 定数定義（マジックナンバー対策） ==================
    // BossEnemy.cpp で直接数値を書かず、調整値をここに集約する。
    inline static constexpr MyEngine::Vector3 kZeroVector_{ 0.0f, 0.0f, 0.0f };

    static constexpr float kPi_ = 3.14159265358979323846f;
    static constexpr float kTwoPi_ = kPi_ * 2.0f;
    static constexpr float kFrameRate_ = 60.0f;
    static constexpr float kTinyValue_ = 0.0001f;
    static constexpr float kSmallEpsilon_ = 0.001f;
    static constexpr float kHalf_ = 0.5f;
    static constexpr int   kFacingRight_ = 1;
    static constexpr int   kFacingLeft_ = -1;

    // HP / 基本挙動
    static constexpr int   kBossMaxHp_ = 3;
    static constexpr int   kNormalEnemyMaxHp_ = 1;
    static constexpr int   kEnrageHpDivisor_ = 5;
    static constexpr float kGravityBase_ = -2.20f;
    static constexpr float kMinFallVelocity_ = -2.5f;
    static constexpr float kDefaultVisualOffsetY_ = 0.0f;
    static constexpr uint32_t kDefaultBattleTriggerXIndex_ = 33;

    // モデル / 判定
    static constexpr float kDefaultEnemyWidth_ = 1.5f;
    static constexpr float kDefaultEnemyHeight_ = 1.5f;
    static constexpr float kBossModelScale_ = 1.35f;
    static constexpr float kBossVisualOffsetY_ = -0.85f;
    static constexpr float kBossBaseWidth_ = 2.6f;
    static constexpr float kBossBaseHeight_ = 3.0f;
    static constexpr float kBossDamageBlinkInterval_ = 0.055f;
    static constexpr float kDefaultDamageBlinkInterval_ = 0.08f;
    static constexpr float kDashHurtScaleX_ = 0.70f;
    static constexpr float kDashHurtScaleY_ = 0.85f;

    // AI / 移動調整値
    static constexpr float kMoveSpeed_ = 0.18f;
    static constexpr float kDashSpeed_ = 0.45f;
    static constexpr float kJumpVelocity_ = 0.62f;
    static constexpr float kDetectRange_ = 18.0f;
    static constexpr float kIdealRange_ = 4.0f;
    static constexpr float kMeleeRange_ = 2.2f;
    static constexpr float kDashMinRange_ = 4.0f;
    static constexpr float kDashMaxRange_ = 10.0f;
    static constexpr float kRangedMinRange_ = 4.2f;
    static constexpr float kRangedMaxRange_ = 14.0f;
    static constexpr float kCloseDashRange_ = 4.8f;
    static constexpr float kCloseDashWindup_ = 0.12f;
    static constexpr float kCloseDashDuration_ = 0.16f;
    static constexpr float kCloseDashSpeed_ = 0.30f;
    static constexpr float kMicroDashCooldown_ = 1.20f;
    static constexpr float kFarRangedPrefer_ = 9.0f;
    static constexpr float kUltimateSpeed_ = 0.35f;
    static constexpr float kUltimateDuration_ = 5.0f;
    static constexpr float kUltimateInitialCooldown_ = 2.0f;
    static constexpr int   kUltimateMaxBounces_ = 4;
    static constexpr float kRestDuration_ = 1.8f;
    static constexpr float kLeadTime_ = 0.25f;
    static constexpr float kProjectileLeadTime_ = 0.35f;

    // 攻撃選択 / クールダウン
    static constexpr float kPlayerMovingAwayThreshold_ = 0.05f;
    static constexpr float kDistanceIncreasingMargin_ = 0.08f;
    static constexpr float kChaseDeadZone_ = 0.35f;
    static constexpr float kMoveEpsilon_ = 0.01f;
    static constexpr float kObstacleCheckXMargin_ = 0.15f;
    static constexpr float kObstacleCheckYMargin_ = 0.10f;
    static constexpr float kPointBlankRangeMargin_ = 0.4f;
    static constexpr float kBarrageMinRange_ = 5.5f;
    static constexpr float kBarrageMaxRange_ = 13.5f;
    static constexpr float kPlayerAboveHeightRate_ = 0.15f;
    static constexpr float kSlamEnableRange_ = 6.5f;
    static constexpr float kNovaMinRange_ = 4.8f;
    static constexpr float kNovaMaxRange_ = 12.8f;
    static constexpr float kUltimateMinRange_ = 2.0f;
    static constexpr float kUltimateMaxRange_ = 14.0f;
    static constexpr float kUltimateWindupTime_ = 0.40f;
    static constexpr float kUltimateCooldownLock_ = 399.0f;
    static constexpr float kUltimateCooldownLockThreshold_ = 100.0f;
    static constexpr float kSlamCloseRangeMargin_ = 1.2f;
    static constexpr float kNovaAntiRetreatRange_ = 8.0f;
    static constexpr float kMiddleDashNearRange_ = 7.0f;

    static constexpr float kSlamCooldownEnrage_ = 3.0f;
    static constexpr float kSlamCooldownNormal_ = 4.2f;
    static constexpr float kNovaCooldownEnrageBase_ = 3.2f;
    static constexpr float kNovaCooldownEnrageRandom_ = 1.0f;
    static constexpr float kNovaCooldownNormalBase_ = 4.6f;
    static constexpr float kNovaCooldownNormalRandom_ = 1.2f;
    static constexpr float kBarrageCooldownEnrageBase_ = 3.0f;
    static constexpr float kBarrageCooldownEnrageRandom_ = 1.0f;
    static constexpr float kBarrageCooldownNormalBase_ = 4.0f;
    static constexpr float kBarrageCooldownNormalRandom_ = 1.2f;
    static constexpr float kRangedCooldownEnrage_ = 1.25f;
    static constexpr float kRangedCooldownNormal_ = 1.75f;
    static constexpr float kCloseDashCooldown_ = 2.70f;
    static constexpr float kMiddleDashCooldown_ = 2.10f;

    // 状態タイマー
    static constexpr float kStunnedNextDecisionTime_ = 0.12f;
    static constexpr float kUltimateGlobalCooldown_ = 1.10f;
    static constexpr float kUltimateDecisionDelay_ = 0.35f;
    static constexpr float kSlamGlobalCooldown_ = 1.15f;
    static constexpr float kSlamDecisionDelay_ = 0.30f;
    static constexpr float kNovaGlobalCooldown_ = 1.15f;
    static constexpr float kNovaDecisionDelay_ = 0.32f;
    static constexpr float kBarrageWindupTime_ = 0.30f;
    static constexpr float kBarrageGlobalCooldown_ = 1.05f;
    static constexpr float kBarrageDecisionDelay_ = 0.30f;
    static constexpr float kRangedWindupTime_ = 0.28f;
    static constexpr float kRangedGlobalCooldown_ = 0.85f;
    static constexpr float kRangedDecisionDelay_ = 0.25f;
    static constexpr float kCloseDashGlobalCooldown_ = 1.05f;
    static constexpr float kCloseDashDecisionDelay_ = 0.25f;
    static constexpr float kMicroDashGlobalCooldown_ = 0.95f;
    static constexpr float kMicroDashDecisionDelay_ = 0.20f;
    static constexpr float kMiddleDashGlobalCooldown_ = 1.00f;
    static constexpr float kMiddleDashDecisionDelay_ = 0.25f;
    static constexpr float kShortDashRecoverTime_ = 0.45f;
    static constexpr float kNormalDashRecoverTime_ = 0.65f;
    static constexpr float kBarrageRecoverTime_ = 0.70f;
    static constexpr float kShootRecoverTime_ = 0.55f;
    static constexpr float kDashWallRecoverTime_ = 0.45f;
    static constexpr float kBattleStartDecisionDelay_ = 0.30f;
    static constexpr float kBattleStartGlobalCooldown_ = 0.60f;
    static constexpr float kRestNextDecisionDelay_ = 0.25f;

    // 弾 / 演出
    static constexpr float kProjectileSpeed_ = 0.25f;
    static constexpr float kProjectileLife_ = 2.20f;
    static constexpr float kProjectileRadius_ = 0.35f;
    static constexpr float kBarrageDuration_ = 1.35f;
    static constexpr float kBarrageFireInterval_ = 0.055f;
    static constexpr float kBarrageAngularSpeed_ = 7.0f;
    static constexpr float kBarrageProjectileSpeed_ = 0.22f;
    static constexpr float kBarrageProjectileLife_ = 2.10f;
    static constexpr float kBarrageBurstInterval_ = 0.38f;
    static constexpr int   kBarrageBurstCount_ = 10;
    static constexpr float kBarrageBurstSpeed_ = 0.18f;
    static constexpr float kBarrageBurstLife_ = 1.45f;
    static constexpr float kNovaWindup_ = 0.26f;
    static constexpr float kNovaDuration_ = 0.95f;
    static constexpr float kNovaRecover_ = 0.85f;
    static constexpr int   kNovaRingsNormal_ = 2;
    static constexpr int   kNovaRingsEnrage_ = 3;
    static constexpr float kNovaRingInterval_ = 0.16f;
    static constexpr int   kNovaBulletCount_ = 18;
    static constexpr float kNovaProjectileSpeed_ = 0.26f;
    static constexpr float kNovaProjectileLife_ = 1.65f;
    static constexpr float kSlamWindup_ = 0.32f;
    static constexpr float kSlamJumpVelocity_ = 0.92f;
    static constexpr float kSlamMaxAirTime_ = 1.60f;
    static constexpr float kSlamImpactHold_ = 0.18f;
    static constexpr float kSlamRecover_ = 0.75f;
    static constexpr float kSlamWaveSpeed_ = 0.38f;
    static constexpr float kSlamWaveLife_ = 1.70f;
    static constexpr float kSlamShardSpeed_ = 0.30f;
    static constexpr float kSlamShardLife_ = 1.40f;
    static constexpr float kShotInterval_ = 0.16f;
    static constexpr int   kFanShotCount_ = 5;
    static constexpr float kFanHalfAngle_ = 0.52f;
    static constexpr float kFanProjectileSpeed_ = 0.24f;

    // 射撃 / 弾幕の詳細調整値
    static constexpr float kFanShotMaxDistance_ = 9.5f;
    static constexpr float kFanShotStateTime_ = 0.95f;
    static constexpr int   kFanShotWaveCount_ = 2;
    static constexpr float kFanShotInterval_ = 0.22f;
    static constexpr float kEnrageShotStateTime_ = 0.85f;
    static constexpr float kNormalShotStateTime_ = 0.55f;
    static constexpr int   kEnrageShotCount_ = 3;
    static constexpr int   kNormalShotCount_ = 1;
    static constexpr float kBarrageEnrageDurationBonus_ = 0.25f;
    static constexpr float kBarrageBurstStartRate_ = 0.65f;
    static constexpr float kRandomTurnThreshold_ = 0.5f;
    static constexpr float kBarrageEnrageFireIntervalRate_ = 0.85f;
    static constexpr float kBarrageEnrageSpeedRate_ = 1.15f;
    static constexpr int   kBarrageEnrageBurstAdd_ = 4;
    static constexpr float kBossChestHeightRate_ = 0.18f;
    static constexpr float kBarrageBurstRadius_ = 0.28f;
    static constexpr float kBarrageSpawnForwardRate_ = 0.15f;
    static constexpr float kBarrageShotRadius_ = 0.32f;
    static constexpr int   kBarrageEmitCountNormal_ = 1;
    static constexpr int   kBarrageEmitCountEnrage_ = 2;
    static constexpr float kNovaRingSpeedStep_ = 0.08f;
    static constexpr float kNovaBurstRadius_ = 0.30f;
    static constexpr float kNovaCrossSpeedRate_ = 1.25f;
    static constexpr float kNovaCrossRadius_ = 0.34f;
    static constexpr float kNovaRingOffsetStep_ = 0.35f;
    static constexpr float kSlamBaseHeightRate_ = 0.50f;
    static constexpr float kSlamBaseLiftOffset_ = 0.25f;
    static constexpr float kSlamWaveRadius_ = 0.36f;
    static constexpr float kSlamShardRadius_ = 0.30f;
    static constexpr float kSlamShardOuterX_ = 1.00f;
    static constexpr float kSlamShardInnerX_ = 0.55f;
    static constexpr float kSlamShardLowY_ = 0.85f;
    static constexpr float kSlamShardHighY_ = 1.00f;
    static constexpr float kSlamShardEnrageX_ = 0.30f;
    static constexpr float kSlamShardEnrageY_ = 1.20f;
    static constexpr float kSlamExplosionLiftY_ = 0.45f;
    static constexpr int   kSlamExplosionCountNormal_ = 12;
    static constexpr int   kSlamExplosionCountEnrage_ = 14;
    static constexpr float kSlamExplosionSpeedNormal_ = 0.20f;
    static constexpr float kSlamExplosionSpeedEnrage_ = 0.22f;
    static constexpr float kSlamExplosionLife_ = 1.25f;
    static constexpr float kShootSpawnForwardOffset_ = 0.25f;
    static constexpr float kShootVerticalSpread_ = 0.55f;
    static constexpr float kFanShotBaseRotateStep_ = 0.18f;
    static constexpr float kFanAngleWidthRate_ = 2.0f;
    static constexpr float kUltimateSpikeFootOffset_ = 0.05f;
    static constexpr float kUltimateSpikeBurstLife_ = 1.10f;
    static constexpr int   kUltimateSpikeBurstCountNormal_ = 12;
    static constexpr int   kUltimateSpikeBurstCountEnrage_ = 14;
    static constexpr float kUltimateSpikeBurstSpeedNormal_ = 0.20f;
    static constexpr float kUltimateSpikeBurstSpeedEnrage_ = 0.22f;
    static constexpr float kUltimateBounceBurstLife_ = 1.05f;
    static constexpr int   kUltimateBounceBurstCountNormal_ = 10;
    static constexpr int   kUltimateBounceBurstCountEnrage_ = 12;
    static constexpr float kUltimateBounceBurstSpeedNormal_ = 0.19f;
    static constexpr float kUltimateBounceBurstSpeedEnrage_ = 0.21f;
    static constexpr float kUltimateBurstRadius_ = 0.28f;
    static constexpr float kUltimateCooldownEnrageBase_ = 2.8f;
    static constexpr float kUltimateCooldownEnrageRandom_ = 1.4f;
    static constexpr float kUltimateCooldownNormalBase_ = 4.0f;
    static constexpr float kUltimateCooldownNormalRandom_ = 2.0f;

    // 当たり判定 / トリガー
    static constexpr float kBattleTriggerVerticalRange_ = 2.5f;
    static constexpr float kGroundProbeOffsetY_ = 0.06f;
    static constexpr float kFootProbeXScale_ = 0.80f;
    static constexpr float kBattleTriggerEdgeEps_ = 0.02f;
    static constexpr float kFacingDeadZone_ = 0.05f;
    static constexpr int   kGroundProbeCount_ = 3;

    // 被弾 / 踏みつけ反応
    static constexpr float kBossHitReactTime_ = 0.55f;
    static constexpr float kBossStunnedTime_ = 0.60f;
    static constexpr float kBossHitDecisionDelay_ = 0.20f;
    static constexpr float kBossHitGlobalCooldown_ = 0.80f;
    static constexpr float kBossStompInvulnTime_ = 0.90f;
    static constexpr float kNormalHitReactTime_ = 0.40f;
    static constexpr float kNormalStompInvulnTime_ = 0.20f;

    // 微振動予備動作
    static constexpr float kPreJitterLead_ = 0.18f;
    static constexpr float kPreJitterSettle_ = 0.08f;
    static constexpr float kPreJitterAmpX_ = 0.06f;
    static constexpr float kPreJitterAmpY_ = 0.04f;
    static constexpr float kPreJitterFreqX1_ = 97.0f;
    static constexpr float kPreJitterFreqX2_ = 211.0f;
    static constexpr float kPreJitterPhaseX2_ = 1.1f;
    static constexpr float kPreJitterFreqY1_ = 131.0f;
    static constexpr float kPreJitterPhaseY1_ = 2.7f;
    static constexpr float kPreJitterFreqY2_ = 233.0f;
    static constexpr float kPreJitterPhaseY2_ = 0.2f;
    static constexpr float kPreJitterSubWaveRate_ = 0.35f;

    float visualOffsetY_ = kDefaultVisualOffsetY_;


    // ----- 微振動予備動作
    MyEngine::Vector3 preAttackJitter_{ kZeroVector_ };
    float   preAttackJitterTime_ = 0.0f;

    // 最後の X 秒に入ったら揺れを開始する（秒）
    float preJitterLeadBarrage_ = kPreJitterLead_;
    float preJitterLeadNova_    = kPreJitterLead_;
    float preJitterLeadJump_    = kPreJitterLead_; // Slam の跳び上がり前（Jump の予備動作）

    // 揺れ終了後に少しだけ静止時間を置いてから技を出す（秒）
    float preJitterSettle_ = kPreJitterSettle_;
    float preJitterAmpX_ = kPreJitterAmpX_;
    float preJitterAmpY_ = kPreJitterAmpY_;


    MyEngine::Vector3 GetRenderPosition() const {
        MyEngine::Vector3 p = position_;
        p.x += preAttackJitter_.x;
        p.y += visualOffsetY_ + preAttackJitter_.y;
        p.z += preAttackJitter_.z;
        return p;
    }

    // ---------- 移動/衝突（Boss 用: position += velocity） ----------
    MyEngine::Vector3 velocity_{ kZeroVector_ };
    bool    isOnGround_ = false;
    float   gravityBase_ = kGravityBase_;

    // ---------- 判定（プレイヤー相互作用）とマップ衝突を分離 ----------
    float mapColliderW_ = 0.0f;
    float mapColliderH_ = 0.0f;

    // 非 Dash 時の基礎「プレイヤー判定」サイズ（毎フレーム復元用）
    float baseHurtW_ = 0.0f;
    float baseHurtH_ = 0.0f;

    // Dash 時縮小判定比例（さらに当てにくくしたいなら 0.60/0.75 などに下げる）
    float dashHurtScaleX_ = kDashHurtScaleX_;
    float dashHurtScaleY_ = kDashHurtScaleY_;

    // ---------- Boss 戦闘トリガー ----------
    bool     battleTriggered_ = false;
    uint32_t battleTriggerXIndex_ = kDefaultBattleTriggerXIndex_;
    bool     requirePlayerOnGroundToTrigger_ = true;

    bool  requirePlayerOnGroundToEngage_ = true; // 接近判定でもプレイヤーが地面上にいること
    float engageVerticalRange_ = 6.0f;

    bool IsPlayerOnGround(const MapChipField& map, const Player& player) const;
    bool IsInEngageRange(const MapChipField& map, const Player& player) const;


    // ---------- Boss AI ----------
    enum class BossState {
        Idle,
        Chase,
        Windup,
        Dash,
        Shoot,
        Barrage,   // 回転弾幕
        Nova,      // 円形爆発
        Jump,      // 跳起
        Slam,      // 着地叩きつけ
        Ultimate,
        Rest,
        Recover,
        Stunned
    };
    BossState bossState_ = BossState::Idle;

    enum class BossAttack { None, Dash, Ranged, Barrage, Nova, Slam, Ultimate };
    BossAttack queuedAttack_ = BossAttack::None;

    float stateTimer_ = 0.0f;   // 予備動作/攻撃
    float decisionTimer_ = 0.0f;
    int   facing_ = 1;      // 1右 -1左
    int   attackFacing_ = 1;      // 本次攻撃向きを固定

    // Dash の持続時間
    float queuedDashDuration_ = kBarrageWindupTime_;
    float queuedDashSpeed_ = kDashSpeed_;  // Dash 実際に使う速度
    bool  isShortDash_ = false;

    float dashBackstepDist_      = 0.75f; // 普通ダッシュ: 後退距離（ワールド座標）
    float dashBackstepDistShort_ = kDashSpeed_; // 密着小ダッシュ: 後退距離
    float dashBackstepMaxSpeed_  = 0.11f; // 後退最大速度（60fps 基準の1フレーム移動量）
    float dashBackstepMinSpeed_  = 0.03f; // 後退最小速度（遅すぎて分かりにくくならないようにする）
    float dashWindupTotal_       = 0.0f;  // 今回の Dash Windup 総時間を記録し、時間に応じて均等に後退させる
    float dashBackstepMoved_     = 0.0f;  // 本次 Windup 後退済み距離（打ち切り用）
    float ultimateWindupBackstepDist_      = 2.00f; // 必殺技発動前: 後退距離（通常より長い）
    float ultimateWindupBackstepMaxSpeed_  = 0.14f; // Windup 後退最大速度（60fps 基準の1フレーム移動量）
    float ultimateWindupBackstepMinSpeed_  = 0.04f; // Windup 後退最小速度
    float ultimateWindupTotal_             = 0.0f;  // 今回の必殺技 Windup の総時間を記録
    float ultimateWindupBackstepMoved_     = 0.0f;  // Windup 已後退距離（打ち切り用）

    // 前フレームのプレイヤーとの距離を記録
    float prevDistToPlayer_ = 1e9f;

    //
    float globalAttackCD_ = 0.0f;
    float meleeCD_ = 0.0f;  // 近接クールダウン
    float dashCD_ = 0.0f;  // ダッシュクールダウン
    float microDashCD_ = 0.0f;  // 小ダッシュ冷却（dashCD_ を消費しない）
    float rangedCD_ = 0.0f;  // 远程冷却
    float barrageCD_ = 0.0f;  // 回転弾幕クールダウン
    float slamCD_ = 0.0f;  // 叩きつけクールダウン
    float novaCD_ = 0.0f;  // 円形爆発クールダウン
    float ultimateCD_ = kSlamCooldownEnrage_;  // 必殺技クールダウン（開幕少し遅延を持たせる）
    int   ultimateBounces_ = 0;    // 必殺技: 反射回数

    // ---------- 调参区 ----------
    float moveSpeed_ = kMoveSpeed_;  // 追跡速度（「1フレーム移動量」）
    float dashSpeed_ = kDashSpeed_;  // ダッシュ速度（1フレーム移動量）
    float jumpVel_ = kJumpVelocity_;

    float detectRange_ = kDetectRange_;
    float idealRange_ = kIdealRange_;
    float meleeRange_ = kMeleeRange_;
    float dashMinRange_ = kDashMinRange_;
    float dashMaxRange_ = kDashMaxRange_;

    // 遠距離攻撃距離
    float rangedMinRange_ = kRangedMinRange_;
    float rangedMaxRange_ = kRangedMaxRange_;

    // 近距離ダッシュ: プレイヤー接近時に即座にダッシュするが、連続で突進し続けない（dashCD_ で制御）
    float closeDashRange_ = kCloseDashRange_;
    float closeDashWindup_ = kCloseDashWindup_;
    float closeDashDuration_ = kCloseDashDuration_;
    float closeDashSpeed_ = kCloseDashSpeed_;  // 小ダッシュ速度（1フレーム移動量）
    float microDashCooldown_ = kMicroDashCooldown_;  // dashCD_ が無い時でも 1 回だけ小ダッシュを許可するためのクールダウン

    // 遠距離制圧: 距離がかなり離れている時は遠距離攻撃を優先
    float farRangedPrefer_ = kFarRangedPrefer_;

    // 必殺技: 左右往復ダッシュ
    float ultimateSpeed_ = kUltimateSpeed_;
    float ultimateDuration_ = kUltimateDuration_; // failsafe: 最長持続
    int   ultimateMaxBounces_ = kUltimateMaxBounces_;    // 反射回数（4=左右往復 2 セット）
    float restDuration_ = kRestDuration_;

    float leadTime_ = kLeadTime_;           // 追跡予判: playerPos + playerVel * leadTime
    float projectileLeadTime_ = kProjectileLeadTime_; // 弾幕照準予判

    // ---------- Boss 弾幕（オブジェクトプール） ----------
    struct BossProjectile {
        std::unique_ptr<MyEngine::Object3d> obj;
        MyEngine::Vector3 pos{ kZeroVector_ };
        MyEngine::Vector3 vel{ kZeroVector_ };
        float   life = 0.0f;
        float   radius = kProjectileRadius_;
        bool    active = false;
    };

    // 弾幕が密なほど派手になるので、プール数を少し増やす
    static constexpr int kMaxBossProjectiles_ = 48;
    std::vector<BossProjectile> projectiles_;

    float projectileSpeed_ = kProjectileSpeed_;  // 1フレーム移動量（跟 dashSpeed_ 同じスケール）
    float projectileLife_ = kProjectileLife_;  // 秒

    // ---------- 追加: 派手な技のパラメータ ----------
    // 旋转弾幕（Barrage）: Boss を中心に回転しながら発射
    float barrageDuration_ = kBarrageDuration_; // 秒
    float barrageFireInterval_ = kBarrageFireInterval_; // 秒
    float barrageFireTimer_ = 0.0f;
    float barrageAngle_ = 0.0f;  // 弧度
    float barrageAngularSpeed_ = kBarrageAngularSpeed_;  // rad/sec
    float barrageSpinDir_ = 1.0f;  // +1/-1: 回転方向
    float barrageProjectileSpd_ = kBarrageProjectileSpeed_; // 1フレーム移動量（projectileSpeed_ とは独立して調整可能）
    float barrageProjectileLife_ = kBarrageProjectileLife_;

    float barrageBurstInterval_ = kBarrageBurstInterval_;
    float barrageBurstTimer_ = 0.0f;
    int   barrageBurstCount_ = kBarrageBurstCount_;
    float barrageBurstSpeed_ = kBarrageBurstSpeed_;
    float barrageBurstLife_ = kBarrageBurstLife_;

    // 円形爆発（Nova）: 多重リング弾幕
    float novaWindup_ = kNovaWindup_;
    float novaDuration_ = kNovaDuration_;
    float novaRecover_ = kNovaRecover_;
    int   novaRingsNormal_ = kNovaRingsNormal_;
    int   novaRingsEnrage_ = kNovaRingsEnrage_;
    float novaRingInterval_ = kNovaRingInterval_;
    int   novaBulletCount_ = kNovaBulletCount_;
    float novaProjectileSpd_ = kNovaProjectileSpeed_;
    float novaProjectileLife_ = kNovaProjectileLife_;
    float novaRingOffset_ = 0.0f;
    float novaFireTimer_ = 0.0f;
    int   novaRingsLeft_ = 0;

    // 叩きつけ（Slam）: 跳び上がり後落地生成「左右の衝撃波 + 破片」
    float slamWindup_ = kSlamWindup_;
    float slamJumpVel_ = kSlamJumpVelocity_;
    float slamMaxAirTime_ = kSlamMaxAirTime_; // failsafe
    float slamImpactHold_ = kSlamImpactHold_; // 着地停止
    float slamRecover_ = kSlamRecover_;
    float slamWaveSpeed_ = kSlamWaveSpeed_;
    float slamWaveLife_ = kSlamWaveLife_;
    float slamShardSpeed_ = kSlamShardSpeed_;
    float slamShardLife_ = kSlamShardLife_;
    bool  slamSpawned_ = false;

    // Shoot 状態内の連射
    int   shotsLeft_ = 0;
    int   shotsTotal_ = 0;
    float shotInterval_ = kShotInterval_;  // 秒
    float shotTimer_ = 0.0f;   // 秒（<=0 で発射）

    // 追加: 扇状散布（Shoot 状態内の「オン / オフ」で実装し、追加状態は増やさない）
    bool  fanShot_ = false;
    int   fanCount_ = kFanShotCount_;      // 1 波あたりの弾数
    float fanHalfAngle_ = kFanHalfAngle_;  // 半角（弧度）≈ 30°
    float fanProjectileSpd_ = kFanProjectileSpeed_;

    bool ultimateLocked_ = false;     // ultimateCD_ を 399 まで引き上げたロック期間中かどうか

    float battleTriggerVerticalRange_ = kBattleTriggerVerticalRange_;
    // 汎用: リング弾幕用ツール（オブジェクトプールを再利用）
    void SpawnRadialBurst(const MyEngine::Vector3& center, int count, float speed, float life, float radius, float angleOffset = 0.0f);

private:
    void UpdatePreAttackJitter(float dt);

    void ResolveMapCollision(const MapChipField& map, float dt);
    void UpdateBossFacing(const Player& player);

    void UpdateBossProjectiles(float dt, const MapChipField& map);
    void SpawnBossProjectile(const MyEngine::Vector3& spawnPos, const MyEngine::Vector3& aimDir);
    void SpawnBossProjectileRaw(const MyEngine::Vector3& spawnPos, const MyEngine::Vector3& velocity, float life, float radius);
    void DrawBossProjectiles();

    void FinishUltimateCooldown();
};
