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

    void SetPosition(const Vector3& pos) { position_ = pos; }
    const Vector3& GetPosition() const { return position_; }

    bool IsDashing() const { return isDashing_; }
    float GetDashCooldown() const { return dashCooldownTimer_; }
    float GetDashCooldownDuration() const { return dashCooldown_; }
    bool CanDash() const { return canDash_; }
    float GetHeight() const { return height_; }

    void ResetForMapTransition(bool keepFacing = true);
private:
    Object3d* model_ = nullptr;
    Object3dCommon* object3dCommon_ = nullptr;
    Camera* camera_ = nullptr;

    Vector3 position_ = { 0, 0, 0 };
    Vector3 velocity_ = { 0, 0, 0 };

    float moveSpeed_ = 0.1f;
    float jumpPower_ = 0.25f;       // 基础跳跃高度
    float maxJumpPower_ = 0.5f;     // 长按最大跳跃高度
    float gravity_ = -0.01f;
    bool isOnGround_ = false;
    bool isJumping_ = false;        // 是否正在蓄力跳跃
    bool wasSpacePressed_ = false;  // 上一帧空格键是否按下
    float jumpPressDuration_ = 0.0f; // 空格键按下的持续时间（秒）
    const float maxJumpPressTime_ = 0.5f; // 长按空格的最大有效时间（秒）

    enum class LRDirection {
        kRight,//右
        kLeft,//左
    };
    LRDirection lrDirection_ = LRDirection::kRight;
    float turnStartRotationY_ = 0;
    float turnTargetRotationY_ = 0;
    int turnCurrentFrame_ = 0;
    const int turnTotalFrames_ = 10;
    float currentRotationY_ = 0.0f;

    float width_ = 1.5f;    // 玩家碰撞宽度（假设为1单位）
    float height_ = 1.5f;   // 玩家碰撞高度（假设为2单位）

    bool isDashing_ = false;          // 是否正在冲刺
    float dashSpeedMultiplier_ = 2.0f; // 冲刺速度倍率
    float dashDuration_ = 0.5f;        // 冲刺持续时间（秒）
    float dashTimer_ = 0.0f;           // 冲刺计时器
    float dashCooldown_ = 1.5;        // 冲刺冷却时间（秒）
    float dashCooldownTimer_ = 0.0f;   // 冷却计时器
    bool canDash_ = true;              // 是否可以冲刺
    const float dashCooldownThreshold_ = 0.1f; // 冷却完成阈值
    Vector3 dashDirection_ = { 1, 0, 0 }; // 冲刺方向（默认为向右）

    bool isDashJumping_ = false;        // 是否在跳跃过程中冲刺
    float originalGravity_ = -0.01f;    // 原始重力值
    float dashGravity_ = 0.0f;          // 冲刺时的重力（设置为0实现无重力效果） 

    // Player.h
    float headBonkTimer_ = 0.0f;                // 顶头后的短暂锁
    static inline const float kHeadBonkLock_ = 0.05f; // 50ms，可根据手感调


};
