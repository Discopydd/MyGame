#pragma once

#include "Enemy.h"

// ------------------ 普通敌人 ------------------
class NormalEnemy final : public Enemy {
public:
    void Initialize(Object3dCommon* common, Camera* camera, const Vector3& spawnPos, EnemyType type) override;
    void Update(float deltaTime, const MapChipField& map, const Player& player) override;
    void Draw() override;
};
