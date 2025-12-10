#include "Enemy.h"
#include "ModelManager.h"

void Enemy::Initialize(
    Object3dCommon* common,
    Camera* camera,
    const Vector3& spawnPos,
    EnemyType type
) {
    type_     = type;
    position_ = spawnPos;

    obj_ = std::make_unique<Object3d>();
    obj_->Initialize(common);
    obj_->SetCamera(camera);

    // 根据敌人类型切换模型（这里的路径你按自己实际资源改）
    switch (type_) {
    case EnemyType::Type0:
        obj_->SetModel("enemy0/enemy0.obj");
        break;
    case EnemyType::Type1:
        obj_->SetModel("enemy1/enemy1.obj");
        break;
    }
    obj_->SetTranslate(position_);
}

void Enemy::Update(float deltaTime)
{

    if (isHitReacting_) {
        hitReactTimer_ -= deltaTime;
        if (hitReactTimer_ <= 0.0f) {
            isHitReacting_ = false;
            damageBlinkVisible_ = true;
        }
        else {
            damageBlinkTimer_ += deltaTime;
            if (damageBlinkTimer_ >= damageBlinkInterval_) {
                damageBlinkTimer_ -= damageBlinkInterval_;
                damageBlinkVisible_ = !damageBlinkVisible_;
            }
        }
    }
    if (obj_) {
        obj_->SetTranslate(position_);
        obj_->Update();
    }
    // 目前啥也不做，只保留接口
    // 后面可以在这里加移动 / 追踪 / 跳跃等行为
}

void Enemy::Draw()
{
   if (!obj_) { return; }

    // 正在受击且当前帧不可见 ⇒ 不画，实现“闪烁”
    if (isHitReacting_ && !damageBlinkVisible_) {
        return;
    }
    obj_->Draw();
}
void Enemy::StartHitReaction(float duration)
{
    isHitReacting_      = true;
    hitReactTimer_      = duration;
    damageBlinkTimer_   = 0.0f;
    damageBlinkVisible_ = true;
}