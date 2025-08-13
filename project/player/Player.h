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

    void SetPosition(const Vector3& pos) { position_ = pos; }
    Vector3 GetPosition() const { return position_; }

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
};
