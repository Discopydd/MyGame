#include "BossEnemy.h"

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
} // namespace

void BossEnemy::Initialize(
    Object3dCommon* common,
    Camera* camera,
    const Vector3& spawnPos,
    EnemyType type
) {
    type_     = type;
    position_ = spawnPos;

    // Boss：默认先“睡眠”，等玩家进入指定区域再唤醒（见 Update 里的触发条件）
    battleTriggered_ = (type != EnemyType::Boss);

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
    // 默认闪烁频率（Boss 会在下面覆写成更快一点）
    damageBlinkInterval_ = 0.08f;

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
    barrageCD_ = 0.0f;
    slamCD_ = 0.0f;
    novaCD_ = 0.0f;

    barrageFireTimer_ = 0.0f;
    barrageAngle_ = 0.0f;
    barrageSpinDir_ = 1.0f;
    barrageBurstTimer_ = 0.0f;
    slamSpawned_ = false;

    novaFireTimer_ = 0.0f;
    novaRingsLeft_ = 0;
    novaRingOffset_ = 0.0f;

    ultimateCD_ = 2.0f;
    ultimateBounces_ = 0;

    shotsLeft_ = 0;
    shotsTotal_ = 0;
    shotInterval_ = 0.16f;
    shotTimer_ = 0.0f;
    fanShot_ = false;

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
        obj_->SetModel("enemy2/enemy2.obj");
        break;
    case EnemyType::Boss: {
        obj_->SetModel("enemy1/enemy1.obj"); // 没有就换成你已有的模型路径

        // Boss 受击闪烁：频率更快一点，更有“打击感”
        damageBlinkInterval_ = 0.055f;

        // ===== 放大 Boss（模型）+ 增加碰撞体积 =====
        // 说明：如果你的 Object3d 没有 SetScale()，请改成你工程里对应的缩放 API。
        const float kBossScale = 1.35f;

        // 碰撞体积（AABB）跟着放大
        width_  = 2.6f * kBossScale;
        height_ = 3.0f * kBossScale;

        // 模型缩放
        obj_->SetScale({ kBossScale, kBossScale, kBossScale });

        // Boss 受击闪烁更“电流感”：频率略快
        damageBlinkInterval_ = 0.055f;

        // Boss 受击闪烁：更明显一点
        damageBlinkInterval_ = 0.06f;

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
    default:
        break;
    }

    // --- 判定尺寸初始化 ---
    // 以当前 width_/height_ 为“基础玩家判定”与“地图碰撞体积”。
    // Dash 时会临时缩小 width_/height_（玩家判定），但地图碰撞会始终用 mapColliderW_/H_。
    baseHurtW_ = width_;
    baseHurtH_ = height_;
    mapColliderW_ = width_;
    mapColliderH_ = height_;

    obj_->SetTranslate(position_);
    // 初始朝向：右=0，左=PI（和 Player 逻辑一致）
    obj_->SetRotate({ 0.0f, 0.0f, 0.0f });
    obj_->Update();
}

void BossEnemy::Update(float dt, const MapChipField& map, const Player& player)
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
// ===== 战斗触发：玩家越过指定列并“站到地面”后才唤醒 Boss =====
// 默认触发列：AH（0-based=33）。如果你关卡里不是 AH，调用 SetBattleTriggerXIndex() 或直接改 battleTriggerXIndex_。
if (!battleTriggered_) {
    const auto pIdx = map.GetMapChipIndexByPosition(player.GetPosition());
    const bool passedX = (pIdx.xIndex >= battleTriggerXIndex_);
    const bool onGround = (!requirePlayerOnGroundToTrigger_) ? true : IsPlayerOnGround(map, player);

    if (passedX && onGround) {
        battleTriggered_ = true;

        // 刚进入战斗：重置一下节奏，避免“刚触发就立刻贴脸大招”
        bossState_ = BossState::Idle;
        queuedAttack_ = BossAttack::None;
        stateTimer_ = 0.0f;
        decisionTimer_ = 0.30f;
        globalAttackCD_ = (std::max)(globalAttackCD_, 0.60f);

        // 清掉残留弹幕（以防复用对象/读档等情况）
        for (auto& p : projectiles_) {
            p.active = false;
            p.life = 0.0f;
        }
    } else {
        // 未触发：不更新 AI/弹幕，只更新渲染（Boss 可以当作“雕像/待机”）
        if (obj_) {
            obj_->SetTranslate(position_);
            obj_->Update();
        }
        return;
    }
}


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
        barrageCD_ = (std::max)(0.0f, barrageCD_ - dt);
        slamCD_ = (std::max)(0.0f, slamCD_ - dt);
        novaCD_ = (std::max)(0.0f, novaCD_ - dt);
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
            if (IsInEngageRange(map, player)) {
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

                // 地形探测用“身体体积”（不要受 Dash hurtbox 缩放影响）
                float checkX = position_.x + moveDir * (mapColliderW_ * 0.5f + 0.15f);
                float checkY = position_.y - mapColliderH_ * 0.5f + 0.10f;

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
                const bool canBarrage = (!tooCloseForRanged && dist >= 5.5f && dist <= 13.5f && barrageCD_ <= 0.0f);
                const bool playerAbove = (pPos.y > position_.y + height_ * 0.15f);
                const bool canSlam = (isOnGround_ && slamCD_ <= 0.0f && dist <= 6.5f);
                const bool canNova = (!tooCloseForRanged && dist >= 4.8f && dist <= 12.8f && novaCD_ <= 0.0f);

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
                // ② 砸地：克制“跳头顶/贴脸绕圈”，落地会生成冲击波弹幕
                else if (canSlam && (playerAbove || dist <= (meleeRange_ + 1.2f))) {
                    queuedAttack_ = BossAttack::Slam;
                    bossState_ = BossState::Windup;
                    attackFacing_ = facing_;
                    stateTimer_ = slamWindup_;

                    slamCD_ = (hp_ <= enrageHp_) ? 3.0f : 4.2f;
                    globalAttackCD_ = 1.15f;
                    decisionTimer_ = 0.30f;
                }
                // ②.5 圆形爆发：更“炸裂”的环形弹幕（残血优先；也会惩罚玩家一直后撤）
                else if (canNova && (hp_ <= enrageHp_ || (playerMovingAway && dist >= 8.0f) || distIncreasing)) {
                    queuedAttack_ = BossAttack::Nova;
                    bossState_ = BossState::Windup;
                    attackFacing_ = facing_;
                    stateTimer_ = novaWindup_;

                    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                    novaCD_ = (hp_ <= enrageHp_) ? (3.2f + 1.0f * r) : (4.6f + 1.2f * r);
                    globalAttackCD_ = 1.15f;
                    decisionTimer_ = 0.32f;
                }
                // ③ 旋转弹幕：中远距离压制（更炫酷），玩家一直后撤时更容易触发
                else if (canBarrage && (hp_ <= enrageHp_ || playerMovingAway || dist >= farRangedPrefer_)) {
                    queuedAttack_ = BossAttack::Barrage;
                    bossState_ = BossState::Windup;
                    attackFacing_ = facing_;
                    stateTimer_ = 0.30f;

                    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                    barrageCD_ = (hp_ <= enrageHp_) ? (3.0f + 1.0f * r) : (4.0f + 1.2f * r);
                    globalAttackCD_ = 1.05f;
                    decisionTimer_ = 0.30f;
                }
                // ④ 远距离：优先远程压制
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

                        // -------- 射击模式选择 --------
                        // 残血 & 中距离：改为“扇形散射”更炫酷
                        fanShot_ = (hp_ <= enrageHp_) && (dist <= 9.5f);

                        if (fanShot_) {
                            stateTimer_ = 0.95f;
                            shotsTotal_ = 2;           // 两波散射
                            shotInterval_ = 0.22f;
                        }
                        else {
                            // 原有：单发 / 三连发（纵向散射）
                            stateTimer_ = (hp_ <= enrageHp_) ? 0.85f : 0.55f;
                            shotsTotal_ = (hp_ <= enrageHp_) ? 3 : 1;
                            shotInterval_ = 0.16f;
                        }

                        shotsLeft_ = shotsTotal_;
                        shotTimer_ = 0.0f; // 立即发射
                    }
                }
                else if (queuedAttack_ == BossAttack::Barrage) {
                    bossState_ = BossState::Barrage;

                    // 残血时更久、更密集
                    stateTimer_ = (hp_ <= enrageHp_) ? (barrageDuration_ + 0.25f) : barrageDuration_;
                    barrageFireTimer_ = 0.0f;
                    barrageBurstTimer_ = barrageBurstInterval_ * 0.65f;

                    // 角度随机一点，避免每次都一样
                    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                    barrageAngle_ = r * 6.283185307179586f; // 2*pi
                    barrageSpinDir_ = (r < 0.5f) ? 1.0f : -1.0f;
                }
                else if (queuedAttack_ == BossAttack::Nova) {
                    bossState_ = BossState::Nova;

                    // 残血时多一环、更密集
                    // 确保有足够时间把所有环都放完
                    novaFireTimer_ = 0.0f; // 立即释放第一环
                    novaRingsLeft_ = (hp_ <= enrageHp_) ? novaRingsEnrage_ : novaRingsNormal_;
                    stateTimer_ = novaDuration_ + novaRingInterval_ * (std::max)(0, novaRingsLeft_ - 1);

                    // 每次环形弹幕都转一点角度，避免“固定花纹”
                    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                    novaRingOffset_ = r * 6.283185307179586f;
                }
                else if (queuedAttack_ == BossAttack::Slam) {
                    bossState_ = BossState::Jump;
                    stateTimer_ = slamMaxAirTime_;

                    // 起跳（砸地）
                    velocity_.x = 0.0f;
                    velocity_.y = slamJumpVel_;
                    slamSpawned_ = false;
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

        case BossState::Barrage:
        {
            // 旋转弹幕：站定发射，靠角度持续旋转制造“弹幕地狱”效果
            velocity_.x = 0.0f;
            facing_ = attackFacing_;

            // 残血时更密集/旋转更快
            const float fireInterval = (hp_ <= enrageHp_) ? (barrageFireInterval_ * 0.85f) : barrageFireInterval_;
            const float angSpeed = (hp_ <= enrageHp_) ? (barrageAngularSpeed_ * 1.15f) : barrageAngularSpeed_;

            // 追加：周期性“爆环”，让弹幕更有层次
            barrageBurstTimer_ -= dt;
            if (barrageBurstTimer_ <= 0.0f && stateTimer_ > 0.0f) {
                barrageBurstTimer_ += barrageBurstInterval_;
                const int count = (hp_ <= enrageHp_) ? (barrageBurstCount_ + 4) : barrageBurstCount_;
                const float spd = (hp_ <= enrageHp_) ? (barrageBurstSpeed_ * 1.15f) : barrageBurstSpeed_;
                Vector3 c = position_;
                c.y += height_ * 0.18f;
                SpawnRadialBurst(c, count, spd, barrageBurstLife_, 0.28f, barrageAngle_);
            }

            barrageFireTimer_ -= dt;
            while (barrageFireTimer_ <= 0.0f && stateTimer_ > 0.0f) {
                barrageFireTimer_ += fireInterval;

                // 弹幕出生点：胸口略前
                Vector3 spawn = position_;
                spawn.x += attackFacing_ * (width_ * 0.15f);
                spawn.y += height_ * 0.18f;

                // 发射 1~2 发：残血时双发更炫
                const int emitCount = (hp_ <= enrageHp_) ? 2 : 1;
                for (int i = 0; i < emitCount; ++i) {
                    float ang = barrageAngle_ + (i == 0 ? 0.0f : kPi);
                    Vector3 dir{ std::cos(ang), std::sin(ang), 0.0f };
                    Vector3 vel{ dir.x * barrageProjectileSpd_, dir.y * barrageProjectileSpd_, 0.0f };
                    SpawnBossProjectileRaw(spawn, vel, barrageProjectileLife_, 0.32f);
                }

                // 角度推进：按旋转方向旋转
                barrageAngle_ += angSpeed * fireInterval * barrageSpinDir_;
            }

            if (stateTimer_ <= 0.0f) {
                bossState_ = BossState::Recover;
                stateTimer_ = 0.70f;
            }
            break;
        }

        case BossState::Nova:
        {
            // 圆形爆发：多环环形弹幕
            velocity_.x = 0.0f;
            facing_ = attackFacing_;

            const int totalRings = (hp_ <= enrageHp_) ? novaRingsEnrage_ : novaRingsNormal_;

            auto emitRing = [&](int ringIndex) {
                const int count = novaBulletCount_ + ((hp_ <= enrageHp_) ? 2 : 0);
                const float speed = novaProjectileSpd_ * (1.0f + 0.08f * static_cast<float>(ringIndex));
                const float offset = novaRingOffset_ + ((ringIndex % 2 == 0) ? 0.0f : (kPi / (float)(count)));

                Vector3 c = position_;
                c.y += height_ * 0.18f;
                SpawnRadialBurst(c, count, speed, novaProjectileLife_, 0.30f, offset);

                // 第一环额外加“十字”强子弹，画面更炸裂
                if (ringIndex == 0) {
                    const float spd2 = speed * 1.25f;
                    SpawnBossProjectileRaw(c, {  spd2, 0.0f, 0.0f }, novaProjectileLife_, 0.34f);
                    SpawnBossProjectileRaw(c, { -spd2, 0.0f, 0.0f }, novaProjectileLife_, 0.34f);
                    SpawnBossProjectileRaw(c, { 0.0f,  spd2, 0.0f }, novaProjectileLife_, 0.34f);
                    SpawnBossProjectileRaw(c, { 0.0f, -spd2, 0.0f }, novaProjectileLife_, 0.34f);
                }

                novaRingOffset_ += 0.35f;
            };

            // 释放每一环
            novaFireTimer_ -= dt;
            while (novaFireTimer_ <= 0.0f && novaRingsLeft_ > 0) {
                novaFireTimer_ += novaRingInterval_;
                const int ringIndex = totalRings - novaRingsLeft_;
                emitRing(ringIndex);
                novaRingsLeft_--;
            }

            if (stateTimer_ <= 0.0f) {
                // 极端掉帧：一次性补齐剩余环，避免漏放
                while (novaRingsLeft_ > 0) {
                    const int ringIndex = totalRings - novaRingsLeft_;
                    emitRing(ringIndex);
                    novaRingsLeft_--;
                }
                bossState_ = BossState::Recover;
                stateTimer_ = novaRecover_;
            }
            break;
        }

        case BossState::Jump:
            // 跳起砸地：空中不做水平移动（更好读招）
            velocity_.x = 0.0f;
            facing_ = attackFacing_;

            // failsafe：如果空中太久（某些地图/碰撞极端情况），强制进入 Slam
            if (stateTimer_ <= 0.0f) {
                bossState_ = BossState::Slam;
                stateTimer_ = slamImpactHold_;
                slamSpawned_ = false;
            }
            break;

        case BossState::Slam:
        {
            // 落地砸地：停顿一下 + 生成冲击波弹幕
            velocity_.x = 0.0f;
            facing_ = attackFacing_;

            if (!slamSpawned_) {
                slamSpawned_ = true;

                // 冲击波：沿地面左右扩散
                Vector3 base = position_;
                base.y -= height_ * 0.50f - 0.25f;

                SpawnBossProjectileRaw(base, { -slamWaveSpeed_, 0.0f, 0.0f }, slamWaveLife_, 0.36f);
                SpawnBossProjectileRaw(base, {  slamWaveSpeed_, 0.0f, 0.0f }, slamWaveLife_, 0.36f);

                // 破片：向上扇形喷出（更炫酷）
                auto emitShard = [&](float x, float y) {
                    float len = std::sqrt(x * x + y * y);
                    if (len < 0.001f) { len = 1.0f; }
                    Vector3 vel{ (x / len) * slamShardSpeed_, (y / len) * slamShardSpeed_, 0.0f };
                    SpawnBossProjectileRaw(base, vel, slamShardLife_, 0.30f);
                };

                emitShard(-1.00f, 0.85f);
                emitShard(-0.55f, 1.00f);
                emitShard( 0.55f, 1.00f);
                emitShard( 1.00f, 0.85f);

                // 残血再加一层小破片
                if (hp_ <= enrageHp_) {
                    emitShard(-0.30f, 1.20f);
                    emitShard( 0.30f, 1.20f);
                }

                // 追加：落地冲击“爆环”（更炫）
                {
                    Vector3 c = base;
                    c.y += 0.45f; // 抬高一点，避免一出生就撞地面块
                    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                    const int count = (hp_ <= enrageHp_) ? 14 : 12;
                    const float spd = (hp_ <= enrageHp_) ? 0.22f : 0.20f;
                    SpawnRadialBurst(c, count, spd, 1.25f, 0.28f, r * 6.283185307179586f);
                }
            }

            if (stateTimer_ <= 0.0f) {
                bossState_ = BossState::Recover;
                stateTimer_ = slamRecover_;
            }
            break;
        }

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

                // -------- 弹幕模式：扇形散射 / 原有三连发 --------
                // 三连发：给一点纵向散射
                int shotIndex = shotsTotal_ - shotsLeft_; // 0..shotsTotal_-1
                if (!fanShot_ && shotsTotal_ >= 3) {
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

                if (fanShot_) {
                    // 基准角
                    // 第二波略微旋转，视觉更丰富
                    float base = std::atan2(dir.y, dir.x) + 0.18f * static_cast<float>(shotIndex);
                    const int nBase = (fanCount_ < 2) ? 2 : fanCount_;
                    const int n = (hp_ <= enrageHp_) ? (nBase + 2) : nBase;
                    const float start = base - fanHalfAngle_;
                    const float step = (n > 1) ? (2.0f * fanHalfAngle_ / static_cast<float>(n - 1)) : 0.0f;
                    for (int i = 0; i < n; ++i) {
                        float a = start + step * static_cast<float>(i);
                        Vector3 d{ std::cos(a), std::sin(a), 0.0f };
                        Vector3 vel{ d.x * fanProjectileSpd_, d.y * fanProjectileSpd_, 0.0f };
                        SpawnBossProjectileRaw(spawn, vel, projectileLife_, 0.32f);
                    }
                }
                else {
                    SpawnBossProjectile(spawn, dir);
                }

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
        const bool wasOnGround = isOnGround_;
        ResolveMapCollision(map, dt);

        // Jump -> Slam：检测落地瞬间触发砸地
        if (bossState_ == BossState::Jump && !wasOnGround && isOnGround_) {
            bossState_ = BossState::Slam;
            stateTimer_ = slamImpactHold_;
            slamSpawned_ = false;
        }

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
                // 地形判定用身体体积（不要受 Dash hurtbox 缩放影响）
                foot.y -= mapColliderH_ * 0.5f - 0.05f;
                auto idx = map.GetMapChipIndexByPosition(foot);
                MapChipType t = map.GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
                if (t == MapChipType::kSpike) {
                    bossState_ = BossState::Rest;
                    stateTimer_ = restDuration_;
                    // 结束大招：恢复 CD
                    FinishUltimateCooldown();
                    ultimateLocked_ = false;

                    // 额外来一圈“终结爆环”，更炫
                    {
                        Vector3 c = position_;
                        c.y += height_ * 0.18f;
                        float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                        const int count = (hp_ <= enrageHp_) ? 14 : 12;
                        const float spd = (hp_ <= enrageHp_) ? 0.22f : 0.20f;
                        SpawnRadialBurst(c, count, spd, 1.10f, 0.28f, r * 6.283185307179586f);
                    }
                }
            }

            // 方块：X 方向被撞停（velocity_.x==0）视为一次反弹
            if (bossState_ == BossState::Ultimate && std::fabs(velocity_.x) < 0.0001f) {
                ultimateBounces_++;
                attackFacing_ *= -1;
                facing_ = attackFacing_;

                // 每次反弹喷一圈，打击感更强
                {
                    Vector3 c = position_;
                    c.y += height_ * 0.18f;
                    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                    const int count = (hp_ <= enrageHp_) ? 12 : 10;
                    const float spd = (hp_ <= enrageHp_) ? 0.21f : 0.19f;
                    SpawnRadialBurst(c, count, spd, 1.05f, 0.28f, r * 6.283185307179586f);
                }

                if (ultimateBounces_ >= ultimateMaxBounces_) {
                    bossState_ = BossState::Rest;
                    stateTimer_ = restDuration_;
                    FinishUltimateCooldown();
                    ultimateLocked_ = false;
                }
            }
        }
    }
    // 如果大招被外部逻辑打断（比如碰到玩家触发受击/硬直），确保 CD 不会卡死在 399
    if (ultimateLocked_ && bossState_ != BossState::Ultimate && ultimateCD_ > 100.0f) {
        FinishUltimateCooldown();
        ultimateLocked_ = false;
    }

    // ===== Dash 时缩小与玩家的判定（hurtbox） =====
    // 说明：这里缩的是 width_/height_（玩家交互判定），地图碰撞仍然用 mapColliderW_/H_。
    if (type_ == EnemyType::Boss) {
        if (bossState_ == BossState::Dash) {
            width_  = baseHurtW_ * dashHurtScaleX_;
            height_ = baseHurtH_ * dashHurtScaleY_;
        } else {
            width_  = baseHurtW_;
            height_ = baseHurtH_;
        }
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

void BossEnemy::Draw()
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

void BossEnemy::OnStomp()
{
    if (isDead_) { return; }

    // 防止同一帧/同一次重叠反复扣血
    if (stompInvuln_ > 0.0f) { return; }

    if (type_ == EnemyType::Boss) {
        // Boss：踩头 30 次死亡（默认）
        hp_ -= 1;

        // 受击反馈：闪烁更久一点 + 短硬直（更容易连续踩）
        StartHitReaction(0.55f);
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

void BossEnemy::ResolveMapCollision(const MapChipField& map, float dt)
{
    const float step = StepScale(dt);
    // 地图碰撞只使用“固定体积”，不要被 Dash 的 hurtbox 缩放影响
    const float kHalfW = mapColliderW_ * 0.5f;
    const float kHalfH = mapColliderH_ * 0.5f;

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


bool BossEnemy::IsPlayerOnGround(const MapChipField& map, const Player& player) const
{
    // 用地图格子判断“脚下是否有实体块”
    const Vector3 pPos = player.GetPosition();
    const float halfW = player.GetWidth()  * 0.5f;
    const float halfH = player.GetHeight() * 0.5f;

    // 往下探一点点，避免因为浮点误差刚好踩在边界上
    const float probeY = 0.06f;

    Vector3 probes[3] = {
        { pPos.x,                 pPos.y - halfH - probeY, 0.0f }, // 脚底中点
        { pPos.x - halfW * 0.80f, pPos.y - halfH - probeY, 0.0f }, // 左脚
        { pPos.x + halfW * 0.80f, pPos.y - halfH - probeY, 0.0f }, // 右脚
    };

    for (const auto& q : probes) {
        auto idx = map.GetMapChipIndexByPosition(q);
        MapChipType t = map.GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
        if (IsSolid(t)) {
            return true;
        }
    }
    return false;
}

bool BossEnemy::IsInEngageRange(const MapChipField& map, const Player& player) const
{
    if (!battleTriggered_) { return false; }

    const Vector3 pPos = player.GetPosition();
    const float dx = std::fabs(pPos.x - position_.x);
    const float dy = std::fabs(pPos.y - position_.y);

    if (dx > detectRange_) { return false; }
    if (dy > engageVerticalRange_) { return false; }

    if (requirePlayerOnGroundToEngage_ && !IsPlayerOnGround(map, player)) {
        return false;
    }
    return true;
}

bool BossEnemy::ShouldShowBossHp(const Player& player, const MapChipField& map) const
{
    // 方案 A（按你需求）：只有“接近 Boss”才显示血条
    //return IsInEngageRange(map, player);

    // 方案 B（更常见）：一旦触发 Boss 战就一直显示（不跟距离闪烁）
    return battleTriggered_;
}

void BossEnemy::UpdateBossFacing(const Player& player)
{
    // 攻击/前摇/射击时锁向
    if (bossState_ == BossState::Dash ||
        bossState_ == BossState::Windup ||
        bossState_ == BossState::Shoot ||
        bossState_ == BossState::Barrage ||
        bossState_ == BossState::Nova ||
        bossState_ == BossState::Jump ||
        bossState_ == BossState::Slam ||
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

void BossEnemy::UpdateBossProjectiles(float dt, const MapChipField& map)
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

void BossEnemy::SpawnBossProjectile(const Vector3& spawnPos, const Vector3& aimDir)
{
    Vector3 vel{ aimDir.x * projectileSpeed_, aimDir.y * projectileSpeed_, 0.0f };
    SpawnBossProjectileRaw(spawnPos, vel, projectileLife_, 0.35f);
}

void BossEnemy::SpawnBossProjectileRaw(const Vector3& spawnPos, const Vector3& velocity, float life, float radius)
{
    // 找一个空槽
    for (auto& p : projectiles_) {
        if (p.active) { continue; }

        p.active = true;
        p.life = life;
        p.radius = radius;
        p.pos = spawnPos;
        p.vel = velocity;

        if (p.obj) {
            p.obj->SetTranslate(p.pos);
            p.obj->Update();
        }
        return;
    }

    // 全满则丢弃（避免无限增长）
}

void BossEnemy::SpawnRadialBurst(const Vector3& center, int count, float speed, float life, float radius, float angleOffset)
{
    if (type_ != EnemyType::Boss) { return; }
    if (count <= 0) { return; }

    const float twoPi = 6.283185307179586f;
    const float step = twoPi / static_cast<float>(count);

    for (int i = 0; i < count; ++i) {
        float a = angleOffset + step * static_cast<float>(i);
        Vector3 d{ std::cos(a), std::sin(a), 0.0f };
        Vector3 vel{ d.x * speed, d.y * speed, 0.0f };
        SpawnBossProjectileRaw(center, vel, life, radius);
    }
}

void BossEnemy::DrawBossProjectiles()
{
    if (type_ != EnemyType::Boss) { return; }

    for (auto& p : projectiles_) {
        if (!p.active || !p.obj) { continue; }
        p.obj->Draw();
    }
}

void BossEnemy::FinishUltimateCooldown()
{
    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    if (hp_ <= enrageHp_) { ultimateCD_ = 2.8f + 1.4f * r; }
    else                 { ultimateCD_ = 4.0f + 2.0f * r; }
}

bool BossEnemy::CheckBossProjectileHit(const Player& player)
{
    if (type_ != EnemyType::Boss) { return false; }
    if (!battleTriggered_) { return false; }

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
