#include "PlayerCamera.h"
#include <cmath>
#include <algorithm>

PlayerCamera::PlayerCamera() : Camera() {}

void PlayerCamera::Initialize(Camera* camera, const Player* player, const MapChipField* map)
{
	camera_ = camera;
    player_ = player;
    map_ = map;
}
void PlayerCamera::Update() {
    if (!camera_ || !player_ || !map_) return;

    // 目標位置を計算
    Vector3 targetPos = CalculateTargetPosition();
    
    // なめらかに追従
    Vector3 currentPos = camera_->GetTransform().translate;
    Vector3 newPos = {
        currentPos.x + (targetPos.x - currentPos.x) * followSpeed_,
        currentPos.y + (targetPos.y - currentPos.y) * followSpeed_,
        currentPos.z + (targetPos.z - currentPos.z) * followSpeed_
    };

    // 制約を適用
    if (constrainToMap_) {
        newPos = ConstrainPosition(newPos);
    }

    camera_->SetTranslate(newPos);
    camera_->Update();
}

void PlayerCamera::SnapToTarget()
{
    if (!camera_ || !player_ || !map_) return;

    Vector3 targetPos = CalculateTargetPosition();
    if (constrainToMap_) {
        targetPos = ConstrainPosition(targetPos);
    }

    camera_->SetTranslate(targetPos);
    camera_->Update();
}

Vector3 PlayerCamera::CalculateTargetPosition() const {
    Vector3 playerPos = player_->GetPosition();
    return {
        playerPos.x + offset_.x,
        playerPos.y + offset_.y,
        playerPos.z + offset_.z
    };
}

Vector3 PlayerCamera::ConstrainPosition(const Vector3& position) const {
  if (!map_ || !camera_) return position;

    // マップの実際の境界を取得
    Vector3 mapMin = useCustomBounds_ ? mapMin_ : map_->GetMapMinPosition();
    Vector3 mapMax = useCustomBounds_ ? mapMax_ : map_->GetMapMaxPosition();
    
    // カメラの実際の位置でビューポートを計算
    float cameraZ = camera_->GetTransform().translate.z;
    float halfViewHeight = std::abs(cameraZ) * std::tan(camera_->GetFovY() / 2.0f);
    float halfViewWidth = halfViewHeight * camera_->GetAspectRatio();

    // 制約境界を計算する（マップの余白も考慮）
    float minX = mapMin.x + halfViewWidth;
    float maxX = mapMax.x - halfViewWidth;
    float minY = mapMin.y + halfViewHeight;
    float maxY = mapMax.y - halfViewHeight;

    // マップが小さすぎる場合を処理
    if (maxX < minX) minX = maxX = (mapMin.x + mapMax.x) / 2.0f;
    if (maxY < minY) minY = maxY = (mapMin.y + mapMax.y) / 2.0f;

    return {
        std::clamp(position.x, minX, maxX),
        std::clamp(position.y, minY, maxY),
        position.z
    };
}
