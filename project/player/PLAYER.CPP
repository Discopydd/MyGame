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
    // 保存上一帧位置（因为外面 Update 已经做了 position_ += velocity_）
    Vector3 prevPos = { position_.x - velocity_.x, position_.y - velocity_.y, position_.z };

    // 临时标记是否着地
    bool onGround = false;

    // --- 先处理水平方向 (只考虑 x 移动) ---
    {
        // 用上一帧的 y（不考虑本帧的 y 变化）来计算上下边界，避免竖直穿透干扰
        float tempPlayerTop    = prevPos.y + height_ / 2.0f;
        float tempPlayerBottom = prevPos.y - height_ / 2.0f;

        // 计算要检查的格子区域（以玩家当前位置 x + velocity.x 为基准）
        float nextLeft  = prevPos.x + velocity_.x - width_ / 2.0f;
        float nextRight = prevPos.x + velocity_.x + width_ / 2.0f;

        // 以玩家高度范围取几个 y 检测点（-1..1 的格子检测）
        for (int yOffset = -1; yOffset <= 1; ++yOffset) {
            for (int xOffset = -1; xOffset <= 1; ++xOffset) {
                Vector3 checkPos = {
                    prevPos.x + xOffset * MapChipField::kBlockWidth + velocity_.x,
                    prevPos.y + yOffset * MapChipField::kBlockHeight,
                    0.0f
                };

                MapChipField::IndexSet index = mapChipField.GetMapChipIndexByPosition(checkPos);
                MapChipType type = mapChipField.GetMapChipTypeByIndex(index.xIndex, index.yIndex);
                if (type != MapChipType::kBlock) continue;

                MapChipField::Rect blockRect = mapChipField.GetRectByIndex(index.xIndex, index.yIndex);

                // 检测与格子在 y 上是否有重叠（只要垂直区间有重叠才需要处理水平碰撞）
                if (!(nextRight > blockRect.left && nextLeft < blockRect.right &&
                      tempPlayerTop > blockRect.bottom && tempPlayerBottom < blockRect.top)) {
                    continue;
                }

                // 移动方向决定如何修正
                if (velocity_.x > 0.0f) {
                    // 向右移动，碰到方块左边
                    position_.x = blockRect.left - width_ / 2.0f;
                    velocity_.x = 0.0f;
                } else if (velocity_.x < 0.0f) {
                    // 向左移动，碰到方块右边
                    position_.x = blockRect.right + width_ / 2.0f;
                    velocity_.x = 0.0f;
                }
                // 一旦处理到一个阻挡块，就不必再处理其它水平块（已被修正）
            }
        }
    }

    // --- 再处理竖直方向 (只考虑 y 移动) ---
    {
        // 用当前（或已被水平修正的）x 来判断横向重叠
        float tempPlayerLeft  = position_.x - width_ / 2.0f;
        float tempPlayerRight = position_.x + width_ / 2.0f;

        float nextBottom = prevPos.y + velocity_.y - height_ / 2.0f;
        float nextTop    = prevPos.y + velocity_.y + height_ / 2.0f;

        for (int yOffset = -1; yOffset <= 1; ++yOffset) {
            for (int xOffset = -1; xOffset <= 1; ++xOffset) {
                Vector3 checkPos = {
                    prevPos.x + xOffset * MapChipField::kBlockWidth,
                    prevPos.y + yOffset * MapChipField::kBlockHeight + velocity_.y,
                    0.0f
                };

                MapChipField::IndexSet index = mapChipField.GetMapChipIndexByPosition(checkPos);
                MapChipType type = mapChipField.GetMapChipTypeByIndex(index.xIndex, index.yIndex);
                if (type != MapChipType::kBlock) continue;

                MapChipField::Rect blockRect = mapChipField.GetRectByIndex(index.xIndex, index.yIndex);

                // 检查横向是否重叠（只有横向有重叠才需处理竖直碰撞）
                if (!(tempPlayerRight > blockRect.left && tempPlayerLeft < blockRect.right &&
                      nextTop > blockRect.bottom && nextBottom < blockRect.top)) {
                    continue;
                }

                // 当向下移动（velocity_.y < 0）且碰到地面时
                if (velocity_.y < 0.0f) {
                    // 把玩家放到方块上方
                    position_.y = blockRect.top + height_ / 2.0f;
                    velocity_.y = 0.0f;
                    onGround = true;
                }
                // 当向上移动（velocity_.y > 0）且撞天花板时
                else if (velocity_.y > 0.0f) {
                    position_.y = blockRect.bottom - height_ / 2.0f;
                    velocity_.y = 0.0f;
                }
                // 处理完一个垂直阻挡后可以继续检查（以保证处理多个小方块的情况）
            }
        }
    }

    // 更新着地状态
    isOnGround_ = onGround;
}

