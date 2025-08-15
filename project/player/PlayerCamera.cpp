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

    // 计算目标位置
    Vector3 targetPos = CalculateTargetPosition();
    
    // 平滑跟随
    Vector3 currentPos = camera_->GetTransform().translate;
    Vector3 newPos = {
        currentPos.x + (targetPos.x - currentPos.x) * followSpeed_,
        currentPos.y + (targetPos.y - currentPos.y) * followSpeed_,
        currentPos.z + (targetPos.z - currentPos.z) * followSpeed_
    };

    // 应用约束
    if (constrainToMap_) {
        newPos = ConstrainPosition(newPos);
    }

    camera_->SetTranslate(newPos);
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

    // 获取地图实际边界
    Vector3 mapMin = map_->GetMapMinPosition();
    Vector3 mapMax = map_->GetMapMaxPosition();
    
    // 使用相机实际位置计算视口
    float cameraZ = camera_->GetTransform().translate.z;
    float halfViewHeight = std::abs(cameraZ) * std::tan(camera_->GetFovY() / 2.0f);
    float halfViewWidth = halfViewHeight * camera_->GetAspectRatio();

    // 计算约束边界（考虑地图偏移）
    float minX = mapMin.x + halfViewWidth;
    float maxX = mapMax.x - halfViewWidth;
    float minY = mapMin.y + halfViewHeight;
    float maxY = mapMax.y - halfViewHeight;

    // 处理地图过小的情况
    if (maxX < minX) minX = maxX = (mapMin.x + mapMax.x) / 2.0f;
    if (maxY < minY) minY = maxY = (mapMin.y + mapMax.y) / 2.0f;

    return {
        std::clamp(position.x, minX, maxX),
        std::clamp(position.y, minY, maxY),
        position.z
    };
}
