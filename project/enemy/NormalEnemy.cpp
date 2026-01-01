#include "NormalEnemy.h"

#include "ModelManager.h"
#include "../map/MapChipField.h"
#include "../player/Player.h"

// ===========================================================
// NormalEnemy
// ===========================================================
void NormalEnemy::Initialize(Object3dCommon* common, Camera* camera, const Vector3& spawnPos, EnemyType type)
{
    InitializeCommon(common, camera, spawnPos, type);

    // 普通敌人 HP
    maxHp_ = 1;
    enrageHp_ = 0;
    hp_ = 1;

    // 根据敌人类型切换模型（路径按你资源改）
    switch (type_) {
    case EnemyType::Type0:
        obj_->SetModel("enemy0/enemy0.obj");
        break;
    case EnemyType::Type1:
        obj_->SetModel("enemy1/enemy1.obj");
        break;
    case EnemyType::Boss:
        // 防御式写法：如果误传 Boss，这里按 Type1 处理
        obj_->SetModel("enemy1/enemy1.obj");
        type_ = EnemyType::Type1;
        break;
    }

    obj_->SetTranslate(position_);
    obj_->SetRotate({ 0.0f, 0.0f, 0.0f });
    obj_->Update();
}

void NormalEnemy::Update(float dt, const MapChipField& /*map*/, const Player& /*player*/)
{
    if (!UpdateCommon(dt)) { return; }

    // 原逻辑：普通敌人仅跟随 position_ 更新渲染
    if (obj_) {
        obj_->SetTranslate(position_);
        obj_->Update();
    }
}

void NormalEnemy::Draw()
{
    DrawCommonBody();
}
