#include "Player.h"
#include "ModelManager.h"
#include <algorithm>
#include <numbers>
#include <cmath>

Player::Player() {}

Player::~Player() {
}

void Player::Initialize(Object3dCommon* object3dCommon, Camera* camera) {
    object3dCommon_ = object3dCommon;
    camera_ = camera;

    model_ = std::make_unique<Object3d>();
    model_->Initialize(object3dCommon_);
    model_->SetModel("player/player.obj");
    model_->SetCamera(camera_);


    position_ = { -10, 0, 0 };
    velocity_ = { 0, 0, 0 };

    // Dash の初期状態
    isDashing_ = false;
    dashTimer_ = 0.0f;
    dashCooldownTimer_ = 0.0f;
    canDash_ = true;
    dashDirection_ = { 1, 0, 0 };

    // 旧来の originalGravity_ を現在のパラメータに合わせる
    originalGravity_ = gravityBase_;
    dashGravity_ = 0.0f;

    stompCooldownTimer_ = 0.0f;

    stompKickTimer_ = 0.0f;
    stompKickTotal_ = 0.0f;
    stompKickVx_ = 0.0f;
}

void Player::Update(Input* input, const MapChipField& mapChipField, bool snowWindActive) {
    const float deltaTime = 1.0f / 60.0f; // とりあえず 60FPS 固定。必要なら後で外部から渡せる
    didDoubleJumpThisFrame_ = false;

   // 1フレーム前の水状態を記録し、ちょうど水に入った瞬間を判定する
    bool wasInWater        = inWater_;
    bool wasOnWaterSurface = onWaterSurface_;

    // 毎フレームいったん非水中状態にし、後でマップに基づいて再計算する
    inWater_ = false;
    onWaterSurface_ = false;
         // === 無敵時間（被ダメージ / ダッシュ / 踏みつけ）と点滅（被ダメージ無敵のみ） ===
    // ① 被ダメージ無敵: 点滅あり
    if (damageInvincibleTimer_ > 0.0f) {
        damageInvincibleTimer_ -= deltaTime;

        damageBlinkTimer_ += deltaTime;
        // damageBlinkInterval_ ごとに表示状態を反転する
        if (damageBlinkTimer_ >= damageBlinkInterval_) {
            damageBlinkTimer_ = 0.0f;
            damageBlinkVisible_ = !damageBlinkVisible_;
        }

        // 無敵時間終了
        if (damageInvincibleTimer_ <= 0.0f) {
            damageInvincibleTimer_ = 0.0f;
            damageBlinkTimer_ = 0.0f;
            damageBlinkVisible_ = true;
        }
    }

    // ② ダッシュ無敵: 点滅なし
    if (dashInvincibleTimer_ > 0.0f) {
        dashInvincibleTimer_ -= deltaTime;
        if (dashInvincibleTimer_ < 0.0f) dashInvincibleTimer_ = 0.0f;
    }

    // ③ 踏みつけ無敵: 点滅なし
    if (stompInvincibleTimer_ > 0.0f) {
        stompInvincibleTimer_ -= deltaTime;
        if (stompInvincibleTimer_ < 0.0f) stompInvincibleTimer_ = 0.0f;
    }


    // ④ 踏みつけクールダウン: Boss の頭上での無限連続踏みを防ぐ
    if (stompCooldownTimer_ > 0.0f) {
        stompCooldownTimer_ -= deltaTime;
        if (stompCooldownTimer_ < 0.0f) stompCooldownTimer_ = 0.0f;
    }

    // ⑤ 踏みつけノックバック時間: 踏みつけ後の短時間だけ有効にする（日常の空中操作感に影響させない）
    if (stompKickTimer_ > 0.0f) {
        stompKickTimer_ -= deltaTime;
        if (stompKickTimer_ < 0.0f) stompKickTimer_ = 0.0f;
    }
    isInvincible_ = (damageInvincibleTimer_ > 0.0f) ||
                    (dashInvincibleTimer_ > 0.0f) ||
                    (stompInvincibleTimer_ > 0.0f);

    // --- HP を毎秒減少させる ---
    if (hpDrainPerSec_ > 0.0f && hp_ > 0.0f) {
        //hp_ -= hpDrainPerSec_ * deltaTime;
        if (hp_ < 0.0f) hp_ = 0.0f;
    }
    if (hp_ <= 0.0f && !deathStarted_) {
        StartDeathFall();
    }
    // 頭上衝突後の短いロック
    headBonkTimer_ = (std::max)(0.0f, headBonkTimer_ - deltaTime);
    if (isDead_) {
        const float dt = 1.0f / 60.0f;

        // 倒立するまで回転させる（Z 軸 0→π）
        deathTimer_ += dt;
        float t = (std::min)(1.0f, deathTimer_ / deathRotateTime_);
        deathSpinZ_ = DirectX::XM_PI * (t * t * (3.0f - 2.0f * t)); // smoothstep

        // 重力を強めて落下させる。跳ね上がり後の最初のフレームではすでに上向き初速を与えている
        float g = gravityBase_ * deathExtraGravScale_;
        velocity_.y += g * dt;

        // フレームごとに移動し、**衝突を無視して** そのまま画面外へ落とす
        position_.x += velocity_.x;
        position_.y += velocity_.y;
        isOnGround_ = false;

        // 回転を適用する。左右向きの Y 回転は維持し、Z のみ 180° にする
        Vector3 rot = { 0.0f, currentRotationY_, deathSpinZ_ };
        model_->SetRotate(rot);
        model_->SetTranslate(position_);
        model_->Update();
        return; // 死亡状態は独立処理なのでここで終了
    }
    // ========== 入力なし（Intro など）のときは物理のみ更新 ==========
    if (input == nullptr) {
        // 可変重力のみに基づいて更新する
        float g = gravityBase_;
        if (!isOnGround_ && velocity_.y <= 0.0f) {
            g *= fallGravityScale_;
        }
        if (isDashJumping_) g = 0.0f;

        velocity_.y += g * deltaTime;

        HandleMapCollision(mapChipField);
        model_->SetTranslate(position_);
        model_->Update();
        return;
    }

    // ========= Dash 状態更新 =========
    if (isDashing_) {
        dashTimer_ += deltaTime;
        if (dashTimer_ >= dashDuration_) {
            isDashing_ = false;
            dashTimer_ = 0.0f;
            dashCooldownTimer_ = dashCooldown_;
            canDash_ = false;

            // 空中ダッシュを終了し、通常の重力制御に戻す
            if (isDashJumping_) {
                isDashJumping_ = false;
            }
        }
    }
    if (!canDash_) {
        dashCooldownTimer_ -= deltaTime;
        if (dashCooldownTimer_ <= dashCooldownThreshold_) {
            dashCooldownTimer_ = 0.0f;
            canDash_ = true;
        }
    }
            // ========= 水ブロック判定 =========
    {
        const float halfW = width_ * 0.5f;
        const float halfH = height_ * 0.5f;

        float left = position_.x - halfW;
        float right = position_.x + halfW;
        float bottom = position_.y - halfH;
        float top = position_.y + halfH;

        const float kWaterProbeOffset = 0.05f;
        auto minIdx = mapChipField.GetMapChipIndexByPosition({ left,  bottom - kWaterProbeOffset, 0 });
        auto maxIdx = mapChipField.GetMapChipIndexByPosition({ right, top,                        0 });

        bool  hasWater = false;
        float waterSurfaceY = 0.0f;

        for (uint32_t y = minIdx.yIndex; y <= maxIdx.yIndex; ++y) {
            for (uint32_t x = minIdx.xIndex; x <= maxIdx.xIndex; ++x) {
                if (mapChipField.GetMapChipTypeByIndex(x, y) != MapChipType::kWater) {
                    continue;
                }
                auto r = mapChipField.GetRectByIndex(x, y);
                if (!hasWater || r.top > waterSurfaceY) {
                    hasWater = true;
                    waterSurfaceY = r.top;
                }
            }
        }

        // いったん水中でも水面でもない状態にし、以下の判定結果で更新する
        inWater_ = false;
        onWaterSurface_ = false;

        if (hasWater) {
            float depth = waterSurfaceY - bottom;
            const float kSurfaceSnapThreshold = height_ * 0.5f;
            // この落下速度以下なら「ゆっくり水面に接触した」とみなし、浮遊させる
            const float kFloatMaxFallSpeed = 0.15f;   // 必要に応じて調整可能

            const bool diveInput =
                input->PushKey(DIK_S) || input->PushKey(DIK_DOWN);

            if (depth > 0.0f) {
                // 垂直速度の絶対値が小さい（上昇でも下降でも）場合は「そっと水面に触れた」とみなす
                bool canFloatOnSurface =
                    (std::fabs(velocity_.y) <= kFloatMaxFallSpeed);

                if (!diveInput &&
                    depth <= kSurfaceSnapThreshold &&
                    canFloatOnSurface) {

                    // ★ 水面に浮かせる: 体の半分が水中に入る位置を保つ
                    SnapOnGround(waterSurfaceY - halfH);
                    onWaterSurface_ = true;
                    inWater_ = false;
                }
                else {
                    // ★ 速度が速い（高所から落下）または S を押している場合は、本当に水中へ入ったとみなす
                    inWater_ = true;
                    onWaterSurface_ = false;
                }
            }
            // depth <= 0: 足が水面より上なので空中扱い
        }
    }


       // === 空中 / 水面からちょうど水中へ入ったフレームでは、固定の入水速度を与える ===
    bool justEnteredWater = (!wasInWater && !wasOnWaterSurface && inWater_);
    if (justEnteredWater) {

        // 今回空中に出た際に何段ジャンプを使ったかを判定する
        // 0: 落下のみ  1: 通常ジャンプ  2: 2段ジャンプ
        int usedJumps = jumpCount_;
        if (usedJumps < 0) usedJumps = 0;
        if (usedJumps > 2) usedJumps = 2;

        // 基本の入水速度（空中から水中へ入る際に少し下へ沈む）
        const float kBaseEnterSinkSpeed = -0.10f; // 単位: ワールド単位 / フレーム程度の感覚

        // 2段ジャンプ時は少し深く沈める（段数が多いほどわずかに深く入水）
        const float kExtraSinkPerExtraJump = -0.05f; // 1段多いごとにこの分だけ深く沈む

        // 目標の入水速度を計算する:
        // - 落下 / 1段ジャンプ:  -0.10
        // - 2段ジャンプで入水: -0.15
        float targetVy = kBaseEnterSinkSpeed
                       + (float)(usedJumps - 1) * kExtraSinkPerExtraJump;

        // 上向きの速度は許可せず、必ず少し下へ沈むようにする
        if (targetVy >= 0.0f) {
            targetVy = -0.05f;
        }

        velocity_.y = targetVy;
    }
    // トリガーダッシュ（左 Shift）
    if (input->TriggerKey(DIK_LSHIFT) && canDash_ && !isDashing_) {
        isDashing_ = true;
        dashTimer_ = 0.0f;
        dashCooldownTimer_ = dashCooldown_;


        // ダッシュ中は無敵（点滅なし）
        StartDashInvincible(dashDuration_);
        // ダッシュ方向を決定する
        if (lrDirection_ == LRDirection::kRight)  dashDirection_ = { 1, 0, 0 };
        else                                      dashDirection_ = { -1, 0, 0 };

        // 空中ダッシュ: 無重力で移動する
        if (!isOnGround_) {
            isDashJumping_ = true;
            velocity_.y = 0.0f;   // 上昇を即座に打ち切る
            isJumping_ = false;   // 制御可能な上昇を終了する
            jumpPressDuration_ = currentMaxJumpHoldTime_; // 長押し可能な時間窓を閉じる
        }
    }

    // ========= 水平移動 =========
    Vector3 moveInput = { 0, 0, 0 };
    if (input->PushKey(DIK_D) || input->PushKey(DIK_RIGHT)) {
        moveInput.x = 1.0f;
        if (lrDirection_ != LRDirection::kRight) {
            lrDirection_ = LRDirection::kRight;
            turnStartRotationY_ = currentRotationY_;
            turnTargetRotationY_ = 0.0f;
            turnCurrentFrame_ = 0;
        }
    } else if (input->PushKey(DIK_A) || input->PushKey(DIK_LEFT)) {
        moveInput.x = -1.0f;
        if (lrDirection_ != LRDirection::kLeft) {
            lrDirection_ = LRDirection::kLeft;
            turnStartRotationY_ = currentRotationY_;
            turnTargetRotationY_ = DirectX::XM_PI;
            turnCurrentFrame_ = 0;
        }
    } 

    // ---- 水中物理の倍率（操作感に応じて調整可能）----
    const float kWaterMoveSpeedScale  = 0.4f;  // 水中移動速度 40%
    const float kWaterDashSpeedScale  = 0.5f;  // 水中ダッシュ速度 50%
    const float kWaterJumpVelScale    = 0.2f;  // 水中ジャンプ初速 20%
    const float kWaterGravityScale    = 0.18f; // 水中重力 18%

    float currentMoveSpeed = moveSpeed_;
    if (inWater_) {
        currentMoveSpeed *= kWaterMoveSpeedScale;
    }
    Vector3 moveVelocity = moveInput * currentMoveSpeed;

    if (isDashing_) {
        // ダッシュ中は固定速度で移動し、入力はほぼ無視する
        float dashSpeed = moveSpeed_ * dashSpeedMultiplier_;
        if (inWater_) {
            dashSpeed *= kWaterDashSpeedScale;
        }
        moveVelocity = dashDirection_ * dashSpeed;
        if (isDashJumping_) {
            moveVelocity.y = 0.0f;
        }
    }

    // ===== Snow stage wind =====
    // 右から左へ吹く風: 静止中は左へゆっくり流され、右移動には抵抗がかかる。
    // moveSpeed_ は「1フレームあたりの移動量」なので、数値は小さめにして操作感を崩しすぎないようにする。
    if (snowWindActive) {
        const float kSnowIdleDrift       = -0.035f; // 無入力時に左へ流される量
        const float kSnowRightResistance =  0.075f; // 右移動速度から引く量
        const float kSnowLeftAssist      = -0.020f; // 左移動時の軽い追い風
        const float kSnowDashScale       =  0.45f;  // ダッシュ中は風の影響を弱める

        float scale = isDashing_ ? kSnowDashScale : 1.0f;

        if (moveVelocity.x > 0.0f) {
            moveVelocity.x = (std::max)(0.0f, moveVelocity.x - kSnowRightResistance * scale);
        }
        else if (moveVelocity.x < 0.0f) {
            moveVelocity.x += kSnowLeftAssist * scale;
        }
        else {
            moveVelocity.x += kSnowIdleDrift * scale;
        }
    }

    float finalVx = moveVelocity.x;

    // 踏みつけノックバック: 短時間だけ安定して効かせ、ease-out で自然に弾く
    if (stompKickTimer_ > 0.0f) {
        float denom = (stompKickTotal_ > 0.0f) ? stompKickTotal_ : 1.0f;
        float r = stompKickTimer_ / denom; // 1 -> 0
        float ease = r * r;                // ease-out
        finalVx = stompKickVx_ * ease + moveVelocity.x * 0.25f; // 少量の空中補正を許可
    }

    velocity_.x = finalVx;

  const bool isSpacePressed   = input->PushKey(DIK_SPACE);
    const bool justPressedJump  = input->TriggerKey(DIK_SPACE);

    // ジャンプ開始 & 2段ジャンプ / 水中無限ジャンプ
    if (justPressedJump) {

        // ① 地上からジャンプ（ジャンプ回数をリセット）
        if (isOnGround_) {
            isOnGround_ = false;

            isJumping_ = true;
            jumpPressDuration_ = 0.0f;
            currentMaxJumpHoldTime_ = maxJumpHoldTimeFirst_;

            velocity_.y = jumpVelInit_;
            // 水面からのジャンプも少し重めにできる（不要ならこの行は外せる）
            if (inWater_) {
                velocity_.y *= kWaterJumpVelScale;
            }
            jumpCount_ = 1; // すでに 1 段目を使用済み

            if (isDashing_) {
                velocity_.x = dashDirection_.x * (moveSpeed_ * dashSpeedMultiplier_);
            }
        }
        // ② 空中ジャンプ:
        //  - 通常時: 最大 maxJumpCount_ 回まで（2段ジャンプ）
        //  - inWater_ == true のとき: 回数制限なし → 無限ジャンプ
        else if (!isOnGround_ && (jumpCount_ < maxJumpCount_ || inWater_)) {

            isJumping_ = true;
            jumpPressDuration_ = 0.0f;
            currentMaxJumpHoldTime_ = maxJumpHoldTimeSecond_;

            // ダッシュを中断して挙動の暴走を防ぐ
            isDashing_ = false;
            isDashJumping_ = false;

            velocity_.y = jumpVelInit_;
            if (inWater_) {
                velocity_.y *= kWaterJumpVelScale;
            }

            // jumpCount_ が無制限に増えないよう、上限未満のときだけ +1 する
            if (jumpCount_ < maxJumpCount_) {
                jumpCount_++;
            }

            didDoubleJumpThisFrame_ = true; // 水中の追加ジャンプも「2段ジャンプ発生」として扱い、演出を出しやすくする
        }
    }

    // 長押し時の追加加速: 「制御可能な上昇」ウィンドウ内かつ上昇中のみ有効
    if (isJumping_) {
        if (isSpacePressed && jumpPressDuration_ < currentMaxJumpHoldTime_) {
            jumpPressDuration_ += deltaTime;

            if (velocity_.y > 0.0f) {
                velocity_.y += jumpHoldAccel_ * deltaTime;
                if (velocity_.y > jumpVelMax_) {
                    velocity_.y = jumpVelMax_;
                }
            }
        } else {
            isJumping_ = false; // ウィンドウ終了またはボタンを離した
        }
    }

      // 可変重力
    float g = gravityBase_;

    if (inWater_) {
        // ★ 水中では基礎重力 × 水中倍率のみを使い、追加の落下重力 / 早離し重力は重ねない
        g *= kWaterGravityScale;
    } else {
        // ★ 空中ではじめて「早離し / 落下を重くする」ロジックを使う
        // 早離し（制御上昇終了後もまだ上昇中）→ 重力を強めて短いジャンプにする
        if (!isOnGround_ && velocity_.y > 0.0f && !isJumping_) {
            g *= lowJumpGravityScale_;
        }
        // 落下中 → より重くする
        if (!isOnGround_ && velocity_.y <= 0.0f) {
            g *= fallGravityScale_;
        }
    }

    // 水面に浮いている間 → 重力の影響を受けない
    if (onWaterSurface_) {
        g = 0.0f;
    }

    // 空中ダッシュ中 → 無重力（最優先）
    if (isDashing_) {
        g = 0.0f;
    }

    // 重力を適用
    velocity_.y += g * deltaTime;
    // ★ 水中: 最大沈下速度を制限し、どんどん速く落ちないようにする
    if (inWater_ && !onWaterSurface_) {
        const float kMaxSinkSpeed = -0.20f; // 1 フレームあたりの最大沈下速度（調整可）
        if (velocity_.y < kMaxSinkSpeed) {
            velocity_.y = kMaxSinkSpeed;
        }
    }
    // ========= 衝突補正 =========
    HandleMapCollision(mapChipField);

    // ========= 旋回補間 =========
    if (turnCurrentFrame_ < turnTotalFrames_) {
        turnCurrentFrame_++;
        float t = static_cast<float>(turnCurrentFrame_) / turnTotalFrames_;
        float easedT = t * t * (3.0f - 2.0f * t);
        currentRotationY_ = turnStartRotationY_ + (turnTargetRotationY_ - turnStartRotationY_) * easedT;
        model_->SetRotate({ 0.0f, currentRotationY_, 0.0f });
    }

    // ========= モデル更新 =========
    model_->SetTranslate(position_);
    model_->Update();
}

void Player::Draw() {
    if (!model_) { return; }

    // 無敵中は「特定フレームを描画しない」ことで点滅を表現する
    if (isInvincible_ && !damageBlinkVisible_) {
        return;
    }

    model_->Draw();
}

void Player::HandleMapCollision(const MapChipField& mapChipField) {
    const float kHalfW = width_ / 2.0f;
    const float kHalfH = height_ / 2.0f;

    bool onGround = false;

    // ---- 先に X を処理 ----
    {
        float nextX = position_.x + velocity_.x;

        float left   = nextX - kHalfW;
        float right  = nextX + kHalfW;
        float bottom = position_.y - kHalfH;
        float top    = position_.y + kHalfH;

        auto minIdx = mapChipField.GetMapChipIndexByPosition({ left,  bottom, 0 });
        auto maxIdx = mapChipField.GetMapChipIndexByPosition({ right, top,    0 });

        bool hitX = false;
        float fixX = nextX;

        for (uint32_t y = minIdx.yIndex; y <= maxIdx.yIndex; ++y) {
            for (uint32_t x = minIdx.xIndex; x <= maxIdx.xIndex; ++x) {
                if (mapChipField.GetMapChipTypeByIndex(x, y) != MapChipType::kBlock && mapChipField.GetMapChipTypeByIndex(x, y) != MapChipType::kBlock2) continue;
                auto r = mapChipField.GetRectByIndex(x, y);

                bool overlapY = !(top <= r.bottom || bottom >= r.top);
                bool overlapX = !(right <= r.left || left >= r.right);
                if (overlapX && overlapY) {
                    hitX = true;
                    if (velocity_.x > 0)      fixX = r.left  - kHalfW;
                    else if (velocity_.x < 0) fixX = r.right + kHalfW;
                }
            }
        }

        position_.x = hitX ? fixX : nextX;
        if (hitX) velocity_.x = 0.0f;
    }

    // ---- 次に Y を処理 ----
    {
        float nextY = position_.y + velocity_.y;

        float left   = position_.x - kHalfW;
        float right  = position_.x + kHalfW;
        float bottom = nextY - kHalfH;
        float top    = nextY + kHalfH;

        auto minIdx = mapChipField.GetMapChipIndexByPosition({ left,  bottom, 0 });
        auto maxIdx = mapChipField.GetMapChipIndexByPosition({ right, top,    0 });

        bool hitY = false;
        float fixY = nextY;

        for (uint32_t y = minIdx.yIndex; y <= maxIdx.yIndex; ++y) {
            for (uint32_t x = minIdx.xIndex; x <= maxIdx.xIndex; ++x) {
                MapChipType t = mapChipField.GetMapChipTypeByIndex(x, y);
                if (t != MapChipType::kBlock && t != MapChipType::kBlock2) continue;
                auto r = mapChipField.GetRectByIndex(x, y);

                bool overlapX = !(right <= r.left || left >= r.right);
                bool overlapY = !(top   <= r.bottom || bottom >= r.top);
                if (overlapX && overlapY) {
                    hitY = true;
                    if (velocity_.y > 0) { // 頭上衝突
                        fixY = r.bottom - kHalfH;
                        velocity_.y = 0.0f;
                        isJumping_ = false;
                        headBonkTimer_ = kHeadBonkLock_;
                    } else if (velocity_.y < 0) { // 着地
                        fixY = r.top + kHalfH;
                        velocity_.y = 0.0f;
                        onGround = true;
                    }
                }
            }
        }

        position_.y = hitY ? fixY : nextY;
    }

    isOnGround_ = onGround;
    if (isOnGround_) {
        // 着地後にジャンプ状態をリセット
        jumpCount_ = 0;
        isJumping_ = false;
        jumpPressDuration_ = 0.0f;
        currentMaxJumpHoldTime_ = 0.0f;
    }
}

void Player::LandOnExternalGround(float groundTopY)
{
    const float kHalfH = height_ * 0.5f;
    position_.y = groundTopY + kHalfH;
    if (velocity_.y < 0.0f) {
        velocity_.y = 0.0f;
    }
    isOnGround_ = true;
    jumpCount_ = 0;
    isJumping_ = false;
    jumpPressDuration_ = 0.0f;
    currentMaxJumpHoldTime_ = 0.0f;
}

void Player::SnapOnGround(float groundTopY)
{
    const float kHalfH = height_ * 0.5f;

    // 「足」が groundTopY の上に来るように配置する
    position_.y = groundTopY + kHalfH;

    // 下降中なら、そのまま垂直速度を 0 にする
    if (velocity_.y < 0.0f) {
        velocity_.y = 0.0f;
    }

    // 着地したものとして扱い、ジャンプ状態をリセットする
    isOnGround_ = true;
    jumpCount_ = 0;
    isJumping_ = false;
    jumpPressDuration_ = 0.0f;
    currentMaxJumpHoldTime_ = 0.0f;
}

void Player::ApplyExternalDisplacement(const Vector3& delta)
{
     position_ = position_ + delta;

    if (model_) {
        model_->SetTranslate(position_);
        model_->Update();
    }
}

void Player::ResetForMapTransition(bool keepFacing) {
    // 速度と動作状態をリセット
    velocity_ = { 0, 0, 0 };
    isJumping_ = false;
    jumpPressDuration_ = 0.0f;
    isOnGround_ = false;
    jumpCount_ = 0;
    headBonkTimer_ = 0.0f;
    currentMaxJumpHoldTime_ = 0.0f;

    // ダッシュ状態をリセット
    isDashing_ = false;
    isDashJumping_ = false;
    dashTimer_ = 0.0f;
    dashCooldownTimer_ = 0.0f;  // クールダウンを維持したい場合はこの行を削除
    canDash_ = true;

    isInvincible_          = false;
    damageInvincibleTimer_   = 0.0f;
    dashInvincibleTimer_     = 0.0f;
    stompInvincibleTimer_    = 0.0f;
    damageBlinkTimer_        = 0.0f;
    damageBlinkVisible_      = true;
    // 重力の基準値を復帰する（新しいパラメータ体系に合わせる）
    originalGravity_ = gravityBase_;
    // 現在のマップを離れる / 転送時は水中状態を保持しない
    inWater_ = false;
    onWaterSurface_ = false;
}
void Player::StartDeathFall() {
    if (deathStarted_) { return; }
    deathStarted_ = true;
    isDead_ = true;
    // 死亡後は水判定を行わない
    inWater_ = false;
    onWaterSurface_ = false;
    // ただちにすべての動作を中断する
    isDashing_ = false; canDash_ = false;
    isDashJumping_ = false; dashTimer_ = 0.0f;
    isJumping_ = false; jumpPressDuration_ = currentMaxJumpHoldTime_;
    headBonkTimer_ = 0.0f;
    isOnGround_ = false;
    currentMaxJumpHoldTime_ = 0.0f;

    velocity_.x = 0.0f;          // ← ゼロクリア水平速度
    velocity_.z = 0.0f;          // ← かつクリアするZ方向

    // 小さく跳ね上げて倒立回転の準備をする
    velocity_.y = deathJumpVel_; // 少し上向きの初速度を与える
    deathTimer_ = 0.0f;
    deathSpinZ_ = 0.0f;
    jumpCount_ = 0;
}

void Player::StartInvincible(float duration)
{
    // 被ダメージ無敵（点滅あり）
    if (duration <= 0.0f) {
        damageInvincibleTimer_ = 0.0f;
        damageBlinkTimer_ = 0.0f;
        damageBlinkVisible_ = true;
        isInvincible_ = (dashInvincibleTimer_ > 0.0f) || (stompInvincibleTimer_ > 0.0f);
        return;
    }

    // 既存の無敵時間を短くしない
    damageInvincibleTimer_ = (std::max)(damageInvincibleTimer_, duration);

    // 点滅を再開始する
    damageBlinkTimer_ = 0.0f;
    // 開始時にすぐ点滅させるため、先に不可視にする
    damageBlinkVisible_ = false;

    isInvincible_ = true;
}

void Player::StartDashInvincible(float duration)
{
    if (duration <= 0.0f) { return; }
    dashInvincibleTimer_ = (std::max)(dashInvincibleTimer_, duration);
    isInvincible_ = true;
}

void Player::StartStompInvincible(float duration)
{
    if (duration <= 0.0f) { return; }
    stompInvincibleTimer_ = (std::max)(stompInvincibleTimer_, duration);
    isInvincible_ = true;
}

void Player::StartStompCooldown(float duration)
{
    // duration <= 0 の場合はクリアとして扱ってもよい
    if (duration <= 0.0f) {
        stompCooldownTimer_ = 0.0f;
        return;
    }
    stompCooldownTimer_ = (std::max)(stompCooldownTimer_, duration);
}


void Player::StartStompKick(float vx, float duration)
{
    if (duration <= 0.0f) { return; }

    // 既存のノックバック時間を短くしない
    stompKickTimer_ = (std::max)(stompKickTimer_, duration);
    stompKickTotal_ = (std::max)(stompKickTotal_, duration);
    stompKickVx_ = vx;
}


