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
        // 60fps を基準とし、フレーム落ちで 1 フレームの移動量が大きくなりすぎて壁抜けしないよう上限を設ける
        float s = dt * 60.0f;
        if (s < 0.0f) s = 0.0f;
        if (s > 3.0f) s = 3.0f;
        return s;
    }

    inline bool HitSpikeUnderFoot(const Vector3& pos, float width, float height, const MapChipField& map)
    {
        const float halfW = width * 0.5f;
        const float halfH = height * 0.5f;

        // 少し下に伸ばして、確実に「ちょうど接地（bottom==tileTop）」も検出できるようにする
        const float probeY = pos.y - halfH - 0.08f;

        // 3 点サンプリング: 中/左脚/右脚（中心だけを見ると足先が端のトゲに触れても見逃すため）
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

    // 前方の足元が抜けているかどうか
    inline bool IsMissingGroundAhead(const Vector3& pos, int facing, float width, float height, const MapChipField& map)
    {
        const float checkX = pos.x + static_cast<float>(facing) * (width * 0.5f + 0.20f);
        const float checkY = pos.y - height * 0.5f - 0.15f;
        auto idx = map.GetMapChipIndexByPosition({ checkX, checkY, 0.0f });
        return !IsSolid(map.GetMapChipTypeByIndex(idx.xIndex, idx.yIndex));
    }

    // Type1 / Type2: 目の前のタイルが障害物（Block/Spike/MovingBlock 等）ならジャンプしたい
    enum class JumpKind { None, Small, High };

    inline JumpKind NeedJumpAheadKind(const Vector3& pos, int facing, float width, float height, const MapChipField& map)
    {
        const float halfW = width * 0.5f;
        const float halfH = height * 0.5f;

        // 遠すぎると早めに跳んでしまうので、長くしすぎない
        const float probeX = pos.x + static_cast<float>(facing) * (halfW + 0.12f);

        const float yFoot = pos.y - halfH + 0.12f; // 脚辺
        const float yMid = pos.y;                  // 身体中部

        auto idxF = map.GetMapChipIndexByPosition({ probeX, yFoot, 0.0f });
        MapChipType tF = map.GetMapChipTypeByIndex(idxF.xIndex, idxF.yIndex);

        auto idxM = map.GetMapChipIndexByPosition({ probeX, yMid, 0.0f });
        MapChipType tM = map.GetMapChipTypeByIndex(idxM.xIndex, idxM.yIndex);

        // 体の高さで当たる（壁 / ブロック / 移動床の側面）=> 高ジャンプ
        if (IsSolid(tM)) { return JumpKind::High; }

        // 足元付近だけ当たる（通常はトゲ / 小さな段差）=> 低ジャンプ
        if (IsSolid(tF)) { return JumpKind::Small; }

        return JumpKind::None;
    }

    // BossEnemy::ResolveMapCollision の「簡易再利用版」
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
                        if (vel.y > 0.0f) {          // 頭突き
                            fixY = r.bottom - halfH;
                            vel.y = 0.0f;
                        }
                        else if (vel.y < 0.0f) {     // 着地
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

    // 通常敵 HP
    maxHp_ = 1;
    enrageHp_ = 0;
    hp_ = 1;

    // 敵タイプに応じてモデルを切り替える
    switch (type_) {
    case EnemyType::Type0:
        obj_->SetModel("enemy0/enemy0.obj");
        break;
    case EnemyType::Type1:
        obj_->SetModel("enemy2/enemy2.obj");
        break;
    case EnemyType::Type2:
        obj_->SetModel("enemy3/enemy3.obj");
        break;
    case EnemyType::Boss:
        // 防御的な書き方: 誤って Boss が渡された場合は、ここでは Type1 として処理
        obj_->SetModel("enemy2/enemy2.obj");
        type_ = EnemyType::Type1;
        break;
    }

    obj_->SetTranslate(position_);
    obj_->SetRotate({ 0.0f, 0.0f, 0.0f });
    obj_->Update();

    // 移動／死亡アニメ状態
    velocity_ = { 0.0f, 0.0f, 0.0f };
    isOnGround_ = false;
    facing_ = (std::rand() % 2 == 0) ? 1 : -1;
    isDying_ = false;
    deathTimer_ = 0.0f;
    deathSpin_ = 0.0f;

    // 記録「存活時衝突尺寸」（死亡アニメ中のマップ衝突用）
    aliveWidth_ = width_;
    aliveHeight_ = height_;

    // 各種状態
    homePos_ = spawnPos;
    type1State_ = Type1State::Patrol;
    type2State_ = Type2State::Patrol;
    jumpCooldown_ = 0.0f;
    type0JumpCooldown_ = 0.0f;
    pounceCooldown_ = 0.0f;
    recoverTimer_ = 0.0f;
    chaseMemoryTimer_ = 0.0f;

    // タイプごとの初期チューニング
    switch (type_) {
    case EnemyType::Type0:
        patrolHalfWidth_ = 0.0f;
        break;
    case EnemyType::Type1:
        patrolHalfWidth_ = 2.5f;   // 最初から少し動かして待機を自然にする
        aggroRange_ = 12.0f;
        leashRange_ = 20.0f;
        stopRange_ = 0.90f;
        break;
    case EnemyType::Type2:
        patrolHalfWidth_ = 3.5f;
        aggroRange_ = 14.0f;
        leashRange_ = 22.0f;
        stopRange_ = 1.10f;
        break;
    case EnemyType::Boss:
        break;
    }
}

void NormalEnemy::Update(float dt, const MapChipField& map, const Player& player)
{
    // 完全に死亡済み（アニメ終了）
    if (isDead_) { return; }

    // ===== 死亡アニメ更新 =====
    if (isDying_) {
        deathTimer_ = (std::max)(0.0f, deathTimer_ - dt);

        velocity_.y += gravity_ * dt;
        if (velocity_.y < -2.5f) { velocity_.y = -2.5f; }

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

    // ===== 生存状態 =====
    if (!UpdateCommon(dt)) { return; }

    // cooldown / memory
    jumpCooldown_ = (std::max)(0.0f, jumpCooldown_ - dt);
    type0JumpCooldown_ = (std::max)(0.0f, type0JumpCooldown_ - dt);
    pounceCooldown_ = (std::max)(0.0f, pounceCooldown_ - dt);
    recoverTimer_ = (std::max)(0.0f, recoverTimer_ - dt);
    chaseMemoryTimer_ = (std::max)(0.0f, chaseMemoryTimer_ - dt);

    // 重力
    velocity_.y += gravity_ * dt;
    if (velocity_.y < -2.5f) { velocity_.y = -2.5f; }

    const Vector3 playerPos = player.GetPosition();
    const float playerX = playerPos.x;
    const float playerY = playerPos.y;
    const float distPlayerHome = std::fabs(playerX - homePos_.x);
    const float distEnemyHome  = std::fabs(position_.x - homePos_.x);

    auto MoveToX = [&](float targetX, float speed, float stopRange) {
        const float dx = targetX - position_.x;
        constexpr float kDeadZone = 0.10f;

        if (dx > kDeadZone) { facing_ = 1; }
        else if (dx < -kDeadZone) { facing_ = -1; }

        const bool nearTarget = (std::fabs(dx) < stopRange);
        if (nearTarget) {
            velocity_.x = 0.0f;
            return false;
        }

        velocity_.x = static_cast<float>(facing_) * speed;
        return true;
    };

    auto TryJumpAhead = [&](float lowVel, float highVel, float& cooldownTimer, float cooldownTime) {
        if (!isOnGround_ || cooldownTimer > 0.0f) { return; }
        JumpKind k = NeedJumpAheadKind(position_, facing_, width_, height_, map);
        if (k == JumpKind::None) { return; }
        velocity_.y = (k == JumpKind::High) ? highVel : lowVel;
        isOnGround_ = false;
        cooldownTimer = cooldownTime;
    };

    auto PatrolAroundHome = [&](float halfWidth, float speed) {
        if (halfWidth <= 0.0f) {
            velocity_.x = 0.0f;
            return;
        }

        if (position_.x < homePos_.x - halfWidth) { facing_ = 1; }
        if (position_.x > homePos_.x + halfWidth) { facing_ = -1; }

        if (isOnGround_ && IsMissingGroundAhead(position_, facing_, width_, height_, map)) {
            facing_ *= -1;
        }
        velocity_.x = static_cast<float>(facing_) * speed;
    };

    // =====================================================
    // 挙動: Type0 警戒巡回 / Type1 追跡強化 / Type2 飛びかかり
    // =====================================================
    if (type_ == EnemyType::Type1) {
        const bool playerInAggroRange = (distPlayerHome <= aggroRange_);
        if (playerInAggroRange) {
            chaseMemoryTimer_ = chaseMemoryTime_;
        }

        const bool canAggro = playerInAggroRange || (chaseMemoryTimer_ > 0.0f);
        const bool inLeash  = (distPlayerHome <= leashRange_) && (distEnemyHome <= leashRange_ * 1.2f);

        const float homeLeft  = homePos_.x - leashRange_;
        const float homeRight = homePos_.x + leashRange_;
        const float chaseTargetX = std::clamp(playerX, homeLeft, homeRight);

        switch (type1State_) {
        case Type1State::Patrol:
            if (canAggro) {
                type1State_ = Type1State::Chase;
                break;
            }
            PatrolAroundHome(patrolHalfWidth_, moveSpeed_);
            break;

        case Type1State::Chase:
            if (!inLeash) {
                type1State_ = Type1State::Return;
                break;
            }
            {
                const float dx = std::fabs(chaseTargetX - position_.x);
                const float chaseSpeedNow = (dx > 6.0f) ? chaseBoostSpeed_ : chaseSpeed_;
                const bool moving = MoveToX(chaseTargetX, chaseSpeedNow, stopRange_);
                if (moving) {
                    TryJumpAhead(smallJumpVelocity_, highJumpVelocity_, jumpCooldown_, jumpCooldownTime_);
                }
            }
            break;

        case Type1State::Return:
            if (canAggro) {
                type1State_ = Type1State::Chase;
                break;
            }
            if (std::fabs(position_.x - homePos_.x) <= returnStopDist_) {
                velocity_.x = 0.0f;
                type1State_ = Type1State::Patrol;
                break;
            }
            MoveToX(homePos_.x, moveSpeed_, returnStopDist_);
            TryJumpAhead(smallJumpVelocity_, highJumpVelocity_, jumpCooldown_, jumpCooldownTime_);
            break;
        }
    }
    else if (type_ == EnemyType::Type2) {
        const float dx = playerX - position_.x;
        const float adx = std::fabs(dx);
        const float ady = std::fabs(playerY - position_.y);

        const bool canAggro = (distPlayerHome <= aggroRange_);
        const bool inLeash  = (distPlayerHome <= leashRange_) && (distEnemyHome <= leashRange_ * 1.15f);

        auto MoveInDistanceBand = [&](float targetX, float minDist, float maxDist, float speed) {
            const float localDx = targetX - position_.x;
            const float localAbsDx = std::fabs(localDx);

            if (localAbsDx > maxDist) {
                MoveToX(targetX, speed, maxDist);
                return true;
            }
            if (localAbsDx < minDist) {
                const float retreatDir = (localDx >= 0.0f) ? -1.0f : 1.0f;
                facing_ = (retreatDir >= 0.0f) ? 1 : -1;
                velocity_.x = retreatDir * speed;
                return true;
            }

            velocity_.x = 0.0f;
            if (localDx > 0.1f) { facing_ = 1; }
            else if (localDx < -0.1f) { facing_ = -1; }
            return false;
        };

        switch (type2State_) {
        case Type2State::Patrol:
            if (canAggro) {
                type2State_ = Type2State::Stalk;
                break;
            }
            PatrolAroundHome(patrolHalfWidth_, moveSpeed_);
            break;

        case Type2State::Stalk:
            if (!inLeash) {
                type2State_ = Type2State::Return;
                break;
            }

            if (dx > 0.1f) { facing_ = 1; }
            else if (dx < -0.1f) { facing_ = -1; }

            if (isOnGround_ && pounceCooldown_ <= 0.0f &&
                adx >= pounceMinRange_ && adx <= pounceMaxRange_ && ady <= pounceVerticalRange_)
            {
                velocity_.x = static_cast<float>(facing_) * pounceSpeed_;
                velocity_.y = pounceJumpVelocity_;
                isOnGround_ = false;
                pounceCooldown_ = pounceCooldownTime_;
                type2State_ = Type2State::Pounce;
                break;
            }

            if (MoveInDistanceBand(playerX, stalkPreferredMin_, stalkPreferredMax_, stalkSpeed_)) {
                TryJumpAhead(smallJumpVelocity_, highJumpVelocity_, jumpCooldown_, jumpCooldownTime_);
            }
            break;

        case Type2State::Pounce:
            velocity_.x = static_cast<float>(facing_) * (isOnGround_ ? pounceSpeed_ : pounceAirControl_);
            break;

        case Type2State::Recover:
            velocity_.x = 0.0f;
            if (recoverTimer_ <= 0.0f) {
                type2State_ = inLeash ? Type2State::Stalk : Type2State::Return;
            }
            break;

        case Type2State::Return:
            if (canAggro) {
                type2State_ = Type2State::Stalk;
                break;
            }
            if (std::fabs(position_.x - homePos_.x) <= returnStopDist_) {
                velocity_.x = 0.0f;
                type2State_ = Type2State::Patrol;
                break;
            }
            MoveToX(homePos_.x, moveSpeed_, returnStopDist_);
            TryJumpAhead(smallJumpVelocity_, highJumpVelocity_, jumpCooldown_, jumpCooldownTime_);
            break;
        }
    }
    else {
        // Type0: シンプルな巡回を維持しつつ、近くのプレイヤーにだけ反応を少し強くする
        const float dx = playerX - position_.x;
        const bool isAlert = (std::fabs(dx) <= type0AlertRange_) && (std::fabs(playerY - position_.y) <= 2.5f);

        if (isAlert && std::fabs(dx) > 0.70f) {
            facing_ = (dx >= 0.0f) ? 1 : -1;
        }

        const float speed = isAlert ? type0AlertSpeed_ : moveSpeed_;
        velocity_.x = static_cast<float>(facing_) * speed;

        // 崖端で反転
        if (isOnGround_ && IsMissingGroundAhead(position_, facing_, width_, height_, map)) {
            facing_ *= -1;
            velocity_.x = static_cast<float>(facing_) * speed;
        }

        // 近距離警戒中だけ軽い段差ジャンプを許可し、従来より詰まりにくくする
        if (isAlert) {
            TryJumpAhead(type0JumpVelocity_, type0JumpVelocity_ + 0.08f, type0JumpCooldown_, type0JumpCooldownTime_);
        }
    }

    // =====================================================
    // Map 衝突
    // =====================================================
    const float prevVX = velocity_.x;
    const bool wasOnGround = isOnGround_;
    ResolveMapCollision(position_, velocity_, isOnGround_, width_, height_, map, dt);

    // =====================================================
    // 衝突後処理
    // =====================================================
    if (type_ == EnemyType::Type1) {
        if (isOnGround_ && jumpCooldown_ <= 0.0f &&
            std::fabs(prevVX) > 0.0001f && std::fabs(velocity_.x) < 0.0001f)
        {
            JumpKind k = NeedJumpAheadKind(position_, facing_, width_, height_, map);
            if (k != JumpKind::None) {
                velocity_.y = highJumpVelocity_; // 脱困用高跳
                isOnGround_ = false;
                jumpCooldown_ = jumpCooldownTime_;
            }
            else {
                facing_ *= -1;
            }
        }
    }
    else if (type_ == EnemyType::Type2) {
        if (type2State_ == Type2State::Pounce) {
            // 着地したら短い硬直を入れて「飛びかかった感」を出す
            if (!wasOnGround && isOnGround_) {
                type2State_ = Type2State::Recover;
                recoverTimer_ = recoverDuration_;
                velocity_.x = 0.0f;
            }
            else if (std::fabs(prevVX) > 0.0001f && std::fabs(velocity_.x) < 0.0001f && !isOnGround_) {
                // 空中で壁に刺さったら、そのまま惰性で引っかかるのを避ける
                facing_ *= -1;
            }
        }
        else if (std::fabs(prevVX) > 0.0001f && std::fabs(velocity_.x) < 0.0001f && isOnGround_) {
            TryJumpAhead(highJumpVelocity_, highJumpVelocity_, jumpCooldown_, jumpCooldownTime_);
        }
    }
    else {
        // Type0: 壁に当たったら反転
        if (std::fabs(prevVX) > 0.0001f && std::fabs(velocity_.x) < 0.0001f) {
            facing_ *= -1;
        }
    }

    // =====================================================
    // トゲ: 踏むと死亡
    // =====================================================
    if (!isDying_ && HitSpikeUnderFoot(position_, width_, height_, map)) {
        OnStomp();
        return;
    }

    // =====================================================
    // プレイヤーの踏みつけ
    // =====================================================
    {
        const auto contact = CheckPlayerContact(player);
        if (contact.overlap && contact.stomp) {
            OnStomp();
            return;
        }
    }

    // =====================================================
    // 描画更新
    // =====================================================
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

    // 死亡アニメ期間は点滅で隠さない（見た目が消えたように見えるため）
    if (!isDying_ && isHitReacting_ && !damageBlinkVisible_) {
        return;
    }
    obj_->Draw();
}

void NormalEnemy::OnStomp()
{
    if (isDead_ || isDying_) { return; }

    // 1 回の踏みつけで即座に死亡
    hp_ = 0;

    // 死亡アニメに入る: まず少し跳ね上げる
    isDying_ = true;
    deathTimer_ = deathDuration_;
    deathSpin_ = 0.0f;

    velocity_.x = 0.0f;
    velocity_.y = 0.75f;
    isOnGround_ = false;

    // ゲームロジック上はすぐに衝突しないようにする（ただし死亡アニメは描画する）
    width_ = 0.0f;
    height_ = 0.0f;

    // 少しだけ被弾フィードバックを残す
    StartHitReaction(0.10f);
}
