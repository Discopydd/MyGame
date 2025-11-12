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

    // --- HP APIs ---
    float  GetHpRatio()   const { return maxHP_ > 0 ? hp_ / (float)maxHP_ : 0.0f; }
    float  GetHP()        const { return hp_; }
    int    GetMaxHP()     const { return maxHP_; }
    void   SetHpDrainPerSec(float v) { hpDrainPerSec_ = (std::max)(0.0f, v); }
    void   TakeDamage(float v) { hp_ = (std::max)(0.0f, hp_ - v); }
    void   Heal(float v) { hp_ = (std::min)(hp_ + v, (float)maxHP_); }
    bool IsOnGround() const { return isOnGround_; }

    bool  IsDead() const { return isDead_; }
    void  StartDeathFall();   // 触发死亡演出（被 GameScene 调用）
private:
    Object3d* model_ = nullptr;
    Object3dCommon* object3dCommon_ = nullptr;
    Camera* camera_ = nullptr;

    Vector3 position_ = { 0, 0, 0 };
    Vector3 velocity_ = { 0, 0, 0 };

    // ===== Movement =====
    float moveSpeed_ = 0.25f;

    // ===== Jump tuning =====
    // 起跳瞬间给一次“初速度”；长按在限定时窗内给予少量“向上加速度”，并对上升速度设上限
    float jumpVelInit_ = 0.56f;   // 起跳初速度
    float jumpVelMax_ = 1.05f;   // 上升速度上限
    float jumpHoldAccel_ = 1.5f;   // 长按时的额外向上加速度（单位：世界单位/秒^2）
    float maxJumpHoldTimeFirst_ = 0.15f;   // 长按生效的最大时长（秒）
    float maxJumpHoldTimeSecond_ = 0.13f;   // 二段跳可蓄力时间
    bool  isOnGround_ = false;
    bool  isJumping_ = false;   // “可控上升”阶段（在 hold 窗口内且还在上升）
    float jumpPressDuration_ = 0.0f;   // 已长按时长（秒）
    
    // 当前这一跳实际使用的最大蓄力时间（根据是第几段跳赋值）
    float currentMaxJumpHoldTime_ = 0.0f;
    // 可变重力：让“早松手/下落”更利落
    float gravityBase_ = -2.20f; // 基础重力（向下为负）
    float lowJumpGravityScale_ = 1.60f;  // 早松手仍在上升时的额外下拉倍率
    float fallGravityScale_ = 2.20f;  // 下落期的额外下拉倍率

    // 朝向 / 转身
    enum class LRDirection { kRight, kLeft };
    LRDirection lrDirection_ = LRDirection::kRight;
    float turnStartRotationY_ = 0;
    float turnTargetRotationY_ = 0;
    int   turnCurrentFrame_ = 0;
    const int turnTotalFrames_ = 10;
    float currentRotationY_ = 0.0f;

    // 碰撞体积
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

    bool  isDashJumping_ = false;       // 空中冲刺：无重力漂移
    float dashGravity_ = 0.0f;          // 仅用于保持兼容（Update 内以 isDashJumping_ 控制 g=0）

    // 顶头后的短锁
    float headBonkTimer_ = 0.0f;
    static inline const float kHeadBonkLock_ = 0.05f;

    // --- HP members ---
    int   maxHP_ = 100;
    float hp_ = 100.0f;
    float hpDrainPerSec_ = 1.0f;   // 每秒扣 5 点

    // 兼容旧接口（现在不再直接使用外部“每帧重力值”，但保留复位逻辑）
    float originalGravity_ = -2.20f; // 与 gravityBase_ 对齐

    // ---- Death (GameOver) ----
    bool  isDead_ = false;
    bool  deathStarted_ = false;
    float deathTimer_ = 0.0f;
    float deathJumpVel_ = 0.78f;     // 微跳初速度(与现有单位一致, 直接加到velocity_.y)
    float deathRotateTime_ = 0.35f;  // 旋转到倒立所需时间(s)
    float deathExtraGravScale_ = 2.2f; // 死亡下坠时的重力放大
    float deathSpinZ_ = 0.0f;        // 倒立旋转Z角(0→π)

    // ---- Jump / Double Jump ----
    int   jumpCount_ = 0;        // 已使用的跳跃次数
    int   maxJumpCount_ = 2;     // 最大跳跃次数（2 = 二段跳）
};
