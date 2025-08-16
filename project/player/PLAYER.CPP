#include "Player.h"
#include "ModelManager.h"
#include <algorithm>
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

    // 长按空格延长跳跃时间（但不增加速度）
    if (isJumping_ && isSpacePressed && !isDashing_) {
        jumpPressDuration_ += deltaTime;
        jumpPressDuration_ = (std::min)(jumpPressDuration_, maxJumpPressTime_);

        // 仅当按住空格且在最大时间内时，保持跳跃速度
        if (jumpPressDuration_ < maxJumpPressTime_) {
            velocity_.y = jumpPower_; // 保持初始跳跃速度
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

    // 临时保存下一步位置
    position_ += velocity_;

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
    Vector3 prevPos = position_ - velocity_; // 上一帧位置

    bool onGround = false;

    // --- 水平碰撞检测 (x轴) ---
    {
        float playerTop = prevPos.y + height_ / 2.0f;
        float playerBottom = prevPos.y - height_ / 2.0f;

        // 计算玩家下一步的左右边界
        float nextLeft = prevPos.x + velocity_.x - width_ / 2.0f;
        float nextRight = prevPos.x + velocity_.x + width_ / 2.0f;

        // 检测玩家周围的9个格子
        for (int yOffset = -1; yOffset <= 1; ++yOffset) {
            for (int xOffset = -1; xOffset <= 1; ++xOffset) {
                // 检测点使用玩家当前位置
                Vector3 checkPos = {
                    position_.x + xOffset * MapChipField::kBlockWidth,
                    position_.y + yOffset * MapChipField::kBlockHeight,
                    0.0f
                };

                MapChipField::IndexSet index = mapChipField.GetMapChipIndexByPosition(checkPos);
                MapChipType type = mapChipField.GetMapChipTypeByIndex(index.xIndex, index.yIndex);
                if (type != MapChipType::kBlock) continue;

                MapChipField::Rect blockRect = mapChipField.GetRectByIndex(index.xIndex, index.yIndex);

                // 检查是否与玩家下一步位置重叠
                if (nextRight > blockRect.left && nextLeft < blockRect.right &&
                    playerTop > blockRect.bottom && playerBottom < blockRect.top)
                {
                    if (velocity_.x > 0) { // 向右移动
                        position_.x = blockRect.left - width_ / 2.0f;
                        velocity_.x = 0;
                    }
                    else if (velocity_.x < 0) { // 向左移动
                        position_.x = blockRect.right + width_ / 2.0f;
                        velocity_.x = 0;
                    }
                }
            }
        }
    }

    // --- 垂直碰撞检测 (y轴) ---
    {
        float playerLeft = position_.x - width_ / 2.0f;
        float playerRight = position_.x + width_ / 2.0f;

        // 计算玩家下一步的上下边界
        float nextBottom = prevPos.y + velocity_.y - height_ / 2.0f;
        float nextTop = prevPos.y + velocity_.y + height_ / 2.0f;

        for (int yOffset = -1; yOffset <= 1; ++yOffset) {
            for (int xOffset = -1; xOffset <= 1; ++xOffset) {
                Vector3 checkPos = {
                    position_.x + xOffset * MapChipField::kBlockWidth,
                    position_.y + yOffset * MapChipField::kBlockHeight,
                    0.0f
                };

                MapChipField::IndexSet index = mapChipField.GetMapChipIndexByPosition(checkPos);
                MapChipType type = mapChipField.GetMapChipTypeByIndex(index.xIndex, index.yIndex);
                if (type != MapChipType::kBlock) continue;

                MapChipField::Rect blockRect = mapChipField.GetRectByIndex(index.xIndex, index.yIndex);

                // 检查是否与玩家下一步位置重叠
                if (playerRight > blockRect.left && playerLeft < blockRect.right &&
                    nextTop > blockRect.bottom && nextBottom < blockRect.top)
                {
                    if (velocity_.y > 0) { // 向上移动
                        position_.y = blockRect.bottom - height_ / 2.0f;
                        velocity_.y = 0;
                    }
                    else if (velocity_.y < 0) { // 向下移动
                        position_.y = blockRect.top + height_ / 2.0f;
                        velocity_.y = 0;
                        onGround = true;
                    }
                }
            }
        }
    }

    isOnGround_ = onGround;
}

