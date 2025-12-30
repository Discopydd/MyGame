#include "Enemy.h"

#include "ModelManager.h"
#include "../map/MapChipField.h"
#include "../player/Player.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace {
    inline bool IsSolid(MapChipType t) {
        return t == MapChipType::kBlock || t == MapChipType::kBlock2;
    }
    constexpr float kPi = 3.14159265358979323846f;

    inline float StepScale(float dt) {
        // 以 60fps 为基准；防止卡顿一帧太大直接穿墙，做个上限
        float s = dt * 60.0f;
        if (s < 0.0f) s = 0.0f;
        if (s > 3.0f) s = 3.0f;
        return s;
    }
} // namespace

void Enemy::Initialize(
    Object3dCommon* common,
    Camera* camera,
    const Vector3& spawnPos,
    EnemyType type
) {
    type_     = type;
    position_ = spawnPos;

    // ===== 生命值 =====
    isDead_ = false;
    stompInvuln_ = 0.0f;
    if (type == EnemyType::Boss) {
        maxHp_ = 30;
        // 残血阈值：20%（30 -> 6）
        enrageHp_ = (std::max)(1, maxHp_ / 5);
        hp_ = maxHp_;
    } else {
        maxHp_ = 1;
        enrageHp_ = 0;
        hp_ = 1;
    }
    // 状态复位
    isHitReacting_ = false;
    hitReactTimer_ = 0.0f;
    damageBlinkTimer_ = 0.0f;
    damageBlinkVisible_ = true;

    velocity_ = { 0.0f, 0.0f, 0.0f };
    isOnGround_ = false;
    facing_ = 1;
    attackFacing_ = 1;

    bossState_ = BossState::Idle;
    queuedAttack_ = BossAttack::None;
    stateTimer_ = 0.0f;
    decisionTimer_ = 0.0f;


    // Dash 参数复位
    queuedDashDuration_ = 0.30f;
    queuedDashSpeed_    = dashSpeed_;
    isShortDash_        = false;
    microDashCD_        = 0.0f;
    globalAttackCD_ = 0.0f;
    meleeCD_ = 0.0f;
    dashCD_ = 0.0f;
    rangedCD_ = 0.0f;

    ultimateCD_ = 2.0f;
    ultimateBounces_ = 0;

    shotsLeft_ = 0;
    shotsTotal_ = 0;
    shotTimer_ = 0.0f;

    obj_ = std::make_unique<Object3d>();
    obj_->Initialize(common);
    obj_->SetCamera(camera);

    // 默认体积
    width_  = 1.5f;
    height_ = 1.5f;

    // 根据敌人类型切换模型（路径按你资源改）
    switch (type_) {
    case EnemyType::Type0:
        obj_->SetModel("enemy0/enemy0.obj");
        break;
    case EnemyType::Type1:
        obj_->SetModel("enemy1/enemy1.obj");
        break;
    case EnemyType::Boss:
        obj_->SetModel("enemy1/enemy1.obj"); // 没有就换成你已有的模型路径
        width_  = 2.6f;
        height_ = 3.0f;
        bossState_ = BossState::Idle;
        queuedAttack_ = BossAttack::None;

        // 预生成 Boss 弹幕对象池
        projectiles_.clear();
        projectiles_.resize(kMaxBossProjectiles_);
        for (auto& p : projectiles_) {
            p.obj = std::make_unique<Object3d>();
            p.obj->Initialize(common);
            p.obj->SetCamera(camera);
            // 用现有模型顶一下（你也可以换成 fireball.obj 之类）
            p.obj->SetModel("star/star.obj");
            p.pos = spawnPos;
            p.obj->SetTranslate(p.pos);
            p.obj->Update();
            p.active = false;
            p.life = 0.0f;
            p.radius = 0.35f;
        }
        break;
    }

    obj_->SetTranslate(position_);
    // 初始朝向：右=0，左=PI（和 Player 逻辑一致）
    obj_->SetRotate({ 0.0f, 0.0f, 0.0f });
    obj_->Update();
}

void Enemy::Update(float dt, const MapChipField& map, const Player& player)
{
    if (isDead_) { return; }

    // ===== 受击闪烁 =====
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

    // 踩头无敌时间（Boss 用）
    stompInvuln_ = (std::max)(0.0f, stompInvuln_ - dt);

    // ===== Boss AI =====
    if (type_ == EnemyType::Boss) {

        // 先更新弹幕（不依赖 Boss 身体是否与玩家重叠）
        UpdateBossProjectiles(dt, map);

        // ---- 朝向（会在 Dash/Melee/Windup/Shoot 时锁定，避免抽风转身）----
        UpdateBossFacing(player);

        // ---- 重力 ----
        velocity_.y += gravityBase_ * dt;
        if (velocity_.y < -2.5f) { velocity_.y = -2.5f; }

        // ---- 感知 ----
        const Vector3 pPos = player.GetPosition();
        const Vector3 pVel = player.GetVelocity();

        float dx = pPos.x - position_.x;
        float dist = std::fabs(dx);
        const bool playerMovingAway = (pVel.x * facing_ > 0.05f);      // 玩家朝“远离Boss”的方向跑
        const bool distIncreasing = (dist > prevDistToPlayer_ + 0.08f); // 距离在变大（风筝）
        prevDistToPlayer_ = dist;
        // ---- 计时器 ----
        decisionTimer_ = (std::max)(0.0f, decisionTimer_ - dt);
        stateTimer_ = (std::max)(0.0f, stateTimer_ - dt);

        meleeCD_ = (std::max)(0.0f, meleeCD_ - dt);
        dashCD_ = (std::max)(0.0f, dashCD_ - dt);
        microDashCD_ = (std::max)(0.0f, microDashCD_ - dt);
        rangedCD_ = (std::max)(0.0f, rangedCD_ - dt);
        globalAttackCD_ = (std::max)(0.0f, globalAttackCD_ - dt);
        ultimateCD_ = (std::max)(0.0f, ultimateCD_ - dt);

        switch (bossState_) {

        case BossState::Stunned:
            // 被踩后硬直：给玩家二段/三段踩的窗口
            velocity_.x = 0.0f;
            if (stateTimer_ <= 0.0f) {
                bossState_ = BossState::Chase;
                decisionTimer_ = 0.12f;
            }
            break;

        case BossState::Idle:
            velocity_.x = 0.0f;
            if (dist < detectRange_) {
                bossState_ = BossState::Chase;
                decisionTimer_ = 0.0f;
            }
            break;

        case BossState::Chase:
        {
            // 预判目标点：更“像 AI”
            float targetX = pPos.x + pVel.x * leadTime_;

            // 接近时别后撤（否则玩家很难“贴近触发冲刺”）
            float dx2 = targetX - position_.x;
            float absDx2 = std::fabs(dx2);
            int dirToTarget = (dx2 >= 0.0f) ? 1 : -1;

            const float deadZone = 0.35f; // 防抖
            if (absDx2 > idealRange_ + deadZone) {
                velocity_.x = dirToTarget * moveSpeed_;
            }
            else if (absDx2 < idealRange_ - deadZone) {
                velocity_.x = 0.0f;
            }
            else {
                // 距离刚好：小停顿读招
                velocity_.x = 0.0f;
            }

            // 简单“跳障碍”（前方一格是墙就跳）
            if (isOnGround_) {
                int moveDir = facing_;
                if (velocity_.x > 0.01f) moveDir = 1;
                if (velocity_.x < -0.01f) moveDir = -1;

                float checkX = position_.x + moveDir * (width_ * 0.5f + 0.15f);
                float checkY = position_.y - height_ * 0.5f + 0.10f;

                auto idx = map.GetMapChipIndexByPosition({ checkX, checkY, 0.0f });
                if (IsSolid(map.GetMapChipTypeByIndex(idx.xIndex, idx.yIndex))) {
                    velocity_.y = jumpVel_;
                }
            }

            // 进攻选择（用 queuedAttack_ 避免 Windup 里“状态被覆盖”）
            if (decisionTimer_ <= 0.0f && globalAttackCD_ <= 0.0f) {
                const bool canDashAny = (dashCD_ <= 0.0f);

                // 真正“贴脸”的距离：只在这个距离内禁止远程/改用小冲刺
                const float pointBlankRange = meleeRange_ + 0.4f; // 约 2.6
                const bool  tooCloseForRanged = (dist <= pointBlankRange);

                const bool canRanged = (!tooCloseForRanged && dist >= rangedMinRange_ && dist <= rangedMaxRange_ && rangedCD_ <= 0.0f);
                const bool canUltimate = (ultimateCD_ <= 0.0f);

                if (canUltimate && dist >= 2.0f && dist <= 14.0f) {
                    queuedAttack_ = BossAttack::Ultimate;
                    bossState_ = BossState::Windup;
                    attackFacing_ = facing_;
                    stateTimer_ = 0.40f; // 大招前摇更明显

                    // 先把 CD 拉高，等大招结束再重新设置一个随机 CD
                    ultimateCD_ = 399.0f;
                    ultimateLocked_ = true;
                    globalAttackCD_ = 1.10f;
                    decisionTimer_ = 0.35f;
                }
                // ② 远距离：优先远程压制
                else if (canRanged && dist >= farRangedPrefer_ && dist > dashMaxRange_) {
                    queuedAttack_ = BossAttack::Ranged;
                    bossState_ = BossState::Windup;
                    attackFacing_ = facing_;
                    stateTimer_ = 0.28f;

                    rangedCD_ = (hp_ <= enrageHp_) ? 1.25f : 1.75f;
                    globalAttackCD_ = 0.85f;
                    decisionTimer_ = 0.25f;
                }
                // ③ 玩家贴近：朝玩家冲刺（但不会一直冲——靠 dashCD_ 控制）
                else if (canDashAny && dist <= closeDashRange_) {
                    queuedAttack_ = BossAttack::Dash;
                    bossState_ = BossState::Windup;
                    attackFacing_ = facing_;
                    stateTimer_ = closeDashWindup_;
                    queuedDashDuration_ = closeDashDuration_;
                    queuedDashSpeed_ = closeDashSpeed_;
                    isShortDash_ = true;
                    microDashCD_ = microDashCooldown_;

                    dashCD_ = 2.70f;
                    globalAttackCD_ = 1.05f;
                    decisionTimer_ = 0.25f;
                }
                // ③.5 dashCD_ 还在转：但玩家贴脸时别用远程，改为一次小冲刺
                else if (!canDashAny && dist <= closeDashRange_ && microDashCD_ <= 0.0f) {
                    queuedAttack_ = BossAttack::Dash;
                    bossState_ = BossState::Windup;
                    attackFacing_ = facing_;
                    stateTimer_ = closeDashWindup_;
                    queuedDashDuration_ = closeDashDuration_;
                    queuedDashSpeed_ = closeDashSpeed_;
                    isShortDash_ = true;

                    microDashCD_ = microDashCooldown_;
                    globalAttackCD_ = 0.95f;
                    decisionTimer_ = 0.20f;
                }

                // ④ 中距离：择机冲刺（玩家在风筝/拉开距离时更容易触发）
                else if (canDashAny && dist >= dashMinRange_ && dist <= dashMaxRange_
                    && (playerMovingAway || distIncreasing || dist <= 7.0f)) {
                    queuedAttack_ = BossAttack::Dash;
                    bossState_ = BossState::Windup;
                    attackFacing_ = facing_;
                    stateTimer_ = 0.30f;
                    queuedDashDuration_ = 0.30f;
                    queuedDashSpeed_ = dashSpeed_;
                    isShortDash_ = false;

                    dashCD_ = 2.10f;
                    globalAttackCD_ = 1.00f;
                    decisionTimer_ = 0.25f;
                }
                // ⑤ 兜底：还能远程就远程
                else if (canRanged) {
                    queuedAttack_ = BossAttack::Ranged;
                    bossState_ = BossState::Windup;
                    attackFacing_ = facing_;
                    stateTimer_ = 0.28f;

                    rangedCD_ = (hp_ <= enrageHp_) ? 1.25f : 1.75f;
                    globalAttackCD_ = 0.85f;
                    decisionTimer_ = 0.25f;
                }

            }
            break;
        }

        case BossState::Windup:
            // 前摇：停一下给玩家读招
            velocity_.x = 0.0f;
            facing_ = attackFacing_;

            if (stateTimer_ <= 0.0f) {
                if (queuedAttack_ == BossAttack::Dash) {
                    bossState_ = BossState::Dash;
                    stateTimer_ = queuedDashDuration_;
                }
                else if (queuedAttack_ == BossAttack::Ranged) {
                    // 玩家贴脸：不要近距离弹幕（反应不过来），改为向前小冲刺
                    const float pointBlankRange = meleeRange_ + 0.4f;
                    if (dist <= pointBlankRange) {
                        bossState_ = BossState::Dash;
                        queuedDashDuration_ = closeDashDuration_;
                        queuedDashSpeed_ = closeDashSpeed_;
                        isShortDash_ = true;
                        microDashCD_ = microDashCooldown_;
                        stateTimer_ = queuedDashDuration_;
                        attackFacing_ = facing_;
                    }
                    else {
                        bossState_ = BossState::Shoot;

                        // Shoot 状态时长，略长一点方便连射/读招
                        stateTimer_ = (hp_ <= enrageHp_) ? 0.85f : 0.55f;

                        shotsTotal_ = (hp_ <= enrageHp_) ? 3 : 1;
                        shotsLeft_ = shotsTotal_;
                        shotTimer_ = 0.0f; // 立即发射
                    }
                }
                else if (queuedAttack_ == BossAttack::Ultimate) {
                    bossState_ = BossState::Ultimate;
                    stateTimer_ = ultimateDuration_;
                    ultimateBounces_ = 0;
                    // 大招开始就锁一个方向：先朝玩家那边冲
                    attackFacing_ = (pPos.x - position_.x >= 0.0f) ? -1 : 1; // 与玩家相反方向
                    facing_ = attackFacing_;
                }
                else {
                    bossState_ = BossState::Chase;
                }
                queuedAttack_ = BossAttack::None;
            }
            break;

        case BossState::Dash:
            velocity_.x = attackFacing_ * queuedDashSpeed_;
            if (stateTimer_ <= 0.0f) {
                bossState_ = BossState::Recover;
                stateTimer_ = isShortDash_ ? 0.45f : 0.65f;
                isShortDash_ = false;
            }
            break;

        case BossState::Ultimate:
        {
            // 大招：左右来回冲刺
            velocity_.x = attackFacing_ * ultimateSpeed_;

            // failsafe：最长持续
            if (stateTimer_ <= 0.0f) {
                bossState_ = BossState::Rest;
                stateTimer_ = restDuration_;

                FinishUltimateCooldown();
                ultimateLocked_ = false;
            }
            break;
        }

        case BossState::Rest:
            // 大招后休息几秒
            velocity_.x = 0.0f;
            if (stateTimer_ <= 0.0f) {
                bossState_ = BossState::Chase;
                decisionTimer_ = 0.25f;
            }
            break;

        case BossState::Shoot:
        {
            const float pointBlankRange = meleeRange_ + 0.4f;
            // 玩家贴脸：不要继续射击，改为小冲刺
            if (dist <= pointBlankRange) {
                bossState_ = BossState::Dash;
                queuedDashSpeed_ = closeDashSpeed_;
                isShortDash_ = true;
                microDashCD_ = microDashCooldown_;
                attackFacing_ = facing_;
                stateTimer_ = closeDashDuration_;
                shotsLeft_ = 0;
                break;
            }


            // 远程射击：站定瞄准
            velocity_.x = 0.0f;
            facing_ = attackFacing_;

            // 连射计时
            shotTimer_ -= dt;

            if (shotsLeft_ > 0 && shotTimer_ <= 0.0f) {
                // 计算弹幕出生点
                Vector3 spawn = position_;
                spawn.x += attackFacing_ * (width_ * 0.5f + 0.25f);
                spawn.y += height_ * 0.15f;

                // 预测目标
                Vector3 aimTarget{ pPos.x + pVel.x * projectileLeadTime_, pPos.y + pVel.y * projectileLeadTime_, 0.0f };

                // 三连发：给一点纵向散射
                int shotIndex = shotsTotal_ - shotsLeft_; // 0..shotsTotal_-1
                if (shotsTotal_ >= 3) {
                    aimTarget.y += (shotIndex - 1) * 0.55f;
                }

                Vector3 dir{ aimTarget.x - spawn.x, aimTarget.y - spawn.y, 0.0f };
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len < 0.001f) {
                    dir = { static_cast<float>(attackFacing_), 0.0f, 0.0f };
                }
                else {
                    dir.x /= len;
                    dir.y /= len;
                }

                SpawnBossProjectile(spawn, dir);

                shotsLeft_--;
                shotTimer_ = shotInterval_;
            }

            if (stateTimer_ <= 0.0f) {
                bossState_ = BossState::Recover;
                stateTimer_ = 0.55f;
            }
            break;
        }

        case BossState::Recover:
            // 后摇：不要无限连，给玩家反击窗口
            velocity_.x = 0.0f;
            if (stateTimer_ <= 0.0f) {
                bossState_ = BossState::Chase;
                decisionTimer_ = 0.12f;
            }
            break;

        default:
            bossState_ = BossState::Chase;
            break;
        }

        // 地图碰撞：按“先X后Y扫格子修正”
        ResolveMapCollision(map, dt);

        // Dash：撞墙惩罚
        if (bossState_ == BossState::Dash && std::fabs(velocity_.x) < 0.0001f && stateTimer_ > 0.0f) {
            bossState_ = BossState::Recover;
            stateTimer_ = 0.45f; // 撞墙惩罚窗口
        }

        // Ultimate：碰到方块/地刺就算“反弹/触发”，来回几次后进入休息
        if (bossState_ == BossState::Ultimate) {
            // 地刺判定：脚下那格是刺就立刻结束大招
            {
                Vector3 foot = position_;
                foot.y -= height_ * 0.5f - 0.05f;
                auto idx = map.GetMapChipIndexByPosition(foot);
                MapChipType t = map.GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
                if (t == MapChipType::kSpike) {
                    bossState_ = BossState::Rest;
                    stateTimer_ = restDuration_;
                    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                    if (hp_ <= enrageHp_) { ultimateCD_ = 2.8f + 1.4f * r; }
                    else { ultimateCD_ = 4.0f + 2.0f * r; }
                }
            }

            // 方块：X 方向被撞停（velocity_.x==0）视为一次反弹
            if (bossState_ == BossState::Ultimate && std::fabs(velocity_.x) < 0.0001f) {
                ultimateBounces_++;
                attackFacing_ *= -1;
                facing_ = attackFacing_;

                if (ultimateBounces_ >= ultimateMaxBounces_) {
                    bossState_ = BossState::Rest;
                    stateTimer_ = restDuration_;
                    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                    if (hp_ <= enrageHp_) { ultimateCD_ = 2.8f + 1.4f * r; }
                    else { ultimateCD_ = 4.0f + 2.0f * r; }
                }
            }
        }
    }
    // 如果大招被外部逻辑打断（比如碰到玩家触发受击/硬直），确保 CD 不会卡死在 399
    if (ultimateLocked_ && bossState_ != BossState::Ultimate && ultimateCD_ > 100.0f) {
        FinishUltimateCooldown();
        ultimateLocked_ = false;
    }
    // ===== 渲染更新 =====
    if (obj_) {
        // Boss 的模型朝向左
        if (type_ == EnemyType::Boss) {
            const float rotY = (facing_ >= 0) ? kPi : 0.0f;
            obj_->SetRotate({ 0.0f, rotY, 0.0f });
        }
        obj_->SetTranslate(position_);
        obj_->Update();
    }
}

void Enemy::Draw()
{
    if (isDead_) { return; }

    // Boss 弹幕不跟着 Boss 闪烁隐藏（否则玩家会感觉“子弹消失了”）
    if (type_ == EnemyType::Boss) {
        DrawBossProjectiles();
    }

    if (!obj_) { return; }

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


void Enemy::OnStomp()
{
    if (isDead_) { return; }

    // 防止同一帧/同一次重叠反复扣血
    if (stompInvuln_ > 0.0f) { return; }

    if (type_ == EnemyType::Boss) {
        // Boss：踩头 30 次死亡（默认）
        hp_ -= 1;

        // 受击反馈：闪烁 + 短硬直（更容易连续踩）
        StartHitReaction(0.35f);
        bossState_ = BossState::Stunned;
        stateTimer_ = 0.60f;
        decisionTimer_ = 0.20f;

        // 被踩后先别立刻反打
        globalAttackCD_ = (std::max)(globalAttackCD_, 0.80f);

        stompInvuln_ = 0.90f;

        if (hp_ <= 0) {
            isDead_ = true;
            // 死亡时清掉弹幕
            for (auto& p : projectiles_) {
                p.active = false;
                p.life = 0.0f;
            }
        }
    }
    else {
        // 普通敌人：先保留原行为（只闪一下）
        StartHitReaction(0.40f);
        stompInvuln_ = 0.20f;
    }
}

void Enemy::ResolveMapCollision(const MapChipField& map, float dt)
{
    const float step = StepScale(dt);
    const float kHalfW = width_  * 0.5f;
    const float kHalfH = height_ * 0.5f;

    bool onGround = false;

    auto isSolid = [](MapChipType t) {
        return t == MapChipType::kBlock || t == MapChipType::kBlock2;
    };

    // ---- X ----
    {
                float nextX = position_.x + velocity_.x * step;
        float left   = nextX - kHalfW;
        float right  = nextX + kHalfW;
        float bottom = position_.y - kHalfH;
        float top    = position_.y + kHalfH;

        auto minIdx = map.GetMapChipIndexByPosition({ left,  bottom, 0.0f });
        auto maxIdx = map.GetMapChipIndexByPosition({ right, top,    0.0f });

        bool  hitX = false;
        float fixX = nextX;

        for (uint32_t y = minIdx.yIndex; y <= maxIdx.yIndex; ++y) {
            for (uint32_t x = minIdx.xIndex; x <= maxIdx.xIndex; ++x) {
                MapChipType t = map.GetMapChipTypeByIndex(x, y);
                if (!isSolid(t)) { continue; }

                auto r = map.GetRectByIndex(x, y);

                bool overlapY = !(top <= r.bottom || bottom >= r.top);
                bool overlapX = !(right <= r.left || left >= r.right);
                if (overlapX && overlapY) {
                    hitX = true;
                    if (velocity_.x > 0.0f)      { fixX = r.left  - kHalfW; }
                    else if (velocity_.x < 0.0f) { fixX = r.right + kHalfW; }
                }
            }
        }

        position_.x = hitX ? fixX : nextX;
        if (hitX) { velocity_.x = 0.0f; }
    }

    // ---- Y ----
    {
                float nextY = position_.y + velocity_.y * step;
        float left   = position_.x - kHalfW;
        float right  = position_.x + kHalfW;
        float bottom = nextY - kHalfH;
        float top    = nextY + kHalfH;

        auto minIdx = map.GetMapChipIndexByPosition({ left,  bottom, 0.0f });
        auto maxIdx = map.GetMapChipIndexByPosition({ right, top,    0.0f });

        bool  hitY = false;
        float fixY = nextY;

        for (uint32_t y = minIdx.yIndex; y <= maxIdx.yIndex; ++y) {
            for (uint32_t x = minIdx.xIndex; x <= maxIdx.xIndex; ++x) {
                MapChipType t = map.GetMapChipTypeByIndex(x, y);
                if (!isSolid(t)) { continue; }

                auto r = map.GetRectByIndex(x, y);

                bool overlapX = !(right <= r.left || left >= r.right);
                bool overlapY = !(top   <= r.bottom || bottom >= r.top);
                if (overlapX && overlapY) {
                    hitY = true;
                    if (velocity_.y > 0.0f) {          // 顶头
                        fixY = r.bottom - kHalfH;
                        velocity_.y = 0.0f;
                    } else if (velocity_.y < 0.0f) {   // 落地
                        fixY = r.top + kHalfH;
                        velocity_.y = 0.0f;
                        onGround = true;
                    }
                }
            }
        }

        position_.y = hitY ? fixY : nextY;
    }

    isOnGround_ = onGround;
}

void Enemy::UpdateBossFacing(const Player& player)
{
    // 攻击/前摇/射击时锁向
    if (bossState_ == BossState::Dash ||
        bossState_ == BossState::Windup ||
        bossState_ == BossState::Shoot ||
        bossState_ == BossState::Ultimate) {
        return;
    }

    const float dx = player.GetPosition().x - position_.x;

    // 防抖
    if (std::fabs(dx) <= 0.05f) {
        return;
    }

    facing_ = (dx >= 0.0f) ? 1 : -1;
}

void Enemy::UpdateBossProjectiles(float dt, const MapChipField& map)
{
    if (type_ != EnemyType::Boss) { return; }
    const float step = StepScale(dt);

    for (auto& p : projectiles_) {
        if (!p.active) { continue; }

        p.life -= dt;
        if (p.life <= 0.0f) {
            p.active = false;
            continue;
        }

        Vector3 next = p.pos;
                next.x += p.vel.x * step;
        next.y += p.vel.y * step;
        // 碰到方块就消失
        auto idx = map.GetMapChipIndexByPosition(next);
        if (IsSolid(map.GetMapChipTypeByIndex(idx.xIndex, idx.yIndex))) {
            p.active = false;
            continue;
        }

        p.pos = next;
        if (p.obj) {
            p.obj->SetTranslate(p.pos);
            p.obj->Update();
        }
    }
}

void Enemy::SpawnBossProjectile(const Vector3& spawnPos, const Vector3& aimDir)
{
    // 找一个空槽
    for (auto& p : projectiles_) {
        if (p.active) { continue; }

        p.active = true;
        p.life = projectileLife_;
        p.pos = spawnPos;
        p.vel = { aimDir.x * projectileSpeed_, aimDir.y * projectileSpeed_, 0.0f };

        if (p.obj) {
            p.obj->SetTranslate(p.pos);
            p.obj->Update();
        }
        return;
    }

    // 全满则丢弃（避免无限增长）
}

void Enemy::DrawBossProjectiles()
{
    if (type_ != EnemyType::Boss) { return; }

    for (auto& p : projectiles_) {
        if (!p.active || !p.obj) { continue; }
        p.obj->Draw();
    }
}

void Enemy::FinishUltimateCooldown()
{
    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    if (hp_ <= enrageHp_) { ultimateCD_ = 2.8f + 1.4f * r; }
    else                 { ultimateCD_ = 4.0f + 2.0f * r; }
}

bool Enemy::CheckBossProjectileHit(const Player& player)
{
    if (type_ != EnemyType::Boss) { return false; }

    Vector3 pPos = player.GetPosition();
    float   pHalfW = player.GetWidth() * 0.5f;
    float   pHalfH = player.GetHeight() * 0.5f;

    float pLeft   = pPos.x - pHalfW;
    float pRight  = pPos.x + pHalfW;
    float pBottom = pPos.y - pHalfH;
    float pTop    = pPos.y + pHalfH;

    for (auto& pr : projectiles_) {
        if (!pr.active) { continue; }

        float r = pr.radius;
        float left   = pr.pos.x - r;
        float right  = pr.pos.x + r;
        float bottom = pr.pos.y - r;
        float top    = pr.pos.y + r;

        bool overlapX = !(pRight <= left || pLeft >= right);
        bool overlapY = !(pTop <= bottom || pBottom >= top);
        if (overlapX && overlapY) {
            pr.active = false; // 命中后消耗弹丸
            return true;
        }
    }

    return false;
}
