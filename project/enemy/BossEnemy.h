#pragma once

#include "Enemy.h"

#include <vector>
#include <memory>

// ------------------ Boss ------------------
class BossEnemy final : public Enemy {
public:
    void Initialize(Object3dCommon* common, Camera* camera, const Vector3& spawnPos, EnemyType type) override;
    void Update(float deltaTime, const MapChipField& map, const Player& player) override;
    void Draw() override;

    void OnStomp() override;
    bool CheckBossProjectileHit(const Player& player) override;

    // ================== Boss battle トリガー / HPバー表示 ==================
    // 目的:
    // - のみプレイヤー「入る Boss 区域/接近 Boss」後、Boss で初めて開始攻撃
    // - のみ接近 Boss 時で初めて表示 Boss HPバー（GameScene から呼び出す）
    bool IsBattleTriggered() const { return battleTriggered_; }

    // GameScene 用: 当プレイヤー到達 Boss トリガー点時、先に播「カメラ演出」、
    // 演出終了後に TriggerBattleNow() を呼び出し、Boss の正式戦闘を開始する。
    // これにより: カメラが Boss に寄る → 名前を表示 → カメラがプレイヤーに戻る → Boss が AI／攻撃を開始し、HPバーが表示される。
    bool IsBattleTriggerReady(const Player& player, const MapChipField& map) const;
    void TriggerBattleNow();

    // GameScene 用: Boss HPバーを表示するかどうかを決める（必要なら「トリガー後は常時表示」に変更してもよい）
    bool ShouldShowBossHp(const Player& player, const MapChipField& map) const;

    // 任意: 「トリガー列」（0-based）を変更する。たとえば AH 列は 33。
    void SetBattleTriggerXIndex(uint32_t xIndex) { battleTriggerXIndex_ = xIndex; }


private:
    // ----- 見た目(モデル)のY補正 -----
    // NOTE: position_ は当たり判定(AABB)の中心を想定しているが、
    //    モデルの原点(pivot)が足元に無い場合、拡大時に「浮いて見える」ことがある。
    //    この値は「描画のみ」に効かせる（当たり判定/物理には影響しない）。
    float visualOffsetY_ = 0.0f;


    // ----- 微振動予備動作（描画のみに影響し、衝突／物理／弾幕発生位置には影響しない） -----
    Vector3 preAttackJitter_{ 0.0f, 0.0f, 0.0f };
    float   preAttackJitterTime_ = 0.0f;

    // 最後の X 秒に入ったら揺れを開始する（秒）
    float preJitterLeadBarrage_ = 0.18f;
    float preJitterLeadNova_    = 0.18f;
    float preJitterLeadJump_    = 0.18f; // Slam の跳び上がり前（Jump の予備動作）

    

    // 揺れ終了後に少しだけ静止時間を置いてから技を出す（秒）
    float preJitterSettle_ = 0.08f;
// 揺れの振幅（ワールド座標、できるだけ小さめ推奨）
    float preJitterAmpX_ = 0.06f;
    float preJitterAmpY_ = 0.04f;


    Vector3 GetRenderPosition() const {
        Vector3 p = position_;
        p.x += preAttackJitter_.x;
        p.y += visualOffsetY_ + preAttackJitter_.y;
        p.z += preAttackJitter_.z;
        return p;
    }

    // ---------- 移動/衝突（Boss 用: position += velocity） ----------
    Vector3 velocity_{ 0,0,0 };
    bool    isOnGround_ = false;
    float   gravityBase_ = -2.20f;

    // ---------- 判定（プレイヤー相互作用）とマップ衝突を分離 ----------
    // 目標: Dash 時は「プレイヤーとの判定」だけを縮小し、Boss のマップ衝突には影響させない（さもないと壁抜けや壁詰まりの感触が変わる）。
    // 前提:
    // - width_/height_: プレイヤー相互作用用（被ダメージ / 踏みつけ / 近接命中など）
    // - mapColliderW_/mapColliderH_: 僅〜用 ResolveMapCollision（マップブロックとの衝突）
    float mapColliderW_ = 0.0f;
    float mapColliderH_ = 0.0f;

    // 非 Dash 時の基礎「プレイヤー判定」サイズ（毎フレーム復元用）
    float baseHurtW_ = 0.0f;
    float baseHurtH_ = 0.0f;

    // Dash 時縮小判定比例（さらに当てにくくしたいなら 0.60/0.75 などに下げる）
    float dashHurtScaleX_ = 0.70f;
    float dashHurtScaleY_ = 0.85f;

    // ---------- Boss 戦闘トリガー（未トリガー時: AIより新なし／弾発射なし／HPバー非表示） ----------
    // デフォルト: プレイヤー xIndex >= 33（AH列、0-based）かつ地面上にいる時に初めて Boss を起動する
    bool     battleTriggered_ = false;
    uint32_t battleTriggerXIndex_ = 33;              // AH (0-based)
    bool     requirePlayerOnGroundToTrigger_ = true; // トリガー時にプレイヤーが「地面上」にいることを要求

    // 「接近した時に初めて攻撃 / HPバー表示」を行う判定（プレイヤーが上空や別階層から Boss を起動してしまうのを防ぐ）
    bool  requirePlayerOnGroundToEngage_ = true; // 接近判定でもプレイヤーが地面上にいることを要求
    float engageVerticalRange_ = 6.0f;           // 許容する |dy|（ワールド座標）

    bool IsPlayerOnGround(const MapChipField& map, const Player& player) const;
    bool IsInEngageRange(const MapChipField& map, const Player& player) const;


    // ---------- Boss AI ----------
    // 説明: できるだけ元のステートマシンを踏襲し、少量の追加状態で「より派手」な技を実装する
    enum class BossState {
        Idle,
        Chase,
        Windup,
        Dash,
        Shoot,
        Barrage,   // 回転弾幕（棒立ち／小範囲移動の「弾幕地獄」）
        Nova,      // 円形爆発（多重リング弾幕、より「ド派手」）
        Jump,      // 跳起（准备叩きつけ）
        Slam,      // 着地叩きつけ（衝撃波弾幕を生成）
        Ultimate,
        Rest,
        Recover,
        Stunned
    };
    BossState bossState_ = BossState::Idle;

    enum class BossAttack { None, Dash, Ranged, Barrage, Nova, Slam, Ultimate };
    BossAttack queuedAttack_ = BossAttack::None;

    float stateTimer_ = 0.0f;   // 予備動作/攻撃/後隙/硬直计時
    float decisionTimer_ = 0.0f;   // 限制頻繁换招
    int   facing_ = 1;      // 1右 -1左
    int   attackFacing_ = 1;      // 本次攻撃向きを固定

    // Dash の持続時間（Windup -> Dash へ渡す）
    float queuedDashDuration_ = 0.30f;
    float queuedDashSpeed_ = 0.45f;  // Dash 実際に使う速度（Windup -> Dash）
    bool  isShortDash_ = false;  // 密着小ダッシュ/小ダッシュ: 〜用より短後隙



    // Dash 発動前小後退（溜め動作）: 〜させるダッシュより好予兆を読む、より有"初動"
    float dashBackstepDist_      = 0.75f; // 普通ダッシュ: 後退距離（ワールド座標）
    float dashBackstepDistShort_ = 0.45f; // 密着小ダッシュ: 後退距離
    float dashBackstepMaxSpeed_  = 0.11f; // 後退最大速度（60fps 基準の1フレーム移動量）
    float dashBackstepMinSpeed_  = 0.03f; // 後退最小速度（遅すぎて分かりにくくならないようにする）
    float dashWindupTotal_       = 0.0f;  // 今回の Dash Windup 総時間を記録し、時間に応じて均等に後退させる
    float dashBackstepMoved_     = 0.0f;  // 本次 Windup 後退済み距離（打ち切り用）
    // Ultimate（必殺技）: 発動前「初動後退」
    // 要件:のみ必殺技発動前（Windup 期間）後退1区間距離、その後入る Ultimate 左右来回ダッシュ。
    //    必殺技来回反射時以後は〜しない度额外後退。
    float ultimateWindupBackstepDist_      = 2.00f; // 必殺技発動前: 後退距離（通常より長い）
    float ultimateWindupBackstepMaxSpeed_  = 0.14f; // Windup 後退最大速度（60fps 基準の1フレーム移動量）
    float ultimateWindupBackstepMinSpeed_  = 0.04f; // Windup 後退最小速度
    float ultimateWindupTotal_             = 0.0f;  // 今回の必殺技 Windup の総時間を記録
    float ultimateWindupBackstepMoved_     = 0.0f;  // Windup 已後退距離（打ち切り用）

    // 前フレームのプレイヤーとの距離を記録（「引き撃ち検知」などに使える）
    float prevDistToPlayer_ = 1e9f;

    // 出招节奏控制
    float globalAttackCD_ = 0.0f;  // 任意攻撃最小間隔
    float meleeCD_ = 0.0f;  // 近接クールダウン
    float dashCD_ = 0.0f;  // ダッシュクールダウン
    float microDashCD_ = 0.0f;  // 密着小ダッシュ冷却（dashCD_ を消費しない）
    float rangedCD_ = 0.0f;  // 远程冷却
    float barrageCD_ = 0.0f;  // 回転弾幕クールダウン
    float slamCD_ = 0.0f;  // 叩きつけクールダウン
    float novaCD_ = 0.0f;  // 円形爆発クールダウン
    float ultimateCD_ = 3.0f;  // 必殺技クールダウン（開幕少し遅延を持たせる）
    int   ultimateBounces_ = 0;    // 必殺技: 反射回数

    // ---------- 调参区 ----------
    float moveSpeed_ = 0.18f;  // 追跡速度（按「1フレーム移動量」理解）
    float dashSpeed_ = 0.45f;  // ダッシュ速度（1フレーム移動量）
    float jumpVel_ = 0.62f;

    float detectRange_ = 18.0f;
    float idealRange_ = 4.0f;
    float meleeRange_ = 2.2f;
    float dashMinRange_ = 4.0f;
    float dashMaxRange_ = 10.0f;

    // 遠距離攻撃距離
    float rangedMinRange_ = 4.2f;
    float rangedMaxRange_ = 14.0f;

    // 近距離ダッシュ: プレイヤー接近時に即座にダッシュするが、連続で突進し続けない（dashCD_ で制御）
    float closeDashRange_ = 4.8f;
    float closeDashWindup_ = 0.12f;
    float closeDashDuration_ = 0.16f;
    float closeDashSpeed_ = 0.30f;  // 密着小ダッシュ速度（1フレーム移動量）
    float microDashCooldown_ = 1.20f;  // dashCD_ が無い時でも 1 回だけ小ダッシュを許可するためのクールダウン

    // 遠距離制圧: 距離がかなり離れている時は遠距離攻撃を優先
    float farRangedPrefer_ = 9.0f;

    // 必殺技: 左右往復ダッシュ（ブロック / トゲに当たると 1 回の「反射」とみなす）、終了後に休止
    float ultimateSpeed_ = 0.35f;
    float ultimateDuration_ = 5.0f; // failsafe: 最長持続
    int   ultimateMaxBounces_ = 4;    // 反射回数（4=左右往復 2 セット）
    float restDuration_ = 1.8f;

    float leadTime_ = 0.25f;           // 追跡予判: playerPos + playerVel * leadTime
    float projectileLeadTime_ = 0.35f; // 弾幕照準予判

    // ---------- Boss 弾幕（オブジェクトプール） ----------
    struct BossProjectile {
        std::unique_ptr<Object3d> obj;
        Vector3 pos{ 0,0,0 };
        Vector3 vel{ 0,0,0 };
        float   life = 0.0f;
        float   radius = 0.35f;
        bool    active = false;
    };

    // 弾幕が密なほど派手になるので、プール数を少し増やす
    static constexpr int kMaxBossProjectiles_ = 48;
    std::vector<BossProjectile> projectiles_;

    float projectileSpeed_ = 0.25f;  // 1フレーム移動量（跟 dashSpeed_ 同じスケール）
    float projectileLife_ = 2.20f;  // 秒

    // ---------- 追加: 派手な技のパラメータ ----------
    // ① 旋转弾幕（Barrage）: Boss を中心に回転しながら発射
    float barrageDuration_ = 1.35f; // 秒
    float barrageFireInterval_ = 0.055f; // 秒
    float barrageFireTimer_ = 0.0f;
    float barrageAngle_ = 0.0f;  // 弧度
    float barrageAngularSpeed_ = 7.0f;  // rad/sec
    float barrageSpinDir_ = 1.0f;  // +1/-1: 回転方向
    float barrageProjectileSpd_ = 0.22f; // 1フレーム移動量（projectileSpeed_ とは独立して調整可能）
    float barrageProjectileLife_ = 2.10f;

    // Barrage 中に追加の「爆発リング」を入れて画面をより派手にする
    float barrageBurstInterval_ = 0.38f;
    float barrageBurstTimer_ = 0.0f;
    int   barrageBurstCount_ = 10;
    float barrageBurstSpeed_ = 0.18f;
    float barrageBurstLife_ = 1.45f;

    // ①.5 円形爆発（Nova）: 多重リング弾幕
    float novaWindup_ = 0.26f;
    float novaDuration_ = 0.95f;
    float novaRecover_ = 0.85f;
    int   novaRingsNormal_ = 2;
    int   novaRingsEnrage_ = 3;
    float novaRingInterval_ = 0.16f;
    int   novaBulletCount_ = 18;
    float novaProjectileSpd_ = 0.26f;
    float novaProjectileLife_ = 1.65f;
    float novaRingOffset_ = 0.0f;
    float novaFireTimer_ = 0.0f;
    int   novaRingsLeft_ = 0;

    // ② 叩きつけ（Slam）: 跳び上がり後落地生成「左右の衝撃波 + 破片」
    float slamWindup_ = 0.32f;
    float slamJumpVel_ = 0.92f;
    float slamMaxAirTime_ = 1.60f; // failsafe
    float slamImpactHold_ = 0.18f; // 着地停止
    float slamRecover_ = 0.75f;
    float slamWaveSpeed_ = 0.38f;
    float slamWaveLife_ = 1.70f;
    float slamShardSpeed_ = 0.30f;
    float slamShardLife_ = 1.40f;
    bool  slamSpawned_ = false;

    // Shoot 状態内の連射
    int   shotsLeft_ = 0;
    int   shotsTotal_ = 0;
    float shotInterval_ = 0.16f;  // 秒
    float shotTimer_ = 0.0f;   // 秒（<=0 で発射）

    // ③ 追加: 扇状散布（Shoot 状態内の「オン / オフ」で実装し、追加状態は増やさない）
    bool  fanShot_ = false;
    int   fanCount_ = 5;      // 1 波あたりの弾数
    float fanHalfAngle_ = 0.52f;  // 半角（弧度）≈ 30°
    float fanProjectileSpd_ = 0.24f;

    bool ultimateLocked_ = false;     // ultimateCD_ を 399 まで引き上げたロック期間中かどうか

    float battleTriggerVerticalRange_ = 2.5f;
    // 汎用: リング弾幕用ツール（オブジェクトプールを再利用）
    void SpawnRadialBurst(const Vector3& center, int count, float speed, float life, float radius, float angleOffset = 0.0f);

private:
    void UpdatePreAttackJitter(float dt);

    void ResolveMapCollision(const MapChipField& map, float dt);
    void UpdateBossFacing(const Player& player);

    void UpdateBossProjectiles(float dt, const MapChipField& map);
    void SpawnBossProjectile(const Vector3& spawnPos, const Vector3& aimDir);
    void SpawnBossProjectileRaw(const Vector3& spawnPos, const Vector3& velocity, float life, float radius);
    void DrawBossProjectiles();

    void FinishUltimateCooldown();
};
