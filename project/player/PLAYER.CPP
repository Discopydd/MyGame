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

    // Dash 初始状态
    isDashing_ = false;
    dashTimer_ = 0.0f;
    dashCooldownTimer_ = 0.0f;
    canDash_ = true;
    dashDirection_ = { 1, 0, 0 };

    // 让“旧的 originalGravity_”与当前新参数对齐
    originalGravity_ = gravityBase_;
    dashGravity_ = 0.0f;

    stompCooldownTimer_ = 0.0f;

    stompKickTimer_ = 0.0f;
    stompKickTotal_ = 0.0f;
    stompKickVx_ = 0.0f;
}

void Player::Update(Input* input, const MapChipField& mapChipField) {
    const float deltaTime = 1.0f / 60.0f; // 先按 60FPS 固定，后续可外部传参
    didDoubleJumpThisFrame_ = false;

   // 记录上一帧的水状态，用于判断“刚刚进入水中”
    bool wasInWater        = inWater_;
    bool wasOnWaterSurface = onWaterSurface_;

    // 每帧默认认为不在水中，后面再基于地图重算
    inWater_ = false;
    onWaterSurface_ = false;
         // === 无敌计时（受伤/冲刺/踩头） & 闪烁（仅受伤无敌） ===
    // ① 受伤无敌：带闪烁
    if (damageInvincibleTimer_ > 0.0f) {
        damageInvincibleTimer_ -= deltaTime;

        damageBlinkTimer_ += deltaTime;
        // 每隔 damageBlinkInterval_ 翻转一次可见性
        if (damageBlinkTimer_ >= damageBlinkInterval_) {
            damageBlinkTimer_ = 0.0f;
            damageBlinkVisible_ = !damageBlinkVisible_;
        }

        // 无敌时间结束
        if (damageInvincibleTimer_ <= 0.0f) {
            damageInvincibleTimer_ = 0.0f;
            damageBlinkTimer_ = 0.0f;
            damageBlinkVisible_ = true;
        }
    }

    // ② 冲刺无敌：不闪烁
    if (dashInvincibleTimer_ > 0.0f) {
        dashInvincibleTimer_ -= deltaTime;
        if (dashInvincibleTimer_ < 0.0f) dashInvincibleTimer_ = 0.0f;
    }

    // ③ 踩头无敌：不闪烁
    if (stompInvincibleTimer_ > 0.0f) {
        stompInvincibleTimer_ -= deltaTime;
        if (stompInvincibleTimer_ < 0.0f) stompInvincibleTimer_ = 0.0f;
    }


    // ④ 踩头冷却：防止在 Boss 头顶原地无限连踩
    if (stompCooldownTimer_ > 0.0f) {
        stompCooldownTimer_ -= deltaTime;
        if (stompCooldownTimer_ < 0.0f) stompCooldownTimer_ = 0.0f;
    }

    // ⑤ 踩头击退计时：仅踩头后短时间生效（避免影响日常空中手感）
    if (stompKickTimer_ > 0.0f) {
        stompKickTimer_ -= deltaTime;
        if (stompKickTimer_ < 0.0f) stompKickTimer_ = 0.0f;
    }
    isInvincible_ = (damageInvincibleTimer_ > 0.0f) ||
                    (dashInvincibleTimer_ > 0.0f) ||
                    (stompInvincibleTimer_ > 0.0f);

    // --- HP 每秒递减 ---
    if (hpDrainPerSec_ > 0.0f && hp_ > 0.0f) {
        //hp_ -= hpDrainPerSec_ * deltaTime;
        if (hp_ < 0.0f) hp_ = 0.0f;
    }
    if (hp_ <= 0.0f && !deathStarted_) {
        StartDeathFall();
    }
    // 顶头短暂锁
    headBonkTimer_ = (std::max)(0.0f, headBonkTimer_ - deltaTime);
    if (isDead_) {
        const float dt = 1.0f / 60.0f;

        // 旋转到倒立（绕Z轴）0→π
        deathTimer_ += dt;
        float t = (std::min)(1.0f, deathTimer_ / deathRotateTime_);
        deathSpinZ_ = DirectX::XM_PI * (t * t * (3.0f - 2.0f * t)); // smoothstep

        // 强化重力，下坠；跳起后的第一帧已经给了正向初速度
        float g = gravityBase_ * deathExtraGravScale_;
        velocity_.y += g * dt;

        // 逐帧位移，**跳过碰撞**，直接坠出画面
        position_.x += velocity_.x;
        position_.y += velocity_.y;
        isOnGround_ = false;

        // 应用旋转：保持左右朝向Y旋转不变，只把Z转到180°
        Vector3 rot = { 0.0f, currentRotationY_, deathSpinZ_ };
        model_->SetRotate(rot);
        model_->SetTranslate(position_);
        model_->Update();
        return; // 死亡态独立更新完毕
    }
    // ========== 无输入（如 Intro）时：只做物理 ==========
    if (input == nullptr) {
        // 仅根据“可变重力”推进
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

    // ========= Dash 状态刷新 =========
    if (isDashing_) {
        dashTimer_ += deltaTime;
        if (dashTimer_ >= dashDuration_) {
            isDashing_ = false;
            dashTimer_ = 0.0f;
            dashCooldownTimer_ = dashCooldown_;
            canDash_ = false;

            // 结束空中冲刺，恢复正常重力控制
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
            // ========= 水方块判定 =========
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

        // 默认认为不在水中 / 不在水面，下面按检测结果再改
        inWater_ = false;
        onWaterSurface_ = false;

        if (hasWater) {
            float depth = waterSurfaceY - bottom;
            const float kSurfaceSnapThreshold = height_ * 0.5f;
            // 下落速度多大以内，仍然算“慢慢接触水面”，可以漂浮
            const float kFloatMaxFallSpeed = 0.15f;   // 可以自己再调大/调小

            const bool diveInput =
                input->PushKey(DIK_S) || input->PushKey(DIK_DOWN);

            if (depth > 0.0f) {
                // 垂直速度绝对值比较小（无论向上还是向下），认为是“轻轻接触水面”
                bool canFloatOnSurface =
                    (std::fabs(velocity_.y) <= kFloatMaxFallSpeed);

                if (!diveInput &&
                    depth <= kSurfaceSnapThreshold &&
                    canFloatOnSurface) {

                    // ★ 在水面上漂浮：保持一半身体在水中
                    SnapOnGround(waterSurfaceY - halfH);
                    onWaterSurface_ = true;
                    inWater_ = false;
                }
                else {
                    // ★ 速度较快（从高处跳落）或者按了 S：视为真正进入水内
                    inWater_ = true;
                    onWaterSurface_ = false;
                }
            }
            // depth <= 0：脚在水面以上，当成空气
        }
    }


       // === 刚从空气 / 水面进入水中的一帧：给一个固定“入水速度” ===
    bool justEnteredWater = (!wasInWater && !wasOnWaterSurface && inWater_);
    if (justEnteredWater) {

        // 判断这次跳到空中的时候，使用了几段跳
        // 0：只是掉下去    1：普通跳    2：二段跳
        int usedJumps = jumpCount_;
        if (usedJumps < 0) usedJumps = 0;
        if (usedJumps > 2) usedJumps = 2;

        // 基础入水速度（从空中掉进水里时往下轻轻一滑）
        const float kBaseEnterSinkSpeed = -0.10f; // 单位：世界单位/帧左右的感觉

        // 二段跳额外加深一点（越多段跳，入水那一下越深）
        const float kExtraSinkPerExtraJump = -0.05f; // 每比“一段跳”多一段，多沉这么多

        // 计算目标入水速度：
        //  - 掉落/一段跳：   -0.10
        //  - 二段跳入水：    -0.15
        float targetVy = kBaseEnterSinkSpeed
                       + (float)(usedJumps - 1) * kExtraSinkPerExtraJump;

        // 不允许是向上的速度，确保一定是往下轻轻沉
        if (targetVy >= 0.0f) {
            targetVy = -0.05f;
        }

        velocity_.y = targetVy;
    }
    // 触发冲刺（左 Shift）
    if (input->TriggerKey(DIK_LSHIFT) && canDash_ && !isDashing_) {
        isDashing_ = true;
        dashTimer_ = 0.0f;
        dashCooldownTimer_ = dashCooldown_;


        // 冲刺期间无敌（不闪烁）
        StartDashInvincible(dashDuration_);
        // 确定冲刺方向
        if (lrDirection_ == LRDirection::kRight)  dashDirection_ = { 1, 0, 0 };
        else                                      dashDirection_ = { -1, 0, 0 };

        // 空中冲刺：无重力漂移
        if (!isOnGround_) {
            isDashJumping_ = true;
            velocity_.y = 0.0f;   // 立即切断上升
            isJumping_ = false;   // 结束可控上升
            jumpPressDuration_ = currentMaxJumpHoldTime_; // 关闭长按窗口
        }
    }

    // ========= 水平移动 =========
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

    // ---- 水中物理倍率（可以按手感调整）----
    const float kWaterMoveSpeedScale  = 0.4f;  // 水里移动速度 40%
    const float kWaterDashSpeedScale  = 0.5f;  // 水里冲刺速度 50%
    const float kWaterJumpVelScale    = 0.2f;  // 水里起跳初速度 20%
    const float kWaterGravityScale    = 0.18f; // 水里重力 35%

    float currentMoveSpeed = moveSpeed_;
    if (inWater_) {
        currentMoveSpeed *= kWaterMoveSpeedScale;
    }
    Vector3 moveVelocity = moveInput * currentMoveSpeed;

    if (isDashing_) {
        // 冲刺期间固定速度，忽略输入
        float dashSpeed = moveSpeed_ * dashSpeedMultiplier_;
        if (inWater_) {
            dashSpeed *= kWaterDashSpeedScale;
        }
        moveVelocity = dashDirection_ * dashSpeed;
        if (isDashJumping_) {
            moveVelocity.y = 0.0f;
        }
    }

    float finalVx = moveVelocity.x;

    // 踩头击退：短时间内稳定生效，用 ease-out 让弹开更自然
    if (stompKickTimer_ > 0.0f) {
        float denom = (stompKickTotal_ > 0.0f) ? stompKickTotal_ : 1.0f;
        float r = stompKickTimer_ / denom; // 1 -> 0
        float ease = r * r;                // ease-out
        finalVx = stompKickVx_ * ease + moveVelocity.x * 0.25f; // 允许少量空中修正
    }

    velocity_.x = finalVx;

  const bool isSpacePressed   = input->PushKey(DIK_SPACE);
    const bool justPressedJump  = input->TriggerKey(DIK_SPACE);

    // 起跳 & 二段跳 / 水中无限跳
    if (justPressedJump) {

        // ① 地面起跳（重置跳跃次数）
        if (isOnGround_) {
            isOnGround_ = false;

            isJumping_ = true;
            jumpPressDuration_ = 0.0f;
            currentMaxJumpHoldTime_ = maxJumpHoldTimeFirst_;

            velocity_.y = jumpVelInit_;
            // 水面上起跳也可以更“沉”一点（按需要可以去掉这句）
            if (inWater_) {
                velocity_.y *= kWaterJumpVelScale;
            }
            jumpCount_ = 1; // 已经用掉第一段

            if (isDashing_) {
                velocity_.x = dashDirection_.x * (moveSpeed_ * dashSpeedMultiplier_);
            }
        }
        // ② 空中跳跃：
        //    - 普通情况：最多 maxJumpCount_ 次（二段跳）
        //    - inWater_ == true 时：不限制次数 → 无限跳
        else if (!isOnGround_ && (jumpCount_ < maxJumpCount_ || inWater_)) {

            isJumping_ = true;
            jumpPressDuration_ = 0.0f;
            currentMaxJumpHoldTime_ = maxJumpHoldTimeSecond_;

            // 打断冲刺，避免乱飞
            isDashing_ = false;
            isDashJumping_ = false;

            velocity_.y = jumpVelInit_;
            if (inWater_) {
                velocity_.y *= kWaterJumpVelScale;
            }

            // 为了不让 jumpCount_ 无限制变大，只在未达到上限时 +1
            if (jumpCount_ < maxJumpCount_) {
                jumpCount_++;
            }

            didDoubleJumpThisFrame_ = true; // 水中额外跳也当成“二段跳事件”，方便播特效
        }
    }

    // 长按小加速度：在“可控上升”窗口内且仍在上升
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
            isJumping_ = false; // 窗口结束或已松手
        }
    }

      // 可变重力
    float g = gravityBase_;

    if (inWater_) {
        // ★ 在水中：只用基础重力 * 水倍率，不叠加额外下落/早松手重力
        g *= kWaterGravityScale;
    } else {
        // ★ 在空气中才使用“早松手/下落更重”的逻辑
        // 早松手（结束可控上升但还在上升）→ 更大重力，短跳更利落
        if (!isOnGround_ && velocity_.y > 0.0f && !isJumping_) {
            g *= lowJumpGravityScale_;
        }
        // 下落期 → 更重
        if (!isOnGround_ && velocity_.y <= 0.0f) {
            g *= fallGravityScale_;
        }
    }

    // 在水面上漂浮 → 不受重力影响
    if (onWaterSurface_) {
        g = 0.0f;
    }

    // 空中冲刺 → 无重力（优先级最高）
    if (isDashing_) {
        g = 0.0f;
    }

    // 应用重力
    velocity_.y += g * deltaTime;
    // ★ 水中：限制最大下沉速度，避免越掉越快
    if (inWater_ && !onWaterSurface_) {
        const float kMaxSinkSpeed = -0.20f; // 每帧最多这么快往下掉（可调）
        if (velocity_.y < kMaxSinkSpeed) {
            velocity_.y = kMaxSinkSpeed;
        }
    }
    // ========= 碰撞修正 =========
    HandleMapCollision(mapChipField);

    // ========= 转身插值 =========
    if (turnCurrentFrame_ < turnTotalFrames_) {
        turnCurrentFrame_++;
        float t = static_cast<float>(turnCurrentFrame_) / turnTotalFrames_;
        float easedT = t * t * (3.0f - 2.0f * t);
        currentRotationY_ = turnStartRotationY_ + (turnTargetRotationY_ - turnStartRotationY_) * easedT;
        model_->SetRotate({ 0.0f, currentRotationY_, 0.0f });
    }

    // ========= 更新模型 =========
    model_->SetTranslate(position_);
    model_->Update();
}

void Player::Draw() {
    if (!model_) { return; }

    // 无敌期间：通过“某些帧不画”来实现闪烁
    if (isInvincible_ && !damageBlinkVisible_) {
        return;
    }

    model_->Draw();
}

void Player::HandleMapCollision(const MapChipField& mapChipField) {
    const float kHalfW = width_ / 2.0f;
    const float kHalfH = height_ / 2.0f;

    bool onGround = false;

    // ---- 先处理 X ----
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

    // ---- 再处理 Y ----
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
                    if (velocity_.y > 0) { // 顶头
                        fixY = r.bottom - kHalfH;
                        velocity_.y = 0.0f;
                        isJumping_ = false;
                        headBonkTimer_ = kHeadBonkLock_;
                    } else if (velocity_.y < 0) { // 落地
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
        // 落地后重置跳跃状态
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

    // 把“脚”放到 groundTopY 上面
    position_.y = groundTopY + kHalfH;

    // 如果正在往下掉，就把垂直速度清零
    if (velocity_.y < 0.0f) {
        velocity_.y = 0.0f;
    }

    // 视为落地，重置跳跃状态
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
    // 清零速度 & 动作
    velocity_ = { 0, 0, 0 };
    isJumping_ = false;
    jumpPressDuration_ = 0.0f;
    isOnGround_ = false;
    jumpCount_ = 0;
    headBonkTimer_ = 0.0f;
    currentMaxJumpHoldTime_ = 0.0f;

    // 冲刺状态复位
    isDashing_ = false;
    isDashJumping_ = false;
    dashTimer_ = 0.0f;
    dashCooldownTimer_ = 0.0f;  // 若想保留冷却可移除此行
    canDash_ = true;

    isInvincible_          = false;
    damageInvincibleTimer_   = 0.0f;
    dashInvincibleTimer_     = 0.0f;
    stompInvincibleTimer_    = 0.0f;
    damageBlinkTimer_        = 0.0f;
    damageBlinkVisible_      = true;
    // 重力“基准”恢复（与新参数体系对齐）
    originalGravity_ = gravityBase_;
    // 离开当前地图/传送时，不保留水中状态
    inWater_ = false;
    onWaterSurface_ = false;
}
void Player::StartDeathFall() {
    if (deathStarted_) { return; }
    deathStarted_ = true;
    isDead_ = true;
    // 死亡时不再判定水
    inWater_ = false;
    onWaterSurface_ = false;
    // 立即打断一切动作
    isDashing_ = false; canDash_ = false;
    isDashJumping_ = false; dashTimer_ = 0.0f;
    isJumping_ = false; jumpPressDuration_ = currentMaxJumpHoldTime_;
    headBonkTimer_ = 0.0f;
    isOnGround_ = false;
    currentMaxJumpHoldTime_ = 0.0f;

    velocity_.x = 0.0f;          // ← 清零水平速度
    velocity_.z = 0.0f;          // ← 并清掉Z方向

    // 微跳启动 + 倒立旋转准备
    velocity_.y = deathJumpVel_; // 给一个初速度向上跳一下
    deathTimer_ = 0.0f;
    deathSpinZ_ = 0.0f;
    jumpCount_ = 0;
}

void Player::StartInvincible(float duration)
{
    // 受伤无敌（带闪烁）
    if (duration <= 0.0f) {
        damageInvincibleTimer_ = 0.0f;
        damageBlinkTimer_ = 0.0f;
        damageBlinkVisible_ = true;
        isInvincible_ = (dashInvincibleTimer_ > 0.0f) || (stompInvincibleTimer_ > 0.0f);
        return;
    }

    // 不要把已有无敌“缩短”
    damageInvincibleTimer_ = (std::max)(damageInvincibleTimer_, duration);

    // 重新开始闪烁
    damageBlinkTimer_ = 0.0f;
    // 开始时就闪一下：先设为不可见
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
    // duration <= 0 也允许作为清零
    if (duration <= 0.0f) {
        stompCooldownTimer_ = 0.0f;
        return;
    }
    stompCooldownTimer_ = (std::max)(stompCooldownTimer_, duration);
}


void Player::StartStompKick(float vx, float duration)
{
    if (duration <= 0.0f) { return; }

    // 不要把已有击退“缩短”
    stompKickTimer_ = (std::max)(stompKickTimer_, duration);
    stompKickTotal_ = (std::max)(stompKickTotal_, duration);
    stompKickVx_ = vx;
}


