#pragma once
#include "Camera.h"
#include "Vector3.h"
#include "Player.h"
#include "map/MapChipField.h"

/// <summary>
/// プレイヤーを追従しながら表示範囲を制限するゲーム用カメラクラス。
/// </summary>
class PlayerCamera : public MyEngine::Camera {
public:
    PlayerCamera();
    ~PlayerCamera() = default;

    void Initialize(MyEngine::Camera* camera, const Player* player, const MapChipField* map);
    void Update();
    void SnapToTarget();

    void SetOffset(const MyEngine::Vector3& offset) { offset_ = offset; }
    void SetFollowSpeed(float speed) { followSpeed_ = speed; }
    void SetConstrainToMap(bool constrain) { constrainToMap_ = constrain; }
    void SetMapBounds(const MyEngine::Vector3& min, const MyEngine::Vector3& max) {
        mapMin_ = min;
        mapMax_ = max;
        useCustomBounds_ = true;
    }
    
    void ResetMapBounds() {
        useCustomBounds_ = false;
    }
private:
    MyEngine::Camera* camera_ = nullptr;
    const Player* player_ = nullptr;
    const MapChipField* map_ = nullptr;

    MyEngine::Vector3 offset_ = {0, 0.0f, 0.0f}; // デフォルトのカメラオフセット (X,Y,Z)
    float followSpeed_ = 0.1f;            // 追従のなめらかさ (0-1)
    bool constrainToMap_ = true;          // マップ境界内に制限するかどうか

    MyEngine::Vector3 mapMin_ = {0, 0, 0};         // カスタムマップ最小境界
    MyEngine::Vector3 mapMax_ = {100, 100, 0};     // カスタムマップ最大境界
    bool useCustomBounds_ = false;       // カスタム境界を使うかどうか

    MyEngine::Vector3 CalculateTargetPosition() const;
    MyEngine::Vector3 ConstrainPosition(const MyEngine::Vector3& position) const;
};
