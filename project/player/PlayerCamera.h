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
    void SnapToTarget();

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

    Vector3 offset_ = {0, 0.0f, 0.0f}; // デフォルトのカメラオフセット (X,Y,Z)
    float followSpeed_ = 0.1f;            // 追従のなめらかさ (0-1)
    bool constrainToMap_ = true;          // マップ境界内に制限するかどうか

    Vector3 mapMin_ = {0, 0, 0};         // カスタムマップ最小境界
    Vector3 mapMax_ = {100, 100, 0};     // カスタムマップ最大境界
    bool useCustomBounds_ = false;       // カスタム境界を使うかどうか

    Vector3 CalculateTargetPosition() const;
    Vector3 ConstrainPosition(const Vector3& position) const;
};
