#pragma once
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Camera.h"
#include "map/MapChipField.h"
#include <vector>

class MovingPlatform {
public:
    enum class Axis {
        Horizontal,
        Vertical
    };

    MovingPlatform() = default;
    ~MovingPlatform() = default;

    // speed 为正/负决定初始方向（正：右/上，负：左/下）
    void Initialize(Object3dCommon* common, Camera* camera,
                    const Vector3& startPos, Axis axis, float speed,int lengthInTiles);

    // allPlatforms 用来检测平台-平台碰撞
    void Update(float dt,
                const MapChipField& field,
                const std::vector<MovingPlatform*>& allPlatforms);

    void Draw();

    const Vector3& GetPosition() const { return position_; }
    const Vector3& GetPrevPosition() const { return prevPosition_; }

    // 当前平台的 AABB（和 MapChipField::Rect 一致）
    MapChipField::Rect GetRect() const;

private:
    // 一条平台上有多少块
    int lengthInTiles_ = 1;

    // 这一条上所有小方块
    std::vector<std::unique_ptr<Object3d>> tiles_;
    std::vector<Vector3>                   tileOffsets_;

    Axis axis_ = Axis::Horizontal;
    float speed_ = 1.0f;   // 绝对速度（单位：世界单位/秒）
    Vector3 dir_ = { 1, 0, 0 }; // 单位方向（左右/上下）

    Vector3 position_{};
    Vector3 prevPosition_{};

    float halfW_ = 0.0f;      // 整条平台的半宽
    float halfH_ = 0.0f;      // 整条平台的半高

    void HandleBlockCollision(const MapChipField& field);
    void HandlePlatformCollision(const std::vector<MovingPlatform*>& allPlatforms);

};
