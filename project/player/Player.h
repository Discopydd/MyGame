#pragma once
#include "Object3d.h"
#include "Input.h"
#include "map/MapChipField.h"

/// <summary>
/// プレイヤーの入力、移動、ジャンプ、ダッシュ、HP、マップ衝突を管理するクラスです。
/// </summary>
class Player {
public:
    /// <summary>
    /// Playerのインスタンスを生成します。
    /// </summary>
    Player();
    /// <summary>
    /// Playerが保持するリソースを破棄します。
    /// </summary>
    ~Player();

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="object3dCommon">3Dオブジェクト生成に使用する共通描画処理。</param>
    /// <param name="camera">描画および座標変換に使用するカメラ。</param>
    void Initialize(MyEngine::Object3dCommon* object3dCommon, MyEngine::Camera* camera);
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    /// <param name="input">入力状態を取得する入力管理オブジェクト。</param>
    /// <param name="mapChipField">地形情報と衝突判定に使用するマップデータ。</param>
    /// <param name="snowWindActive">雪風エリアの影響を有効にする場合は true。</param>
    void Update(MyEngine::Input* input, const MapChipField& mapChipField, bool snowWindActive = false);
    /// <summary>
    /// 現在の状態を画面へ描画します。
    /// </summary>
    void Draw();

    /// <summary>
    /// プレイヤーとマップチップの衝突を判定し、位置と速度を補正します。
    /// </summary>
    /// <param name="mapChipField">地形情報と衝突判定に使用するマップデータ。</param>
    void HandleMapCollision(const MapChipField& mapChipField);

    /// <summary>
    /// プレイヤーのワールド座標を設定し、描画モデルへ反映します。
    /// </summary>
    /// <param name="pos">設定する座標。</param>
    void SetPosition(const MyEngine::Vector3& pos) {
        position_ = pos;
        if (model_) {
            model_->SetTranslate(position_);
            model_->Update();
        }
    }
    /// <summary>
    /// 現在のワールド座標を取得します。
    /// </summary>
    /// <returns>保持している値への参照。</returns>
    const MyEngine::Vector3& GetPosition() const { return position_; }

    /// <summary>
    /// 現在ダッシュ中かを取得します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool IsDashing() const { return isDashing_; }
    /// <summary>
    /// ダッシュ再使用までの残り時間を取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    float GetDashCooldown() const { return dashCooldownTimer_; }
    /// <summary>
    /// ダッシュのクールダウン総時間を取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    float GetDashCooldownDuration() const { return dashCooldown_; }
    /// <summary>
    /// 現在ダッシュを開始できるかを判定します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool CanDash() const { return canDash_; }
    /// <summary>
    /// 当たり判定の高さを取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    float GetHeight() const { return height_; }

    /// <summary>
    /// 当たり判定の幅を取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    float GetWidth()  const { return width_; }
    /// <summary>
    /// 現在の移動速度を取得します。
    /// </summary>
    /// <returns>保持している値への参照。</returns>
    const MyEngine::Vector3& GetVelocity() const { return velocity_; }

    /// <summary>
    /// 移動速度を設定します。
    /// </summary>
    /// <param name="v">設定または計算に使用する値。</param>
    void SetVelocity(const MyEngine::Vector3& v) { velocity_ = v; }

    // 外部用（移動床など）の「特定の高さに着地させる」処理
    /// <summary>
    /// 移動床など外部の足場上へ着地させ、接地状態を更新します。
    /// </summary>
    /// <param name="groundTopY">足場または地面上面のY座標。</param>
    void LandOnExternalGround(float groundTopY);
    /// <summary>
    /// 最大HPを取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    int GetMaxHp() const { return maxHP_; }
    // プレイヤーを「指定した Y 高さの地面上」に乗せる（足場 / ブロックのどちらでも使える）
    /// <summary>
    /// 指定された地面の高さにプレイヤーを正確に配置します。
    /// </summary>
    /// <param name="groundTopY">足場または地面上面のY座標。</param>
    void SnapOnGround(float groundTopY);

    // 外部からの強制移動量（たとえば移動床に運ばれるとき）
    /// <summary>
    /// 移動床などによる外部からの移動量をプレイヤーへ適用します。
    /// </summary>
    /// <param name="delta">外部から加える移動量。</param>
    void ApplyExternalDisplacement(const MyEngine::Vector3& delta);


    /// <summary>
    /// マップ遷移後に継続可能な状態へプレイヤーをリセットします。
    /// </summary>
    /// <param name="keepFacing">現在の向きを維持する場合は true。</param>
    void ResetForMapTransition(bool keepFacing = true);

    // --- HP APIs ---
    /// <summary>
    /// 最大HPに対する現在HPの割合を取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    float  GetHpRatio()   const { return maxHP_ > 0 ? hp_ / (float)maxHP_ : 0.0f; }
    /// <summary>
    /// 現在HPを取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    float  GetHP()        const { return hp_; }
    /// <summary>
    /// 最大HPを取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    int    GetMaxHP()     const { return maxHP_; }
    /// <summary>
    /// 1秒あたりのHP減少量を設定します。
    /// </summary>
    /// <param name="v">設定または計算に使用する値。</param>
    void   SetHpDrainPerSec(float v) { hpDrainPerSec_ = (std::max)(0.0f, v); }
    /// <summary>
    /// 指定量のダメージをHPへ反映します。
    /// </summary>
    /// <param name="v">設定または計算に使用する値。</param>
    void   TakeDamage(float v) { hp_ = (std::max)(0.0f, hp_ - v); }
    /// <summary>
    /// 指定量のHPを回復します。
    /// </summary>
    /// <param name="v">設定または計算に使用する値。</param>
    void   Heal(float v) { hp_ = (std::min)(hp_ + v, (float)maxHP_); }
    /// <summary>
    /// プレイヤーが接地しているかを取得します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool IsOnGround() const { return isOnGround_; }

     // 水関連の状態（任意: UI / エフェクト用）
    /// <summary>
    /// プレイヤーが水中にいるかを取得します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool IsInWater() const { return inWater_; }
    /// <summary>
    /// プレイヤーが水面上にいるかを取得します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool IsOnWaterSurface() const { return onWaterSurface_; }

        /// <summary>
        /// 何らかの無敵状態が有効かを取得します。
        /// </summary>
        /// <returns>条件を満たす場合は true、それ以外は false。</returns>
        bool  IsInvincible() const { return isInvincible_; }
    /// <summary>
    /// 被ダメージ後の無敵時間が有効かを取得します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool  IsDamageInvincible() const { return damageInvincibleTimer_ > 0.0f; } // 「被ダメージ無敵」のみ（点滅あり）
    /// <summary>
    /// 被ダメージ用の無敵状態を指定時間開始します。
    /// </summary>
    /// <param name="duration">継続時間（秒）。</param>
    void  StartInvincible(float duration);          // 被ダメージ無敵（点滅あり）
    /// <summary>
    /// ダッシュ中の無敵状態を指定時間開始します。
    /// </summary>
    /// <param name="duration">継続時間（秒）。</param>
    void  StartDashInvincible(float duration);      // ダッシュ無敵（点滅なし）
    /// <summary>
    /// 踏みつけ後の無敵状態を指定時間開始します。
    /// </summary>
    /// <param name="duration">継続時間（秒）。</param>
    void  StartStompInvincible(float duration);     // 踏みつけ無敵（点滅なし）
    
    // 踏みつけクールダウン: Boss の頭上での無限連続踏みを防ぐため
    /// <summary>
    /// 踏みつけの再判定を防ぐクールダウン中かを取得します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool  IsStompCooldown() const { return stompCooldownTimer_ > 0.0f; }
    /// <summary>
    /// 踏みつけ判定のクールダウンを指定時間開始します。
    /// </summary>
    /// <param name="duration">継続時間（秒）。</param>
    void  StartStompCooldown(float duration);

    // 踏みつけノックバック: 踏んだ後の横方向への弾きを短時間だけ安定して効かせる（通常の空中操作感には影響しない）
    /// <summary>
    /// 踏みつけ後の水平ノックバックを指定時間開始します。
    /// </summary>
    /// <param name="vx">X方向の初速度。</param>
    /// <param name="duration">継続時間（秒）。</param>
    void  StartStompKick(float vx, float duration);

    /// <summary>
    /// 死亡状態かを取得します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool  IsDead() const { return isDead_; }
    /// <summary>
    /// ゲームオーバー用の死亡落下演出を開始します。
    /// </summary>
    void  StartDeathFall();   // トリガー死亡演出（GameScene から呼び出す）

    /// <summary>
    /// このフレームに発生した二段ジャンプイベントを取得し、消費済みにします。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool ConsumeDoubleJumpEvent() {
        bool v = didDoubleJumpThisFrame_;
        didDoubleJumpThisFrame_ = false;
        return v;
    }
    // プレイヤーが右を向いているかどうか
    /// <summary>
    /// プレイヤーが右方向を向いているかを取得します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool IsFacingRight() const { return lrDirection_ == LRDirection::kRight; }

    /// <summary>
    /// 現在のダッシュ方向を取得します。
    /// </summary>
    /// <returns>保持している値への参照。</returns>
    const MyEngine::Vector3& GetDashDirection() const { return dashDirection_; }
private:
    std::unique_ptr<MyEngine::Object3d> model_;
    MyEngine::Object3dCommon* object3dCommon_ = nullptr;
    MyEngine::Camera* camera_ = nullptr;

    MyEngine::Vector3 position_ = { 0, 0, 0 };
    MyEngine::Vector3 velocity_ = { 0, 0, 0 };

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
    MyEngine::Vector3 dashDirection_ = { 1, 0, 0 };

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
