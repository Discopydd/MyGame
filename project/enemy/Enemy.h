#pragma once
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Camera.h"
#include "Vector3.h"
#include <cstdint>

enum class EnemyType : uint8_t {
    Type0 = 0,   // 比如普通敌人
    Type1 = 1,   // 比如另一种造型
};

class Enemy {
public:
    Enemy() = default;
     ~Enemy() = default;

    // 初始化：指定共通资源、相机、出生位置、敌人类型
    void Initialize(
        Object3dCommon* common,
        Camera* camera,
        const Vector3& spawnPos,
        EnemyType type
    );

    // 先留接口，后面要做行为逻辑直接写在这里
    void Update(float deltaTime);

    void Draw();

    EnemyType GetType() const { return type_; }
    const Vector3& GetPosition() const { return position_; }

    float GetWidth()  const { return width_; }
    float GetHeight() const { return height_; }

    void StartHitReaction(float duration);
    bool IsHitReacting() const { return isHitReacting_; }
private:
    std::unique_ptr<Object3d> obj_;
    Vector3      position_{};
    EnemyType    type_{ EnemyType::Type0 };

    // 碰撞体积（和玩家差不多大就行）
    float width_  = 1.5f;
    float height_ = 1.5f;

    // --- 受击闪烁状态 ---
    bool  isHitReacting_       = false;
    float hitReactTimer_       = 0.0f;
    float damageBlinkTimer_    = 0.0f;
    float damageBlinkInterval_ = 0.08f; // 和玩家一样的闪烁节奏
    bool  damageBlinkVisible_  = true;
};
