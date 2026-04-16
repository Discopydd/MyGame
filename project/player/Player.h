#pragma once
#include "Object3d.h"
#include "Input.h"
#include "map/MapChipField.h"

class Player {
public:
    Player();
    ~Player();

    void Initialize(Object3dCommon* object3dCommon, Camera* camera);
    void Update(Input* input, const MapChipField& mapChipField);
    void Draw();

    void HandleMapCollision(const MapChipField& mapChipField);

    void SetPosition(const Vector3& pos) {
        position_ = pos;
        if (model_) {
            model_->SetTranslate(position_);
            model_->Update();
        }
    }
    const Vector3& GetPosition() const { return position_; }

    bool IsDashing() const { return isDashing_; }
    float GetDashCooldown() const { return dashCooldownTimer_; }
    float GetDashCooldownDuration() const { return dashCooldown_; }
    bool CanDash() const { return canDash_; }
    float GetHeight() const { return height_; }

    float GetWidth()  const { return width_; }
    const Vector3& GetVelocity() const { return velocity_; }

    void SetVelocity(const Vector3& v) { velocity_ = v; }

    // 外部用（移動床など）の「特定の高さに着地させる」処理
    void LandOnExternalGround(float groundTopY);
    int GetMaxHp() const { return maxHP_; }
    // プレイヤーを「指定した Y 高さの地面上」に乗せる（足場 / ブロックのどちらでも使える）
    void SnapOnGround(float groundTopY);

    // 外部からの強制移動量（たとえば移動床に運ばれるとき）
    void ApplyExternalDisplacement(const Vector3& delta);


    void ResetForMapTransition(bool keepFacing = true);

    // --- HP APIs ---
    float  GetHpRatio()   const { return maxHP_ > 0 ? hp_ / (float)maxHP_ : 0.0f; }
    float  GetHP()        const { return hp_; }
    int    GetMaxHP()     const { return maxHP_; }
    void   SetHpDrainPerSec(float v) { hpDrainPerSec_ = (std::max)(0.0f, v); }
    void   TakeDamage(float v) { hp_ = (std::max)(0.0f, hp_ - v); }
    void   Heal(float v) { hp_ = (std::min)(hp_ + v, (float)maxHP_); }
    bool IsOnGround() const { return isOnGround_; }

     // 水関連の状態（任意: UI / エフェクト用）
    bool IsInWater() const { return inWater_; }
    bool IsOnWaterSurface() const { return onWaterSurface_; }

        bool  IsInvincible() const { return isInvincible_; }
    bool  IsDamageInvincible() const { return damageInvincibleTimer_ > 0.0f; } // 「被ダメージ無敵」のみ（点滅あり）
    void  StartInvincible(float duration);          // 被ダメージ無敵（点滅あり）
    void  StartDashInvincible(float duration);      // ダッシュ無敵（点滅なし）
    void  StartStompInvincible(float duration);     // 踏みつけ無敵（点滅なし）
    
    // 踏みつけクールダウン: Boss の頭上での無限連続踏みを防ぐため
    bool  IsStompCooldown() const { return stompCooldownTimer_ > 0.0f; }
    void  StartStompCooldown(float duration);

    // 踏みつけノックバック: 踏んだ後の横方向への弾きを短時間だけ安定して効かせる（通常の空中操作感には影響しない）
    void  StartStompKick(float vx, float duration);

    bool  IsDead() const { return isDead_; }
    void  StartDeathFall();   // トリガー死亡演出（GameScene から呼び出す）

    bool ConsumeDoubleJumpEvent() {
        bool v = didDoubleJumpThisFrame_;
        didDoubleJumpThisFrame_ = false;
        return v;
    }
    // プレイヤーが右を向いているかどうか
    bool IsFacingRight() const { return lrDirection_ == LRDirection::kRight; }

    const Vector3& GetDashDirection() const { return dashDirection_; }
private:
    std::unique_ptr<Object3d> model_;
    Object3dCommon* object3dCommon_ = nullptr;
    Camera* camera_ = nullptr;

    Vector3 position_ = { 0, 0, 0 };
    Vector3 velocity_ = { 0, 0, 0 };

    // ===== Movement =====
    float moveSpeed_ = 0.25f;

    // ===== Jump tuning =====
    // ジャンプ開始時に初速度を与え、長押し中は一定時間だけ上向き加速度を追加し、上昇速度には上限を設ける
    float jumpVelInit_ = 0.56f;   // ジャンプ開始初速度
    float jumpVelMax_ = 1.05f;   // 上昇速度上限
    float jumpHoldAccel_ = 1.5f;   // 長押し時の追加上向き加速度（単位: ワールド単位 / 秒^2）
    float maxJumpHoldTimeFirst_ = 0.15f;   // 長押しが有効な最大時間（秒）
    float maxJumpHoldTimeSecond_ = 0.13f;   // 2段ジャンプで溜められる時間
    bool  isOnGround_ = false;
    bool  isJumping_ = false;   // 「制御可能な上昇」状態（hold 窓内で、まだ上昇中）
    float jumpPressDuration_ = 0.0f;   // 長押ししている時間（秒）

    // 現在のジャンプで実際に使う最大溜め時間（何段目かに応じて設定）
    float currentMaxJumpHoldTime_ = 0.0f;
    // 可変重力: 「早離し / 落下」をよりキビキビさせる
    float gravityBase_ = -2.20f; // 基本重力（下向きは負）
    float lowJumpGravityScale_ = 1.60f;  // 早離ししても上昇中のときの追加下向き倍率
    float fallGravityScale_ = 2.20f;  // 落下中の追加下向き倍率

    // 向き / 旋回
    enum class LRDirection { kRight, kLeft };
    LRDirection lrDirection_ = LRDirection::kRight;
    float turnStartRotationY_ = 0;
    float turnTargetRotationY_ = 0;
    int   turnCurrentFrame_ = 0;
    const int turnTotalFrames_ = 10;
    float currentRotationY_ = 0.0f;

    // 当たり判定サイズ
    float width_ = 1.5f;
    float height_ = 1.5f;

    // ===== Dash =====
    bool  isDashing_ = false;
    float dashSpeedMultiplier_ = 2.0f;
    float dashDuration_ = 0.22f;      // 秒
    float dashTimer_ = 0.0f;
    float dashCooldown_ = 1.5f;      // 秒
    float dashCooldownTimer_ = 0.0f;
    bool  canDash_ = true;
    const float dashCooldownThreshold_ = 0.1f;
    Vector3 dashDirection_ = { 1, 0, 0 };

    bool  isDashJumping_ = false;       // 空中ダッシュ: 無重力で移動する
    float dashGravity_ = 0.0f;          // 互換性維持用のみ（Update 内では isDashJumping_ で g=0 を制御）

    // 頭上衝突後の短いロック
    float headBonkTimer_ = 0.0f;
    static inline const float kHeadBonkLock_ = 0.05f;

    // --- HP members ---
    int   maxHP_ = 100;
    float hp_ = 100.0f;
    float hpDrainPerSec_ = 5.0f;   // 毎秒 5 減少

    // 旧インターフェース互換用（現在は毎フレームの重力値を直接使わないが、復帰ロジックは残す）
    float originalGravity_ = -2.20f; // gravityBase_ に合わせる

    // ---- Death (GameOver) ----
    bool  isDead_ = false;
    bool  deathStarted_ = false;
    float deathTimer_ = 0.0f;
    float deathJumpVel_ = 0.78f;     // 小さく跳ねる初速度（既存単位系に合わせ、そのまま velocity_.y に加える）
    float deathRotateTime_ = 0.35f;  // 倒立するまでの回転時間（秒）
    float deathExtraGravScale_ = 2.2f; // 死亡落下時の重力倍率
    float deathSpinZ_ = 0.0f;        // 倒立時の Z 回転角（0→π）

    // ---- Jump / Double Jump ----
    int   jumpCount_ = 0;        // 使用済みのジャンプ回数
    int   maxJumpCount_ = 2;     // 最大ジャンプ回数（2 = 2段ジャンプ）


    bool  didDoubleJumpThisFrame_ = false; // このフレームで 2段ジャンプが発動したかどうか
    // --- 無敵（被ダメージ / ダッシュ / 踏みつけ）と点滅（被ダメージ無敵のみ） ---
    bool  isInvincible_ = false;

    // 被ダメージ無敵: 点滅あり（被ダメージ後 1 秒無敵など）
    float damageInvincibleTimer_ = 0.0f;

    // ダッシュ無敵: ダッシュ中は無敵（点滅なし）
    float dashInvincibleTimer_ = 0.0f;

    // 踏みつけ無敵: 敵を踏んだ後の短時間無敵（点滅なし）
    float stompInvincibleTimer_ = 0.0f;


    // 踏みつけクールダウン: Boss の頭上での無限連続踏みを防ぐ
    float stompCooldownTimer_ = 0.0f;

    // 踏みつけノックバック: 短時間だけ水平速度を上書き / 混合し、「棒立ち踏み」を防ぎつつ自然な手触りにする
    float stompKickTimer_ = 0.0f;
    float stompKickTotal_ = 0.0f;
    float stompKickVx_    = 0.0f;
    // 点滅（damageInvincibleTimer_ > 0 のときのみ有効）
    float damageBlinkTimer_ = 0.0f;
    float damageBlinkInterval_ = 0.08f; // 点滅間隔（秒）
    bool  damageBlinkVisible_ = true;
     // ---- 水状態 ----
    bool inWater_ = false;        // 水ブロック内部にいるかどうか（S を押すと潜る）
    bool onWaterSurface_ = false; // 水面に浮いているかどうか
};
