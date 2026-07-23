#pragma once
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Camera.h"
#include "map/MapChipField.h"
#include <vector>

/// <summary>
/// マップ上の移動床を指定軸に沿って往復移動させ、プレイヤーとの連動を管理するクラス。
/// </summary>
class MovingPlatform {
public:
    enum class Axis {
        Horizontal,
        Vertical
    };

    MovingPlatform() = default;
    ~MovingPlatform() = default;

    // speed の正負で初期方向を決める（正: 右/上、負: 左/下）
    void Initialize(MyEngine::Object3dCommon* common, MyEngine::Camera* camera,
                    const MyEngine::Vector3& startPos, Axis axis, float speed,int lengthInTiles);

    // allPlatforms 足場同士の衝突検出用
    void Update(float dt,
                const MapChipField& field,
                const std::vector<MovingPlatform*>& allPlatforms);

    void Draw();

    const MyEngine::Vector3& GetPosition() const { return position_; }
    const MyEngine::Vector3& GetPrevPosition() const { return prevPosition_; }

    // 現在の足場の AABB（MapChipField::Rect と一致）
    MapChipField::Rect GetRect() const;

private:
    // 1 本の足場に含まれるブロック数
    int lengthInTiles_ = 1;

    // この足場を構成する小ブロック
    std::vector<std::unique_ptr<MyEngine::Object3d>> tiles_;
    std::vector<MyEngine::Vector3>                   tileOffsets_;

    Axis axis_ = Axis::Horizontal;
    float speed_ = 1.0f;   // 絶対速度（単位: ワールド単位/秒）
    MyEngine::Vector3 dir_ = { 1, 0, 0 }; // 単位方向（左右 / 上下）

    MyEngine::Vector3 position_{};
    MyEngine::Vector3 prevPosition_{};

    float halfW_ = 0.0f;      // 足場全体の半幅
    float halfH_ = 0.0f;      // 足場全体の半高

    void HandleBlockCollision(const MapChipField& field);
    void HandlePlatformCollision(const std::vector<MovingPlatform*>& allPlatforms);

};
