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

        // 少し下に伸ばして、確実に「ちょうど接地（bottom==tileTop）」も能判到トゲ
        const float probeY = pos.y - halfH - 0.08f;

        // 3 点采样: 中/左脚/右脚（中心だけを見ると足先にが端のトゲに触れても見逃すため）
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

    // Type1（E1）: 目の前のタイルが障害物（Block/Spike/MovingBlock 等）ならジャンプしたい
    enum class JumpKind { None, Small, High };

    inline JumpKind NeedJumpAheadKind(const Vector3& pos, int facing, float width, float height, const MapChipField& map)
    {
        const float halfW = width * 0.5f;
        const float halfH = height * 0.5f;

        // ✅探知距離: 遠すぎると早めに跳んでしまうので、長くしすぎない
        const float probeX = pos.x + static_cast<float>(facing) * (halfW + 0.12f);

        const float yFoot = pos.y - halfH + 0.12f; // 脚辺
        const float yMid = pos.y;                 // 身体中部

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
                        if (vel.y > 0.0f) {          // 頂头
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

    // 敵タイプに応じてモデルを切り替える（パスはプロジェクトのリソースに合わせて変より）
    switch (type_) {
    case EnemyType::Type0:
        obj_->SetModel("enemy0/enemy0.obj");
        break;
    case EnemyType::Type1:
        obj_->SetModel("enemy2/enemy2.obj");
        break;
    case EnemyType::Boss:
        // 防御的な書き方: 誤って Boss が渡された場合は、ここでは Type1 として処理
        obj_->SetModel("enemy1/enemy1.obj");
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

    // Type1（E1）エリア制限: スポーン地点（家）を記録して状態を初期化
    homePos_ = spawnPos;
    type1State_ = Type1State::Patrol;
}

void NormalEnemy::Update(float dt, const MapChipField& map, const Player& player)
{
    // 完全に死亡済み（アニメ終了）
    if (isDead_) { return; }

    // ===== 死亡アニメより新 =====
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

    // Type1 cooldown
    if (type_ == EnemyType::Type1) {
        jumpCooldown_ = (std::max)(0.0f, jumpCooldown_ - dt);
    }

    // 重力
    velocity_.y += gravity_ * dt;
    if (velocity_.y < -2.5f) { velocity_.y = -2.5f; }

    // =====================================================
    // 挙動: Type1 追プレイヤー；Type0 巡回
    // =====================================================
    if (type_ == EnemyType::Type1) {
        const float playerX = player.GetPosition().x;

        // 「スポーン地点 homePos_」を中心としたエリア制限: プレイヤーが警戒範囲に入ると追跡し、リーシュ範囲を超えたら離脱して帰還
        const float distPlayerHome = std::fabs(playerX - homePos_.x);
        const float distEnemyHome  = std::fabs(position_.x - homePos_.x);

        const bool canAggro = (distPlayerHome <= aggroRange_);
        // リーシュ判定: プレイヤーがリーシュ範囲を離れる、または敵が家から離れすぎたら追跡をやめる
        const bool inLeash  = (distPlayerHome <= leashRange_) && (distEnemyHome <= leashRange_ * 1.2f);

        const float homeLeft  = homePos_.x - leashRange_;
        const float homeRight = homePos_.x + leashRange_;
        const float chaseTargetX = std::clamp(playerX, homeLeft, homeRight);

        auto MoveToX = [&](float targetX, float speed) {
            const float dx = targetX - position_.x;
            constexpr float kDeadZone = 0.10f;

            if (dx > kDeadZone) { facing_ = 1; }
            else if (dx < -kDeadZone) { facing_ = -1; }

            const bool nearTarget = (std::fabs(dx) < stopRange_);
            if (nearTarget) {
                velocity_.x = 0.0f;
            }
            else {
                velocity_.x = static_cast<float>(facing_) * speed;

                // 障害物でジャンプ（2 段階）
                if (isOnGround_ && jumpCooldown_ <= 0.0f) {
                    JumpKind k = NeedJumpAheadKind(position_, facing_, width_, height_, map);
                    if (k != JumpKind::None) {
                        velocity_.y = (k == JumpKind::High) ? highJumpVelocity_ : smallJumpVelocity_;
                        isOnGround_ = false;
                        jumpCooldown_ = jumpCooldownTime_;
                    }
                }
            }
        };

        // ---------- ステートマシン ----------
        switch (type1State_) {
        case Type1State::Patrol:
            // プレイヤーが警戒範囲に入る -> 追跡
            if (canAggro) {
                type1State_ = Type1State::Chase;
                break;
            }

            // デフォルト棒立ち（patrolHalfWidth_==0）、想巡回そのまま patrolHalfWidth_ 設成 >0
            if (patrolHalfWidth_ <= 0.0f) {
                velocity_.x = 0.0f;
            }
            else {
                velocity_.x = static_cast<float>(facing_) * moveSpeed_;
                if (position_.x < homePos_.x - patrolHalfWidth_) { facing_ = 1; }
                if (position_.x > homePos_.x + patrolHalfWidth_) { facing_ = -1; }
            }
            break;

        case Type1State::Chase:
            // 離脱: プレイヤー／敵がリーシュ範囲を離れる
            if (!inLeash) {
                type1State_ = Type1State::Return;
                break;
            }
            MoveToX(chaseTargetX, chaseSpeed_);
            break;

        case Type1State::Return:
            // プレイヤーが再び警戒範囲に戻る -> 追跡を継続（必要なければ削除可）
            if (canAggro) {
                type1State_ = Type1State::Chase;
                break;
            }
            // 戻る家付近 -> 巡回に戻る/棒立ち
            if (std::fabs(position_.x - homePos_.x) <= returnStopDist_) {
                velocity_.x = 0.0f;
                type1State_ = Type1State::Patrol;
                break;
            }
            MoveToX(homePos_.x, moveSpeed_);
            break;
        }
    }
    else {
        // Type0: 左右巡回
        velocity_.x = static_cast<float>(facing_) * moveSpeed_;

        // 崖端で反転: 前方足元にブロックがなければ方向転換
        if (isOnGround_) {
            float checkX = position_.x + static_cast<float>(facing_) * (width_ * 0.5f + 0.20f);
            float checkY = position_.y - height_ * 0.5f - 0.15f;
            auto idx = map.GetMapChipIndexByPosition({ checkX, checkY, 0.0f });
            if (!IsSolid(map.GetMapChipTypeByIndex(idx.xIndex, idx.yIndex))) {
                facing_ *= -1;
                velocity_.x = static_cast<float>(facing_) * moveSpeed_;
            }
        }
    }

    // =====================================================
    // Map 衝突
    // =====================================================
    const float prevVX = velocity_.x;
    ResolveMapCollision(position_, velocity_, isOnGround_, width_, height_, map, dt);

    // =====================================================
    // 衝突後処理: Type1 壁詰まり時に追加ジャンプ；Type0 壁に当たったら反転
    // =====================================================
    if (type_ == EnemyType::Type1) {
        const float playerX = player.GetPosition().x;
        const float homeLeft  = homePos_.x - leashRange_;
        const float homeRight = homePos_.x + leashRange_;

        float targetX = 0.0f;
        if (type1State_ == Type1State::Return) {
            targetX = homePos_.x;
        }
        else if (type1State_ == Type1State::Patrol) {
            // 棒立ち時は現在位置をそのまま目標にし、巡回時は境界方向を目標にする
            targetX = (patrolHalfWidth_ <= 0.0f)
                ? position_.x
                : (homePos_.x + static_cast<float>(facing_) * patrolHalfWidth_);
        }
        else {
            targetX = std::clamp(playerX, homeLeft, homeRight);
        }

        const bool nearTarget = (std::fabs(targetX - position_.x) < stopRange_);

        if (!nearTarget &&
            isOnGround_ && jumpCooldown_ <= 0.0f &&
            std::fabs(prevVX) > 0.0001f && std::fabs(velocity_.x) < 0.0001f)
        {
            JumpKind k = NeedJumpAheadKind(position_, facing_, width_, height_, map);
            if (k != JumpKind::None) {
                velocity_.y = highJumpVelocity_; // 脱困用高跳
                isOnGround_ = false;
                jumpCooldown_ = jumpCooldownTime_;
            }
        }
    }
    else {
        // ✅Type0: 壁に当たったら反転（以前の実装で抜けていた）
        if (std::fabs(prevVX) > 0.0001f && std::fabs(velocity_.x) < 0.0001f) {
            facing_ *= -1;
        }
    }

    // =====================================================
    // トゲ: 踏むと死亡
    // Type1 だけをトゲで死亡させたい場合は、外側に if(type_==EnemyType::Type1) を追加する
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
    // ✅描画更新: Type0 / Type1 の両方で必ず実行する（以前は Type1 にしか適用されていなかった）
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

    // 死亡アニメ期間不点滅隐藏（否なら看起来像「消失」）
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

    // 必要なら、被弾フィードバックとして少し点滅を残してもよい（コメントアウト可）
    StartHitReaction(0.10f);
}
