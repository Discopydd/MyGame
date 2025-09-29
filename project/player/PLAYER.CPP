#include "Player.h"
#include "ModelManager.h"
#include <algorithm>
#include <numbers>
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

    position_ = {-10, 0, 0};
    velocity_ = {0, 0, 0};

    isDashing_ = false;
    dashTimer_ = 0.0f;
    dashCooldownTimer_ = 0.0f;
    canDash_ = true;
    dashDirection_ = {1, 0, 0};

    originalGravity_ = gravity_;
    dashGravity_ = 0.0f;
}

void Player::Update(Input* input, const MapChipField& mapChipField) {

    const float deltaTime = 1.0f / 60.0f; // 假设60FPS
     // deltaTime 你的代码里目前固定 1/60，也可以传参
    headBonkTimer_ = (std::max)(0.0f, headBonkTimer_ - deltaTime);
    // 更新冲刺状态
    if (isDashing_) {
        dashTimer_ += deltaTime;

        // 冲刺结束
        if (dashTimer_ >= dashDuration_) {
            isDashing_ = false;
            dashTimer_ = 0.0f;
            dashCooldownTimer_ = dashCooldown_;
            canDash_ = false;
            if (isDashJumping_) {
                gravity_ = originalGravity_;
                isDashJumping_ = false;
            }
        }
    }

    // 更新冷却时间
    if (!canDash_) {
        dashCooldownTimer_ -= deltaTime;
        if (dashCooldownTimer_ <= dashCooldownThreshold_) {
            dashCooldownTimer_ = 0.0f;
            canDash_ = true;
        }
    }

    // 检测冲刺按键（左Shift）
    if (input->TriggerKey(DIK_LSHIFT) && canDash_ && !isDashing_) {
        isDashing_ = true;
        dashTimer_ = 0.0f;
        dashCooldownTimer_ = dashCooldown_;
        // 确定冲刺方向
        if (lrDirection_ == LRDirection::kRight) {
            dashDirection_ = { 1, 0, 0 }; // 向右
        }
        else {
            dashDirection_ = { -1, 0, 0 }; // 向左
        }
        // 如果在跳跃过程中冲刺
        if (!isOnGround_) {
            isDashJumping_ = true;
            gravity_ = dashGravity_; // 设置为零重力实现平移效果
            velocity_.y = 0; // 重置垂直速度
            isJumping_ = false;
            jumpPressDuration_ = maxJumpPressTime_;
        }
    }

    // 左右移动（冲刺时保持方向）
    Vector3 moveInput = { 0, 0, 0 };

    // 左右移动
    if (input->PushKey(DIK_D) || input->PushKey(DIK_RIGHT)) {
        moveInput.x = 1.0f;
        if (lrDirection_ != LRDirection::kRight) {
            lrDirection_ = LRDirection::kRight;
            turnStartRotationY_ = currentRotationY_;
            turnTargetRotationY_ = 0.0f;
            turnCurrentFrame_ = 0;
        }
    }
    else if (input->PushKey(DIK_A) || input->PushKey(DIK_LEFT)) {
        moveInput.x = -1.0f;
        if (lrDirection_ != LRDirection::kLeft) {
            lrDirection_ = LRDirection::kLeft;
            turnStartRotationY_ = currentRotationY_;
            turnTargetRotationY_ = DirectX::XM_PI;
            turnCurrentFrame_ = 0;
        }
    }
    else {
        velocity_.x = 0;
    }

    // 计算实际移动速度（冲刺时使用冲刺速度）
    float currentMoveSpeed = moveSpeed_;
    Vector3 moveVelocity = moveInput * currentMoveSpeed;
    if (isDashing_) {
        // 冲刺时使用固定速度，忽略输入
        moveVelocity = dashDirection_ * (moveSpeed_ * dashSpeedMultiplier_);
         // 跳跃冲刺时保持水平移动
        if (isDashJumping_) {
            moveVelocity.y = 0; // 确保垂直速度为0
        }
    }

    // 设置水平速度
    velocity_.x = moveVelocity.x;

    // 检测空格键状态
    bool isSpacePressed = input->PushKey(DIK_SPACE);

    // 按下空格立即跳跃（仅在地面时触发）
    if (isOnGround_ && input->TriggerKey(DIK_SPACE)) {
        velocity_.y = jumpPower_; // 初始跳跃高度
        isOnGround_ = false;
        isJumping_ = true;
        jumpPressDuration_ = 0.0f; // 重置蓄力时间
         if (isDashing_) {
            velocity_.x = dashDirection_.x * (moveSpeed_ * dashSpeedMultiplier_);
        }
    }

     // ====== 长按续升（受顶头锁控制） ======
    if (headBonkTimer_ <= 0.0f) {
        if (isJumping_ && isSpacePressed && !isDashing_) {
            jumpPressDuration_ += deltaTime;
            jumpPressDuration_ = (std::min)(jumpPressDuration_, maxJumpPressTime_);

            if (jumpPressDuration_ < maxJumpPressTime_) {
                velocity_.y = jumpPower_; // 保持初始跳跃速度
            }
        }
    }

    // 松开空格时停止蓄力
    if (isJumping_ && (!isSpacePressed || jumpPressDuration_ >= maxJumpPressTime_)) {
        isJumping_ = false;
    }

    // 更新上一帧空格键状态
    wasSpacePressed_ = isSpacePressed;

    // 重力
    velocity_.y += gravity_;


    // 检测碰撞并修正位置
    HandleMapCollision(mapChipField);

    if (turnCurrentFrame_ < turnTotalFrames_) {
        turnCurrentFrame_++;
        float t = static_cast<float>(turnCurrentFrame_) / turnTotalFrames_;
        float easedT = t * t * (3.0f - 2.0f * t); // 平滑插值
        currentRotationY_ = turnStartRotationY_ + (turnTargetRotationY_ - turnStartRotationY_) * easedT;
        model_->SetRotate({ 0.0f, currentRotationY_, 0.0f });
    }
    // 更新模型
    model_->SetTranslate(position_);
    model_->Update();
}

void Player::Draw() {
    if (model_) {
        model_->Draw();
    }
}
void Player::HandleMapCollision(const MapChipField& mapChipField) {
    const float deltaTime = 1.0f / 60.0f; // 如果你之后传参，可以去掉
    const float kHalfW = width_ / 2.0f;
    const float kHalfH = height_ / 2.0f;

    bool onGround = false;

    // ---------- 先处理 X ----------
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
                    if (velocity_.x > 0) fixX = r.left - kHalfW;
                    else if (velocity_.x < 0) fixX = r.right + kHalfW;
                }
            }
        }

        position_.x = hitX ? fixX : nextX;
        if (hitX) velocity_.x = 0.0f;
    }

    // ---------- 再处理 Y ----------
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
                        velocity_.y = 0;
                        isJumping_ = false;
                        headBonkTimer_ = kHeadBonkLock_; // <<=== 关键
                    } else if (velocity_.y < 0) { // 落地
                        fixY = r.top + kHalfH;
                        velocity_.y = 0;
                        onGround = true;
                    }
                }
            }
        }

        position_.y = hitY ? fixY : nextY;
    }

    isOnGround_ = onGround;
}


