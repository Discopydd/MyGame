#pragma once
#include "Camera.h"
#include "Vector3.h"
#include "Player.h"
#include "map/MapChipField.h"

class PlayerCamera : public Camera {
public:
    PlayerCamera();
    ~PlayerCamera() = default;

    void Initialize(Camera* camera, const Player* player, const MapChipField* map);
    void Update();

    void SetOffset(const Vector3& offset) { offset_ = offset; }
    void SetFollowSpeed(float speed) { followSpeed_ = speed; }
    void SetConstrainToMap(bool constrain) { constrainToMap_ = constrain; }
    void SetMapBounds(const Vector3& min, const Vector3& max) {
        mapMin_ = min;
        mapMax_ = max;
        useCustomBounds_ = true;
    }
    
    void ResetMapBounds() {
        useCustomBounds_ = false;
    }
private:
    Camera* camera_ = nullptr;
    const Player* player_ = nullptr;
    const MapChipField* map_ = nullptr;

    Vector3 offset_ = {0, 0.0f, 0.0f}; // 默认相机偏移 (X,Y,Z)
    float followSpeed_ = 0.1f;            // 跟随平滑度 (0-1)
    bool constrainToMap_ = true;          // 是否限制在地图边界内

    Vector3 mapMin_ = {0, 0, 0};         // 自定义地图最小边界
    Vector3 mapMax_ = {100, 100, 0};     // 自定义地图最大边界
    bool useCustomBounds_ = false;       // 是否使用自定义边界

    Vector3 CalculateTargetPosition() const;
    Vector3 ConstrainPosition(const Vector3& position) const;
};
