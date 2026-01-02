#include "NormalEnemy.h"

#include "ModelManager.h"
#include "../map/MapChipField.h"
#include "../player/Player.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace {
    inline bool IsSolid(MapChipType t) {
        return t == MapChipType::kBlock
        || t == MapChipType::kBlock2
        || t == MapChipType::kSpike
        || t == MapChipType::kMoveHorizontal
        || t == MapChipType::kMoveVertical;
    }
    constexpr float kPi = 3.14159265358979323846f;

    inline float StepScale(float dt) {
        // 以 60fps 为基准；防止卡顿一帧太大直接穿墙，做个上限
        float s = dt * 60.0f;
        if (s < 0.0f) s = 0.0f;
        if (s > 3.0f) s = 3.0f;
        return s;
    }
    inline bool HitSpikeUnderFoot(const Vector3& pos, float width, float height, const MapChipField& map)
    {
        const float halfW = width * 0.5f;
        const float halfH = height * 0.5f;

        // 向下探一点点，确保“刚好贴地（bottom==tileTop）”也能判到地刺
        const float probeY = pos.y - halfH - 0.08f;

        // 3 点采样：中/左脚/右脚（避免只采中心，脚踩到边缘刺却漏判）
        const float xs[3] = {
            pos.x,
            pos.x - halfW * 0.65f,
            pos.x + halfW * 0.65f
        };

        for (float x : xs) {
            auto idx = map.GetMapChipIndexByPosition({ x, probeY, 0.0f });
            if (map.GetMapChipTypeByIndex(idx.xIndex, idx.yIndex) == MapChipType::kSpike) {
                return true;
            }
        }
        return false;
    }

    // BossEnemy::ResolveMapCollision 的“简化复用版”
    inline void ResolveMapCollision(
        Vector3& pos,
        Vector3& vel,
        bool& isOnGround,
        float width,
        float height,
        const MapChipField& map,
        float dt
    ) {
        const float step = StepScale(dt);
        const float halfW = width * 0.5f;
        const float halfH = height * 0.5f;

        bool onGround = false;

        // ---- X ----
        {
            float nextX = pos.x + vel.x * step;
            float left = nextX - halfW;
            float right = nextX + halfW;
            float bottom = pos.y - halfH;
            float top = pos.y + halfH;

            auto minIdx = map.GetMapChipIndexByPosition({ left, bottom, 0.0f });
            auto maxIdx = map.GetMapChipIndexByPosition({ right, top, 0.0f });

            bool  hitX = false;
            float fixX = nextX;

            for (uint32_t y = minIdx.yIndex; y <= maxIdx.yIndex; ++y) {
                for (uint32_t x = minIdx.xIndex; x <= maxIdx.xIndex; ++x) {
                    MapChipType t = map.GetMapChipTypeByIndex(x, y);
                    if (!IsSolid(t)) { continue; }

                    auto r = map.GetRectByIndex(x, y);

                    bool overlapY = !(top <= r.bottom || bottom >= r.top);
                    bool overlapX = !(right <= r.left || left >= r.right);
                    if (overlapX && overlapY) {
                        hitX = true;
                        if (vel.x > 0.0f) { fixX = r.left - halfW; }
                        else if (vel.x < 0.0f) { fixX = r.right + halfW; }
                    }
                }
            }

            pos.x = hitX ? fixX : nextX;
            if (hitX) { vel.x = 0.0f; }
        }

        // ---- Y ----
        {
            float nextY = pos.y + vel.y * step;
            float left = pos.x - halfW;
            float right = pos.x + halfW;
            float bottom = nextY - halfH;
            float top = nextY + halfH;

            auto minIdx = map.GetMapChipIndexByPosition({ left, bottom, 0.0f });
            auto maxIdx = map.GetMapChipIndexByPosition({ right, top, 0.0f });

            bool  hitY = false;
            float fixY = nextY;

            for (uint32_t y = minIdx.yIndex; y <= maxIdx.yIndex; ++y) {
                for (uint32_t x = minIdx.xIndex; x <= maxIdx.xIndex; ++x) {
                    MapChipType t = map.GetMapChipTypeByIndex(x, y);
                    if (!IsSolid(t)) { continue; }

                    auto r = map.GetRectByIndex(x, y);

                    bool overlapX = !(right <= r.left || left >= r.right);
                    bool overlapY = !(top <= r.bottom || bottom >= r.top);
                    if (overlapX && overlapY) {
                        hitY = true;
                        if (vel.y > 0.0f) {          // 顶头
                            fixY = r.bottom - halfH;
                            vel.y = 0.0f;
                        }
                        else if (vel.y < 0.0f) {     // 落地
                            fixY = r.top + halfH;
                            vel.y = 0.0f;
                            onGround = true;
                        }
                    }
                }
            }

            pos.y = hitY ? fixY : nextY;
        }

        isOnGround = onGround;
    }
} // namespace

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

    // 移动/死亡动画状态
    velocity_ = { 0.0f, 0.0f, 0.0f };
    isOnGround_ = false;
    facing_ = (std::rand() % 2 == 0) ? 1 : -1;
    isDying_ = false;
    deathTimer_ = 0.0f;
    deathSpin_ = 0.0f;

    // 记录“存活时碰撞尺寸”（用于死亡动画期间与地图碰撞）
    aliveWidth_ = width_;
    aliveHeight_ = height_;
}

void NormalEnemy::Update(float dt, const MapChipField& map, const Player& player)
{
    // 已经彻底死亡（动画结束）
    if (isDead_) { return; }

    // ===== 死亡动画更新 =====
    if (isDying_) {
        deathTimer_ = (std::max)(0.0f, deathTimer_ - dt);

        // 简单“弹起+旋转+下落”
        velocity_.y += gravity_ * dt;
        if (velocity_.y < -2.5f) { velocity_.y = -2.5f; }

        // 仍然与地形做碰撞，避免死亡动画时穿过地面
        ResolveMapCollision(position_, velocity_, isOnGround_, aliveWidth_, aliveHeight_, map, dt);

        deathSpin_ += deathSpinSpeed_ * dt;

        if (obj_) {
            const float rotY = (facing_ >= 0) ? 0.0f : kPi;
            obj_->SetRotate({ 0.0f, rotY, deathSpin_ });
            obj_->SetTranslate(position_);
            obj_->Update();
        }

        if (deathTimer_ <= 0.0f) {
            isDead_ = true;
        }
        return;
    }

    // ===== 存活状态 =====
    if (!UpdateCommon(dt)) { return; }

    // 重力
    velocity_.y += gravity_ * dt;
    if (velocity_.y < -2.5f) { velocity_.y = -2.5f; }

    // 行走：左右巡逻
    velocity_.x = static_cast<float>(facing_) * moveSpeed_;

    // 悬崖掉头：前方脚下没有方块就换方向
    if (isOnGround_) {
        float checkX = position_.x + static_cast<float>(facing_) * (width_ * 0.5f + 0.20f);
        float checkY = position_.y - height_ * 0.5f - 0.15f;
        auto idx = map.GetMapChipIndexByPosition({ checkX, checkY, 0.0f });
        if (!IsSolid(map.GetMapChipTypeByIndex(idx.xIndex, idx.yIndex))) {
            facing_ *= -1;
            velocity_.x = static_cast<float>(facing_) * moveSpeed_;
        }
    }

    // Map 碰撞
    const float prevVX = velocity_.x;
    ResolveMapCollision(position_, velocity_, isOnGround_, width_, height_, map, dt);
    // 撞墙掉头
    if (std::fabs(prevVX) > 0.0001f && std::fabs(velocity_.x) < 0.0001f) {
        facing_ *= -1;
    }
    // ===== 地刺判定：踩到就死（进入死亡动画）=====
    if (!isDying_ && HitSpikeUnderFoot(position_, width_, height_, map)) {
        OnStomp();   // 直接复用你现有的“死亡动画流程”
        return;
    }

    // ===== 与玩家碰撞/踩头判定 =====
    // overlap: 玩家与敌人 AABB 重叠
    // stomp:   玩家处于下落且从上方命中敌人顶部
    {
        const auto contact = CheckPlayerContact(player);
        if (contact.overlap && contact.stomp) {
            OnStomp();
        }
        // NOTE: contact.overlap && !contact.stomp 的情况一般是“侧面/底部碰到敌人”
        //       玩家受伤/死亡的处理建议放在 GameScene 或 Player 侧。
    }

    // 渲染更新
    if (obj_) {
        const float rotY = (facing_ >= 0) ? kPi : 0.0f;
        obj_->SetRotate({ 0.0f, rotY, 0.0f });
        obj_->SetTranslate(position_);
        obj_->Update();
    }
}

void NormalEnemy::Draw()
{
    if (isDead_) { return; }
    if (!obj_) { return; }

    // 死亡动画期间不闪烁隐藏（否则看起来像“消失”）
    if (!isDying_ && isHitReacting_ && !damageBlinkVisible_) {
        return;
    }
    obj_->Draw();
}

void NormalEnemy::OnStomp()
{
    if (isDead_ || isDying_) { return; }

    // 一次踩头即死亡
    hp_ = 0;

    // 进入死亡动画：先弹起
    isDying_ = true;
    deathTimer_ = deathDuration_;
    deathSpin_ = 0.0f;

    velocity_.x = 0.0f;
    velocity_.y = 0.75f;
    isOnGround_ = false;

    // 让游戏逻辑上“马上不再产生碰撞”（但仍渲染死亡动画）
    width_ = 0.0f;
    height_ = 0.0f;

    // 也可以保留一点闪烁作为受击反馈（可注释）
    StartHitReaction(0.10f);
}
