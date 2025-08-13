#include "Player.h"
#include "ModelManager.h"

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
    model_->SetModel("player/player.obj"); // 模型路径
    model_->SetCamera(camera_);

    position_ = {-10, 0, 0};
    velocity_ = {0, 0, 0};
}

void Player::Update(Input* input, const MapChipField& mapChipField) {
  // 左右移动
    if (input->PushKey(DIK_D) || input->PushKey(DIK_RIGHT)) {
        velocity_.x = moveSpeed_;
        if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;
				turnStartRotationY_ = currentRotationY_;
                turnTargetRotationY_ = 0.0f;
				turnCurrentFrame_ = 0;
			}
    } else if (input->PushKey(DIK_A) || input->PushKey(DIK_LEFT)) {
        velocity_.x = -moveSpeed_;
        if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;
				turnStartRotationY_ = currentRotationY_;
				turnTargetRotationY_  =DirectX::XM_PI;
				turnCurrentFrame_ = 0;
			}
    } else {
        velocity_.x = 0;
    }
 
    
    // 检测空格键状态
    bool isSpacePressed = input->PushKey(DIK_SPACE);

    // 按下空格立即跳跃（仅在地面时触发）
    if (isOnGround_ && input->TriggerKey(DIK_SPACE)) {
        velocity_.y = jumpPower_; // 初始跳跃高度
        isOnGround_ = false;
        isJumping_ = true;
        jumpPressDuration_ = 0.0f; // 重置蓄力时间
    }

    // 长按空格延长跳跃时间（但不增加速度）
    if (isJumping_ && isSpacePressed) {
        jumpPressDuration_ += 1.0f / 60.0f; // 帧率为60FPS
        jumpPressDuration_ = min(jumpPressDuration_, maxJumpPressTime_);

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

    // 更新位置
    position_ += velocity_;

    // 地面判定
    if (position_.y <= 0) {
        position_.y = 0;
        velocity_.y = 0;
        isOnGround_ = true;
    }
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
