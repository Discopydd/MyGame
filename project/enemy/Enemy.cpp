#include "Enemy.h"
#include "ModelManager.h"

#include "../player/Player.h"

#include <algorithm>

// ===========================================================
// Enemy (base)
// ===========================================================
void Enemy::InitializeCommon(Object3dCommon* common, Camera* camera, const Vector3& spawnPos, EnemyType type)
{
    type_ = type;
    position_ = spawnPos;

    // ===== 共通ステート初期化 =====
    isDead_ = false;
    stompInvuln_ = 0.0f;

    // HP（派生側で上書きする）
    maxHp_ = 1;
    hp_ = 1;
    enrageHp_ = 0;

    // 被弾点滅
    isHitReacting_ = false;
    hitReactTimer_ = 0.0f;
    damageBlinkTimer_ = 0.0f;
    damageBlinkVisible_ = true;

    // デフォルトのサイズ（派生側で変更する場合がある）
    width_ = 1.5f;
    height_ = 1.5f;

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
    // 初期向き: 右=0、左=PI（Player のロジックと一致）
    obj_->SetRotate({ 0.0f, 0.0f, 0.0f });
}

bool Enemy::UpdateCommon(float dt)
{
    if (isDead_) {
        return false;
    }

    // ===== 被弾点滅 =====
    if (isHitReacting_) {
        hitReactTimer_ -= dt;
        if (hitReactTimer_ <= 0.0f) {
            isHitReacting_ = false;
            damageBlinkVisible_ = true;
        }
        else {
            damageBlinkTimer_ += dt;
            if (damageBlinkTimer_ >= damageBlinkInterval_) {
                damageBlinkTimer_ -= damageBlinkInterval_;
                damageBlinkVisible_ = !damageBlinkVisible_;
            }
        }
    }

    // 踏みつけ無敵時間（Boss 用；通常敵も元ロジックを維持）
    stompInvuln_ = (std::max)(0.0f, stompInvuln_ - dt);
    return true;
}

void Enemy::DrawCommonBody()
{
    if (isDead_) { return; }
    if (!obj_) { return; }
    if (isHitReacting_ && !damageBlinkVisible_) { return; }
    obj_->Draw();
}

void Enemy::StartHitReaction(float duration)
{
    isHitReacting_ = true;
    hitReactTimer_ = duration;
    damageBlinkTimer_ = 0.0f;
    damageBlinkVisible_ = true;
}

void Enemy::OnStomp()
{
    if (isDead_) { return; }
    // 防止同一フレーム / 同じ重なりで反復トリガーしないようにする
    if (stompInvuln_ > 0.0f) { return; }

    // 通常敵: 先に保留原挙動（1回だけ点滅）
    StartHitReaction(0.40f);
    stompInvuln_ = 0.20f;
}

Enemy::PlayerContact Enemy::CheckPlayerContact(const Player& player, float stompMargin) const
{
    PlayerContact out{};

    if (isDead_) { return out; }

    const Vector3 pPos = player.GetPosition();
    const Vector3 pVel = player.GetVelocity();

    const float pHalfW = player.GetWidth() * 0.5f;
    const float pHalfH = player.GetHeight() * 0.5f;

    const float eHalfW = width_ * 0.5f;
    const float eHalfH = height_ * 0.5f;

    const float pLeft   = pPos.x - pHalfW;
    const float pRight  = pPos.x + pHalfW;
    const float pBottom = pPos.y - pHalfH;
    const float pTop    = pPos.y + pHalfH;

    const float eLeft   = position_.x - eHalfW;
    const float eRight  = position_.x + eHalfW;
    const float eBottom = position_.y - eHalfH;
    const float eTop    = position_.y + eHalfH;

    const bool overlapX = !(pRight <= eLeft || pLeft >= eRight);
    const bool overlapY = !(pTop <= eBottom || pBottom >= eTop);
    out.overlap = overlapX && overlapY;
    if (!out.overlap) { return out; }

    // 「踏みつけ」: プレイヤーが下向きに移動しており、かつプレイヤーの下端が敵の上端付近にある（margin を許容）
    const bool falling = (pVel.y < -0.05f);
    const bool fromAbove = (pBottom >= (eTop - stompMargin));
    out.stomp = falling && fromAbove;

    return out;
}
