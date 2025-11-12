#include "Player.h"
#include "ModelManager.h"
#include <algorithm>
#include <numbers>
#include <cmath>

Player::Player() {}

Player::~Player() {
    if (model_) {
        delete model_;
        model_ = nullptr;
    }
}

void Player::Initialize(Object3dCommon* object3dCommon, Camera* camera) {
    object3dCommon_ = object3dCommon;
    camera_ = camera;

    model_ = new Object3d();
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
}

void Player::Update(Input* input, const MapChipField& mapChipField) {
    const float deltaTime = 1.0f / 60.0f; // 先按 60FPS 固定，后续可外部传参

    // --- HP 每秒递减 ---
    if (hpDrainPerSec_ > 0.0f && hp_ > 0.0f) {
        hp_ -= hpDrainPerSec_ * deltaTime;
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

    // 触发冲刺（左 Shift）
    if (input->TriggerKey(DIK_LSHIFT) && canDash_ && !isDashing_) {
        isDashing_ = true;
        dashTimer_ = 0.0f;
        dashCooldownTimer_ = dashCooldown_;

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
    } else {
        velocity_.x = 0.0f;
    }

    float currentMoveSpeed = moveSpeed_;
    Vector3 moveVelocity = moveInput * currentMoveSpeed;

    if (isDashing_) {
        // 冲刺期间固定速度，忽略输入
        moveVelocity = dashDirection_ * (moveSpeed_ * dashSpeedMultiplier_);
        if (isDashJumping_) {
            moveVelocity.y = 0.0f;
        }
    }

    velocity_.x = moveVelocity.x;

    // ========= 跳跃（一次性冲量 + 长按小加速度 + 可变重力）=========
    const bool isSpacePressed   = input->PushKey(DIK_SPACE);
    const bool justPressedJump  = input->TriggerKey(DIK_SPACE);

 // 起跳 & 二段跳
    if (justPressedJump) {

        // ① 地面起跳（重置跳跃次数）
        if (isOnGround_) {
            isOnGround_ = false;

            isJumping_ = true;
            jumpPressDuration_ = 0.0f;
            currentMaxJumpHoldTime_ = maxJumpHoldTimeFirst_;   // ☆ 一段跳用自己的时间上限

            velocity_.y = jumpVelInit_;
            jumpCount_ = 1; // 已经用掉第一段

            if (isDashing_) {
                velocity_.x = dashDirection_.x * (moveSpeed_ * dashSpeedMultiplier_);
            }
        }
        // ② 空中二段跳
        else if (!isOnGround_ && jumpCount_ < maxJumpCount_) {

            isJumping_ = true;
            jumpPressDuration_ = 0.0f;
            currentMaxJumpHoldTime_ = maxJumpHoldTimeSecond_;  // ☆ 二段跳单独算自己的蓄力时间

            // 打断冲刺，避免乱飞
            isDashing_ = false;
            isDashJumping_ = false;

            velocity_.y = jumpVelInit_;
            jumpCount_++;

            // TODO: 二段跳特效 / 音效
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
    // 早松手（结束可控上升但还在上升）→ 更大重力，短跳更利落
    if (!isOnGround_ && velocity_.y > 0.0f && !isJumping_) {
        g *= lowJumpGravityScale_;
    }
    // 下落期 → 更重
    if (!isOnGround_ && velocity_.y <= 0.0f) {
        g *= fallGravityScale_;
    }
    // 空中冲刺 → 无重力
    if (isDashing_) {
        g = 0.0f;
    }

    // 应用重力
    velocity_.y += g * deltaTime;

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
    if (model_) {
        model_->Draw();
    }
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
                if (mapChipField.GetMapChipTypeByIndex(x, y) != MapChipType::kBlock) continue;
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
                if (mapChipField.GetMapChipTypeByIndex(x, y) != MapChipType::kBlock) continue;
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

    // 重力“基准”恢复（与新参数体系对齐）
    originalGravity_ = gravityBase_;
}
void Player::StartDeathFall() {
    if (deathStarted_) { return; }
    deathStarted_ = true;
    isDead_ = true;

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