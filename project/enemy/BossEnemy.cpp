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
        // 60fps を基準とし、フレーム落ちで 1 フレームの移動量が大きくなりすぎて壁抜けしないよう上限を設ける
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
    visualOffsetY_ = 0.0f;


preAttackJitter_ = { 0.0f, 0.0f, 0.0f };
preAttackJitterTime_ = 0.0f;

    // Boss: デフォルトでは先に「スリープ」状態にし、プレイヤーが指定エリアに入ったら起動する（Update 内のトリガー条件を参照）
    battleTriggered_ = (type != EnemyType::Boss);

    // ===== HP =====
    isDead_ = false;
    stompInvuln_ = 0.0f;
    if (type == EnemyType::Boss) {
        maxHp_ = 30;
        // 低HP閾値: 20%（30 -> 6）
        enrageHp_ = (std::max)(1, maxHp_ / 5);
        hp_ = maxHp_;
    } else {
        maxHp_ = 1;
        enrageHp_ = 0;
        hp_ = 1;
    }
    // 状態リセット
    isHitReacting_ = false;
    hitReactTimer_ = 0.0f;
    damageBlinkTimer_ = 0.0f;
    damageBlinkVisible_ = true;
    // デフォルトの点滅頻度（Boss で下面覆写成より快少し）
    damageBlinkInterval_ = 0.08f;

    velocity_ = { 0.0f, 0.0f, 0.0f };
    isOnGround_ = false;
    facing_ = 1;
    attackFacing_ = 1;

    bossState_ = BossState::Idle;
    queuedAttack_ = BossAttack::None;
    stateTimer_ = 0.0f;
    decisionTimer_ = 0.0f;


    // Dash パラメータをリセット
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
    ultimateWindupTotal_ = 0.0f;
    ultimateWindupBackstepMoved_ = 0.0f;


    shotsLeft_ = 0;
    shotsTotal_ = 0;
    shotInterval_ = 0.16f;
    shotTimer_ = 0.0f;
    fanShot_ = false;

    obj_ = std::make_unique<Object3d>();
    obj_->Initialize(common);
    obj_->SetCamera(camera);

    // デフォルトのサイズ
    width_  = 1.5f;
    height_ = 1.5f;

    // 敵タイプに応じてモデルを切り替える（パスはプロジェクトのリソースに合わせて変より）
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
    case EnemyType::Boss: {
        obj_->SetModel("enemy1/enemy1.obj"); // 無ければ既存のモデルパスに差し替える

        // Boss 被弾点滅: 頻度より快少し、より有「打撃感」
        damageBlinkInterval_ = 0.055f;

        // ===== 拡大 Boss（モデル）+ 增加当たり判定サイズ =====
        // 説明:Object3d に SetScale() が無ければ、このプロジェクトで対応するスケール API に置き換える。
        const float kBossScale = 1.35f;

        // 見た目だけ下げる（モデル原点が足元に無い＆拡大で浮いて見える対策）
        visualOffsetY_ = -0.85f;

        // 当たり判定サイズ（AABB）跟着拡大
        width_  = 2.6f * kBossScale;
        height_ = 3.0f * kBossScale;

        // モデルスケール
        obj_->SetScale({ kBossScale, kBossScale, kBossScale });

        // Boss の被弾点滅をやや速くして「電流感」を出す
        damageBlinkInterval_ = 0.055f;

        // Boss 被弾点滅: より明顕少し
        damageBlinkInterval_ = 0.06f;

        bossState_ = BossState::Idle;
        queuedAttack_ = BossAttack::None;

        // Boss 弾幕オブジェクトプールを事前生成
        projectiles_.clear();
        projectiles_.resize(kMaxBossProjectiles_);
        for (auto& p : projectiles_) {
            p.obj = std::make_unique<Object3d>();
            p.obj->Initialize(common);
            p.obj->SetCamera(camera);
            // ひとまず既存モデルを使う（必要なら fireball.obj などに差し替えてもよい）
            p.obj->SetModel("enemyBullet/enemyBullet.obj");
            p.pos = spawnPos;
            p.obj->SetTranslate(p.pos);
            p.obj->Update();
            p.active = false;
            p.life = 0.0f;
            p.radius = 0.35f;
            p.obj->SetColor({ 1.0f, 0.4f, 0.0f, 1.0f });
        }
        break;
    }
    default:
        break;
    }

    // --- 判定尺寸初期化 ---
    // 現在の width_/height_ を「基礎プレイヤー判定」と「マップ当たり判定サイズ」にする。
    // Dash 時は width_/height_ を一時的に縮小する（プレイヤー判定）が、マップ衝突には常に mapColliderW_/H_ を使う。
    baseHurtW_ = width_;
    baseHurtH_ = height_;
    mapColliderW_ = width_;
    mapColliderH_ = height_;

    obj_->SetTranslate(GetRenderPosition());
    // 初期向き: 右=0、左=PI（Player のロジックと一致）
    obj_->SetRotate({ 0.0f, 0.0f, 0.0f });
    obj_->Update();
}

void BossEnemy::Update(float dt, const MapChipField& map, const Player& player)
{
    if (isDead_) { return; }

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

    // 踏みつけ無敵時間（Boss 用）
    stompInvuln_ = (std::max)(0.0f, stompInvuln_ - dt);

    // ===== Boss AI =====
    if (type_ == EnemyType::Boss) {
        // 未トリガー時: AI や弾幕は更新せず、描画のみ行う（Boss は「彫像／待機」として扱える）
        // トリガーロジックは GameScene に任せる: 先にカメラ演出を流し、終了後に TriggerBattleNow() を呼ぶ。
        if (!battleTriggered_) {
preAttackJitter_ = { 0.0f, 0.0f, 0.0f };
preAttackJitterTime_ = 0.0f;

            // 未トリガーの Boss 戦では AI / 攻撃は行わないが、重力とマップ衝突は引き続き処理する。
            // そうしないと Boss の中心点がスポーン位置の高さに留まり、カメラが寄ったときに地面へ埋まっている／浮いているように見えやすい。
            velocity_.x = 0.0f;

            // Boss を自然に地面へ落とす（静止状態で地形に食い込むのを避けるため、先に下向き速度／重力を与える必要がある）
            velocity_.y += gravityBase_ * dt;
            if (velocity_.y < -2.5f) { velocity_.y = -2.5f; }
            ResolveMapCollision(map, dt);
            if (isOnGround_) { velocity_.y = 0.0f; }

            if (obj_) {
                obj_->SetTranslate(GetRenderPosition());
                obj_->Update();
            }
            return;
        }


        // 先により新弾幕（Boss 本体とプレイヤーの重なりには依存しない）
        UpdateBossProjectiles(dt, map);

        // ---- 向き更新（Dash/Melee/Windup/Shoot 中は固定し、急な反転を避ける）----
        UpdateBossFacing(player);

        // ---- 重力 ----
        velocity_.y += gravityBase_ * dt;
        if (velocity_.y < -2.5f) { velocity_.y = -2.5f; }

        // ---- 感知 ----
        const Vector3 pPos = player.GetPosition();
        const Vector3 pVel = player.GetVelocity();

        float dx = pPos.x - position_.x;
        float dist = std::fabs(dx);
        const bool playerMovingAway = (pVel.x * facing_ > 0.05f);      // プレイヤーが Boss から離れる方向に走っている
        const bool distIncreasing = (dist > prevDistToPlayer_ + 0.08f); // 距離が広がっている（引き撃ち）
        prevDistToPlayer_ = dist;
        // ---- 计時器 ----
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

// 毎フレームデフォルトで「描画揺れ」をクリアし、特定の予備動作ウィンドウ内でのみ UpdatePreAttackJitter() が書き込む
preAttackJitter_ = { 0.0f, 0.0f, 0.0f };
const bool wantsJitter = (bossState_ == BossState::Windup) &&
    (queuedAttack_ == BossAttack::Barrage || queuedAttack_ == BossAttack::Nova || queuedAttack_ == BossAttack::Slam);
if (!wantsJitter) {
    preAttackJitterTime_ = 0.0f;
}

        switch (bossState_) {

        case BossState::Stunned:
            // 踏みつけ後の硬直: プレイヤーが2段／3段で踏める猶予
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
            // 予測目標地点
            float targetX = pPos.x + pVel.x * leadTime_;

            // 接近時は後退しない（そうしないとプレイヤーが接近ダッシュを誘発しづらい）
            float dx2 = targetX - position_.x;
            float absDx2 = std::fabs(dx2);
            int dirToTarget = (dx2 >= 0.0f) ? 1 : -1;

            const float deadZone = 0.35f;
            if (absDx2 > idealRange_ + deadZone) {
                velocity_.x = dirToTarget * moveSpeed_;
            }
            else if (absDx2 < idealRange_ - deadZone) {
                velocity_.x = 0.0f;
            }
            else {
                velocity_.x = 0.0f;
            }

            // 簡易的な「障害物ジャンプ」（前方 1 マスが壁ならそのまま跳ぶ）
            if (isOnGround_) {
                int moveDir = facing_;
                if (velocity_.x > 0.01f) moveDir = 1;
                if (velocity_.x < -0.01f) moveDir = -1;

                // 地形探测用「身体サイズ」（Dash の hurtbox 縮小の影響を受けないようにする）
                float checkX = position_.x + moveDir * (mapColliderW_ * 0.5f + 0.15f);
                float checkY = position_.y - mapColliderH_ * 0.5f + 0.10f;

                auto idx = map.GetMapChipIndexByPosition({ checkX, checkY, 0.0f });
                if (IsSolid(map.GetMapChipTypeByIndex(idx.xIndex, idx.yIndex))) {
                    velocity_.y = jumpVel_;
                }
            }

            // 攻撃選択（queuedAttack_ を使って Windup 中の状態上書きを防ぐ）
            if (decisionTimer_ <= 0.0f && globalAttackCD_ <= 0.0f) {
                const bool canDashAny = (dashCD_ <= 0.0f);

                // 本当の「密着」距離: この距離内では遠距離攻撃を禁止し、小ダッシュへ切り替える
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
                    stateTimer_ = 0.40f; // 必殺技予備動作より明顕

                    ultimateWindupTotal_ = stateTimer_;
                    ultimateWindupBackstepMoved_ = 0.0f;

                    // 先に CD を引き上げ、必殺技終了後にランダム CD を再設定する
                    ultimateCD_ = 399.0f;
                    ultimateLocked_ = true;
                    globalAttackCD_ = 1.10f;
                    decisionTimer_ = 0.35f;
                }
                // ② 叩きつけ: 「頭上ジャンプ／密着旋回」対策。着地時に衝撃波弾幕を生成
                else if (canSlam && (playerAbove || dist <= (meleeRange_ + 1.2f))){
                    queuedAttack_ = BossAttack::Slam;
                    bossState_ = BossState::Windup;
                    attackFacing_ = facing_;
                    stateTimer_ = slamWindup_;

                    slamCD_ = (hp_ <= enrageHp_) ? 3.0f : 4.2f;
                    globalAttackCD_ = 1.15f;
                    decisionTimer_ = 0.30f;
                }
                // ②.5 円形爆発: よりド派手なリング弾幕（低HP時に優先し、プレイヤーの後退し続けにも対抗する）
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
                // ③ 旋转弾幕: 中遠距離制圧（より派手）、プレイヤーずっと後退時より容易トリガー
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
                // ④ 遠距離: 優先に远程制圧
                else if (canRanged && dist >= farRangedPrefer_ && dist > dashMaxRange_) {
                    queuedAttack_ = BossAttack::Ranged;
                    bossState_ = BossState::Windup;
                    attackFacing_ = facing_;
                    stateTimer_ = 0.28f;

                    rangedCD_ = (hp_ <= enrageHp_) ? 1.25f : 1.75f;
                    globalAttackCD_ = 0.85f;
                    decisionTimer_ = 0.25f;
                }
                // ③ プレイヤー接近: 朝プレイヤーダッシュ（ただし連続では突進せず、dashCD_ で制御する）
                else if (canDashAny && dist <= closeDashRange_) {
                    queuedAttack_ = BossAttack::Dash;
                    bossState_ = BossState::Windup;
                    attackFacing_ = facing_;
                    stateTimer_ = closeDashWindup_;
                    queuedDashDuration_ = closeDashDuration_;
                    queuedDashSpeed_ = closeDashSpeed_;
                    isShortDash_ = true;
                    microDashCD_ = microDashCooldown_;


                    dashWindupTotal_ = stateTimer_;
                    dashBackstepMoved_ = 0.0f;
                    dashCD_ = 2.70f;
                    globalAttackCD_ = 1.05f;
                    decisionTimer_ = 0.25f;
                }
                // ③.5 dashCD_ が進行中でも、プレイヤー密着時は遠距離を使わず小ダッシュ 1 回に切り替える
                else if (!canDashAny && dist <= closeDashRange_ && microDashCD_ <= 0.0f) {
                    queuedAttack_ = BossAttack::Dash;
                    bossState_ = BossState::Windup;
                    attackFacing_ = facing_;
                    stateTimer_ = closeDashWindup_;
                    queuedDashDuration_ = closeDashDuration_;
                    queuedDashSpeed_ = closeDashSpeed_;
                    isShortDash_ = true;

                    microDashCD_ = microDashCooldown_;

                    dashWindupTotal_ = stateTimer_;
                    dashBackstepMoved_ = 0.0f;
                    globalAttackCD_ = 0.95f;
                    decisionTimer_ = 0.20f;
                }

                // ④ 中距離: 择机ダッシュ（プレイヤーが引き撃ち／距離を取っている時に発動しやすい）
                else if (canDashAny && dist >= dashMinRange_ && dist <= dashMaxRange_
                    && (playerMovingAway || distIncreasing || dist <= 7.0f)) {
                    queuedAttack_ = BossAttack::Dash;
                    bossState_ = BossState::Windup;
                    attackFacing_ = facing_;
                    stateTimer_ = 0.30f;
                    queuedDashDuration_ = 0.30f;
                    queuedDashSpeed_ = dashSpeed_;
                    isShortDash_ = false;

                    dashWindupTotal_ = stateTimer_;
                    dashBackstepMoved_ = 0.0f;

                    dashCD_ = 2.10f;
                    globalAttackCD_ = 1.00f;
                    decisionTimer_ = 0.25f;
                }
                // ⑤ 保険: まだ远程そのまま远程
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
            // 予備動作: 少し止まってプレイヤーに予兆を読ませる
            velocity_.x = 0.0f;
            facing_ = attackFacing_;



            UpdatePreAttackJitter(dt);

            // Dash: Windup 中少し後退してからダッシュを放つ（予備動作を読みやすくする）
            if (queuedAttack_ == BossAttack::Dash && stateTimer_ > 0.0f) {
                const float targetDist = isShortDash_ ? dashBackstepDistShort_ : dashBackstepDist_;
                const float step = StepScale(dt);

                float total = dashWindupTotal_;
                if (total < 0.001f) { total = (std::max)(stateTimer_, 0.001f); }

                // Windup 総時間に合わせて均等に後退し、Windup 時間が違っても後退距離がだいたい同じになるようにする
                float baseVel = targetDist / (total * 60.0f); // 60fps を基準にした「1フレーム移動量」
                baseVel = (std::min)(baseVel, dashBackstepMaxSpeed_);
                baseVel = (std::max)(baseVel, dashBackstepMinSpeed_);

                float remaining = (std::max)(0.0f, targetDist - dashBackstepMoved_);
                float v = baseVel;

                // フレーム落ち時に一度に下がりすぎるのを防ぐ
                if (remaining <= 0.0f) {
                    v = 0.0f;
                } else if (v * step > remaining) {
                    v = remaining / step;
                }

                // 面向プレイヤー、反方向後退
                velocity_.x = -static_cast<float>(attackFacing_) * v;
            }


            // Ultimate: Windup 中にも少し先に後退し、その後で左右往復ダッシュを開始する
            if (queuedAttack_ == BossAttack::Ultimate && stateTimer_ > 0.0f) {
                const float targetDist = ultimateWindupBackstepDist_;
                const float step = StepScale(dt);

                float total = ultimateWindupTotal_;
                if (total < 0.001f) { total = (std::max)(stateTimer_, 0.001f); }

                // Windup 総時間に合わせて均等に後退
                float baseVel = targetDist / (total * 60.0f); // 60fps を基準にした「1フレーム移動量」
                baseVel = (std::min)(baseVel, ultimateWindupBackstepMaxSpeed_);
                baseVel = (std::max)(baseVel, ultimateWindupBackstepMinSpeed_);

                float remaining = (std::max)(0.0f, targetDist - ultimateWindupBackstepMoved_);
                float v = baseVel;

                // フレーム落ち時に一度に下がりすぎるのを防ぐ
                if (remaining <= 0.0f) {
                    v = 0.0f;
                } else if (v * step > remaining) {
                    v = remaining / step;
                }

                // 面向ダッシュ方向、反方向後退（初動）
                velocity_.x = -static_cast<float>(attackFacing_) * v;
            }
            if (stateTimer_ <= 0.0f) {
                // 技の開始後に予備動作の揺れを引きずらないよう、ここで確実に止める
                preAttackJitter_ = { 0.0f, 0.0f, 0.0f };
                preAttackJitterTime_ = 0.0f;

                if (queuedAttack_ == BossAttack::Dash) {
                    bossState_ = BossState::Dash;
                    stateTimer_ = queuedDashDuration_;
                    // 同一フレームでそのまま Dash を開始（余計に 1 フレーム待たない）
                    velocity_.x = static_cast<float>(attackFacing_) * queuedDashSpeed_;
                }
                else if (queuedAttack_ == BossAttack::Ranged) {
                    // プレイヤー密着時は近距離弾幕を使わず、前方への小ダッシュに切り替える
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

                        // -------- 射撃模式选择 --------
                        // 低HP & 中距離では「扇状散布」に切り替えてより派手にする
                        fanShot_ = (hp_ <= enrageHp_) && (dist <= 9.5f);

                        if (fanShot_) {
                            stateTimer_ = 0.95f;
                            shotsTotal_ = 2;           // 2 波の散射
                            shotInterval_ = 0.22f;
                        }
                        else {
                            // 原有: 単発 / 3連射（縦方向の散布）
                            stateTimer_ = (hp_ <= enrageHp_) ? 0.85f : 0.55f;
                            shotsTotal_ = (hp_ <= enrageHp_) ? 3 : 1;
                            shotInterval_ = 0.16f;
                        }

                        shotsLeft_ = shotsTotal_;
                        shotTimer_ = 0.0f; // 立即発射
                    }
                }
                else if (queuedAttack_ == BossAttack::Barrage) {
                    bossState_ = BossState::Barrage;

                    // 低HP時より久、より密集
                    stateTimer_ = (hp_ <= enrageHp_) ? (barrageDuration_ + 0.25f) : barrageDuration_;
                    barrageFireTimer_ = 0.0f;
                    barrageBurstTimer_ = barrageBurstInterval_ * 0.65f;

                    // 角度を少しランダムにして、毎回同じにならないようにする
                    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                    barrageAngle_ = r * 6.283185307179586f; // 2*pi
                    barrageSpinDir_ = (r < 0.5f) ? 1.0f : -1.0f;
                }
                else if (queuedAttack_ == BossAttack::Nova) {
                    bossState_ = BossState::Nova;

                    // 低HP時多1リング、より密集
                    // すべてのリングを出し切るだけの十分な時間を確保する
                    novaFireTimer_ = 0.0f; // 最初のリングを即時に放つ
                    novaRingsLeft_ = (hp_ <= enrageHp_) ? novaRingsEnrage_ : novaRingsNormal_;
                    stateTimer_ = novaDuration_ + novaRingInterval_ * (std::max)(0, novaRingsLeft_ - 1);

                    // リング弾幕ごとに角度を少し回して、「固定模様」を避ける
                    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                    novaRingOffset_ = r * 6.283185307179586f;
                }
                else if (queuedAttack_ == BossAttack::Slam) {
                    bossState_ = BossState::Jump;
                    stateTimer_ = slamMaxAirTime_;

                    // ジャンプ開始（叩きつけ）
                    velocity_.x = 0.0f;
                    velocity_.y = slamJumpVel_;
                    slamSpawned_ = false;
                }
                else if (queuedAttack_ == BossAttack::Ultimate) {
                    bossState_ = BossState::Ultimate;
                    stateTimer_ = ultimateDuration_;
                    ultimateBounces_ = 0;
                    // 必殺技開始時に向きを 1 方向へ固定し、まずプレイヤーのいる方向へ突っ込む
                    attackFacing_ = (pPos.x - position_.x >= 0.0f) ? -1 : 1; // 朝プレイヤー方向
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
    // 必殺技: 左右来回ダッシュ
    // 要件: 必殺技直前（Windup）で 1 回だけ後退し、必殺技中の往復反射では追加の後退を行わない
    velocity_.x = attackFacing_ * ultimateSpeed_;

    // failsafe: 最長持続
    if (stateTimer_ <= 0.0f) {
        bossState_ = BossState::Rest;
        stateTimer_ = restDuration_;

        FinishUltimateCooldown();
        ultimateLocked_ = false;
    }
    break;
}

        case BossState::Rest:
            // 大招後休息几秒
            velocity_.x = 0.0f;
            if (stateTimer_ <= 0.0f) {
                bossState_ = BossState::Chase;
                decisionTimer_ = 0.25f;
            }
            break;

        case BossState::Barrage:
        {
            // 保険: 攻撃段階では予備動作の揺れを強制的に止める（発射と重ならないようにする）
            preAttackJitter_ = { 0.0f, 0.0f, 0.0f };
            preAttackJitterTime_ = 0.0f;


            // 旋转弾幕: その場で発射、靠角度持続旋转制造「弾幕地獄」效果
            velocity_.x = 0.0f;
            facing_ = attackFacing_;

            // 低HP時より密集/旋转より快
            const float fireInterval = (hp_ <= enrageHp_) ? (barrageFireInterval_ * 0.85f) : barrageFireInterval_;
            const float angSpeed = (hp_ <= enrageHp_) ? (barrageAngularSpeed_ * 1.15f) : barrageAngularSpeed_;

            // 追加: 周期的な「爆発リング」を入れて、弾幕により層を持たせる
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

                // 弾幕スポーン地点: 胸の少し前
                Vector3 spawn = position_;
                spawn.x += attackFacing_ * (width_ * 0.15f);
                spawn.y += height_ * 0.18f;

                // 1〜2 発発射: 低HP時は 2 連射でより派手にする
                const int emitCount = (hp_ <= enrageHp_) ? 2 : 1;
                for (int i = 0; i < emitCount; ++i) {
                    float ang = barrageAngle_ + (i == 0 ? 0.0f : kPi);
                    Vector3 dir{ std::cos(ang), std::sin(ang), 0.0f };
                    Vector3 vel{ dir.x * barrageProjectileSpd_, dir.y * barrageProjectileSpd_, 0.0f };
                    SpawnBossProjectileRaw(spawn, vel, barrageProjectileLife_, 0.32f);
                }

                // 角度推進: 按回転方向旋转
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
            // 保険: 攻撃段階では予備動作の揺れを強制的に止める（発射と重ならないようにする）
            preAttackJitter_ = { 0.0f, 0.0f, 0.0f };
            preAttackJitterTime_ = 0.0f;

            // 円形爆発: 多重リング弾幕
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

                // 第1リングには追加で「十字」状の強い弾を入れ、画面をよりド派手にする
                if (ringIndex == 0) {
                    const float spd2 = speed * 1.25f;
                    SpawnBossProjectileRaw(c, {  spd2, 0.0f, 0.0f }, novaProjectileLife_, 0.34f);
                    SpawnBossProjectileRaw(c, { -spd2, 0.0f, 0.0f }, novaProjectileLife_, 0.34f);
                    SpawnBossProjectileRaw(c, { 0.0f,  spd2, 0.0f }, novaProjectileLife_, 0.34f);
                    SpawnBossProjectileRaw(c, { 0.0f, -spd2, 0.0f }, novaProjectileLife_, 0.34f);
                }

                novaRingOffset_ += 0.35f;
            };

            // 各リングを放つ
            novaFireTimer_ -= dt;
            while (novaFireTimer_ <= 0.0f && novaRingsLeft_ > 0) {
                novaFireTimer_ += novaRingInterval_;
                const int ringIndex = totalRings - novaRingsLeft_;
                emitRing(ringIndex);
                novaRingsLeft_--;
            }

            if (stateTimer_ <= 0.0f) {
                // 極端なフレーム落ち時は残りのリングを一気に補い、発射漏れを避ける
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
            // 保険: 攻撃段階では予備動作の揺れを強制的に止める（発射と重ならないようにする）
            preAttackJitter_ = { 0.0f, 0.0f, 0.0f };
            preAttackJitterTime_ = 0.0f;

            // 跳び上がり叩きつけ: 空中不做水平移動（予備動作を読みやすくする）
            velocity_.x = 0.0f;
            facing_ = attackFacing_;

            // failsafe: 空中に長く居すぎる場合（一部のマップ／衝突の極端なケース）は強制的に Slam に入る
            if (stateTimer_ <= 0.0f) {

                bossState_ = BossState::Slam;
                stateTimer_ = slamImpactHold_;
                slamSpawned_ = false;
            }
            break;

        case BossState::Slam:
        {
            // 着地叩きつけ: 少し止まって衝撃波弾幕を生成
            velocity_.x = 0.0f;
            facing_ = attackFacing_;

            if (!slamSpawned_) {
                slamSpawned_ = true;

                // 衝撃波: 沿地面左右拡散
                Vector3 base = position_;
                base.y -= height_ * 0.50f - 0.25f;

                SpawnBossProjectileRaw(base, { -slamWaveSpeed_, 0.0f, 0.0f }, slamWaveLife_, 0.36f);
                SpawnBossProjectileRaw(base, {  slamWaveSpeed_, 0.0f, 0.0f }, slamWaveLife_, 0.36f);

                // 破片: 向上扇形噴き出す（より派手）
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

                // 低HP時はさらに小さな破片を 1 層追加する
                if (hp_ <= enrageHp_) {
                    emitShard(-0.30f, 1.20f);
                    emitShard( 0.30f, 1.20f);
                }

                // 追加: 着地衝撃の「爆発リング」（より派手）
                {
                    Vector3 c = base;
                    c.y += 0.45f; // 少し持ち上げて、生成直後に地面ブロックへ当たらないようにする
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
            // プレイヤー密着時は継続射撃を行わず、小ダッシュに切り替える
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


            // 遠距離射撃: その場で照準
            velocity_.x = 0.0f;
            facing_ = attackFacing_;

            // 连射计時
            shotTimer_ -= dt;

            if (shotsLeft_ > 0 && shotTimer_ <= 0.0f) {
                // 弾幕の発生位置を計算
                Vector3 spawn = position_;
                spawn.x += attackFacing_ * (width_ * 0.5f + 0.25f);
                spawn.y += height_ * 0.15f;

                // 目標を予測
                Vector3 aimTarget{ pPos.x + pVel.x * projectileLeadTime_, pPos.y + pVel.y * projectileLeadTime_, 0.0f };

                // -------- 弾幕模式: 扇状散布 / 原有3連射 --------
                // 3連射: 少し縦方向に散らす
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
                    // 第二波やや微旋转、見た目がより豊か
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
            // 後隙: 無限連携にせず、プレイヤーへ反撃の猶予を与える
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
        // マップ衝突: 「X → Y」の順でマスを走査して補正する
        const float prevXBeforeMove = position_.x;
        const bool wasOnGround = isOnGround_;
        ResolveMapCollision(map, dt);

        // Dash Windup: 累计後退距離（打ち切り用）
        if (bossState_ == BossState::Windup && queuedAttack_ == BossAttack::Dash) {
            dashBackstepMoved_ += std::fabs(position_.x - prevXBeforeMove);
        }
        // Ultimate Windup: 累计後退距離（打ち切り用）
        if (bossState_ == BossState::Windup && queuedAttack_ == BossAttack::Ultimate) {
            ultimateWindupBackstepMoved_ += std::fabs(position_.x - prevXBeforeMove);
        }


        // Jump -> Slam: 检测落地瞬間トリガー叩きつけ
        if (bossState_ == BossState::Jump && !wasOnGround && isOnGround_) {
            bossState_ = BossState::Slam;
            stateTimer_ = slamImpactHold_;
            slamSpawned_ = false;
        }

        // Dash: 壁衝突ペナルティ
        if (bossState_ == BossState::Dash && std::fabs(velocity_.x) < 0.0001f && stateTimer_ > 0.0f) {
            bossState_ = BossState::Recover;
            stateTimer_ = 0.45f; // 壁衝突ペナルティのウィンドウ
        }

        // Ultimate: 碰到ブロック/トゲそのまま算「反射/トリガー」、来回几次後入る休息
        if (bossState_ == BossState::Ultimate) {
            // トゲ判定: 足元のマスがトゲなら、その場で即座に必殺技を終了する
            {
                Vector3 foot = position_;
                // 地形判定用身体サイズ（Dash の hurtbox 縮小の影響を受けないようにする）
                foot.y -= mapColliderH_ * 0.5f - 0.05f;
                auto idx = map.GetMapChipIndexByPosition(foot);
                MapChipType t = map.GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
                if (t == MapChipType::kSpike) {
                    bossState_ = BossState::Rest;
                    stateTimer_ = restDuration_;
                    // 終了必殺技: 復帰 CD
                    FinishUltimateCooldown();
                    ultimateLocked_ = false;

                    // 追加で 1 周分の「締めの爆発リング」を出して、より派手にする
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

            // ブロック: X 方向で止められた（velocity_.x==0）ら 1 回の反射とみなす
            if (bossState_ == BossState::Ultimate && std::fabs(velocity_.x) < 0.0001f) {
                ultimateBounces_++;
                attackFacing_ *= -1;
                facing_ = attackFacing_;

                // 反射のたびに 1 周分を放ち、打撃感を強くする
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
    // 必殺技が外部ロジックで中断された場合（たとえばプレイヤー接触で被弾／硬直が発生した場合）でも、CD が 399 のまま固定されないようにする
    if (ultimateLocked_ && bossState_ != BossState::Ultimate && ultimateCD_ > 100.0f) {
        FinishUltimateCooldown();
        ultimateLocked_ = false;
    }

    // ===== Dash 時にプレイヤーとの判定（hurtbox）を縮小 =====
    // 説明: ここで縮めるのは width_/height_（プレイヤー相互作用判定）で、マップ衝突には引き続き mapColliderW_/H_ を使う。
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
        // Boss のモデルは左向き
        if (type_ == EnemyType::Boss) {
            const float rotY = (facing_ >= 0) ? kPi : 0.0f;
            obj_->SetRotate({ 0.0f, rotY, 0.0f });
        }
        obj_->SetTranslate(GetRenderPosition());
        obj_->Update();
    }
}

void BossEnemy::Draw()
{
    if (isDead_) { return; }

    // Boss 弾幕は Boss の点滅に連動して非表示にしない（そうしないとプレイヤーには「弾が消えた」ように見える）
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

    // 同一フレーム／同一重なりで反復ダメージにならないようにする
    if (stompInvuln_ > 0.0f) { return; }

    if (type_ == EnemyType::Boss) {
        // Boss: 踏みつけ 30 次死亡（デフォルト）
        hp_ -= 1;

        // 被弾フィードバック: 点滅より久少し + 短硬直（より容易连续踩）
        StartHitReaction(0.55f);
        bossState_ = BossState::Stunned;
        stateTimer_ = 0.60f;
        decisionTimer_ = 0.20f;

        // 踏まれた直後にすぐ反撃しないようにする
        globalAttackCD_ = (std::max)(globalAttackCD_, 0.80f);

        stompInvuln_ = 0.90f;

        if (hp_ <= 0) {
            isDead_ = true;
            // 死亡時に弾幕をクリア
            for (auto& p : projectiles_) {
                p.active = false;
                p.life = 0.0f;
            }
        }
    }
    else {
        // 通常敵: 先に保留原挙動（1回だけ点滅）
        StartHitReaction(0.40f);
        stompInvuln_ = 0.20f;
    }
}

void BossEnemy::ResolveMapCollision(const MapChipField& map, float dt)
{
    const float step = StepScale(dt);
    // マップ衝突には「固定サイズ」だけを使い、Dash の hurtbox 縮小の影響を受けないようにする
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
                    if (velocity_.y > 0.0f) {          // 頂头
                        fixY = r.bottom - kHalfH;
                        velocity_.y = 0.0f;
                    } else if (velocity_.y < 0.0f) {   // 着地
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
    // マップセルを使って「足元に実体ブロックがあるか」を判定
    const Vector3 pPos = player.GetPosition();
    const float halfW = player.GetWidth()  * 0.5f;
    const float halfH = player.GetHeight() * 0.5f;

    // 少し下へ探って、浮動小数誤差でちょうど境界に乗った場合の判定漏れを避ける
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

bool BossEnemy::IsBattleTriggerReady(const Player& player, const MapChipField& map) const
{
    if (type_ != EnemyType::Boss) { return false; }
    if (battleTriggered_) { return false; }

    const Vector3 pPos = player.GetPosition();
    const float halfW = player.GetWidth()  * 0.5f;
    const float halfH = player.GetHeight() * 0.5f;

    // ① 横方向: 「左境界」を使い、中心点ではなくトリガー列を越えたかを判定（防止端トリガー）
    const float edgeEps = 0.02f; // 小さな余裕、避ける浮点ちょうど好卡辺
    auto leftIdx = map.GetMapChipIndexByPosition({ pPos.x - halfW + edgeEps, pPos.y, 0.0f });
    const bool passedX = (leftIdx.xIndex >= battleTriggerXIndex_);

    // ② 縦方向: プレイヤーが必ず Boss エリア付近まで降りてきていることを確認する（上層足場でのトリガーを避ける）
    const float dy = std::fabs(pPos.y - position_.y);
    const bool passedY = (dy <= battleTriggerVerticalRange_);

    // ③ 地面: 足元を支えるブロックもトリガー列より先にあることを要求する（外側の地面でのトリガーを避ける）
    bool onGroundInBossArea = true;
    if (requirePlayerOnGroundToTrigger_) {
        const float probeY = 0.06f;
        Vector3 probes[3] = {
            { pPos.x,                 pPos.y - halfH - probeY, 0.0f },
            { pPos.x - halfW * 0.80f, pPos.y - halfH - probeY, 0.0f },
            { pPos.x + halfW * 0.80f, pPos.y - halfH - probeY, 0.0f },
        };

        onGroundInBossArea = false;
        for (const auto& q : probes) {
            auto idx = map.GetMapChipIndexByPosition(q);

            // 重要: 必ずトリガー列より先の地面を踏んだ場合のみ有効にする
            if (idx.xIndex < battleTriggerXIndex_) { continue; }

            MapChipType t = map.GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
            if (IsSolid(t)) {
                onGroundInBossArea = true;
                break;
            }
        }
    }

    return passedX && passedY && onGroundInBossArea;
}


void BossEnemy::TriggerBattleNow()
{
    if (type_ != EnemyType::Boss) { return; }
    if (battleTriggered_) { return; }
    battleTriggered_ = true;

    // ちょうど入る戦闘: 重置少しテンポ、避ける「ちょうどトリガーそのまま即座に密着必殺技」
    bossState_ = BossState::Idle;
    queuedAttack_ = BossAttack::None;
    stateTimer_ = 0.0f;
    decisionTimer_ = 0.30f;
    globalAttackCD_ = (std::max)(globalAttackCD_, 0.60f);

    // クリアする残留弾幕（オブジェクト再度利用やロード時などに備えて）
    for (auto& p : projectiles_) {
        p.active = false;
        p.life = 0.0f;
    }
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
    // 案A（要件に応じて）: のみ「接近 Boss」で初めて表示HPバー
    //return IsInEngageRange(map, player);

    // 案B（より一般的）: 一度 Boss 戦がトリガーされたら、その後は常に表示する（距離で点滅させない）
    return battleTriggered_;
}

void BossEnemy::UpdateBossFacing(const Player& player)
{
    // 攻撃／予備動作／射撃中は向きを固定
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

void BossEnemy::UpdatePreAttackJitter(float dt)
{
    if (bossState_ != BossState::Windup) { return; }

    float lead = 0.0f;
    if (queuedAttack_ == BossAttack::Barrage) { lead = preJitterLeadBarrage_; }
    else if (queuedAttack_ == BossAttack::Nova) { lead = preJitterLeadNova_; }
    else if (queuedAttack_ == BossAttack::Slam) { lead = preJitterLeadJump_; } // Slam->Jump 前
    else { return; }

    if (lead <= 0.0f) { return; }

    const float settle = std::clamp(preJitterSettle_, 0.0f, lead);

    // 還無入る「最後 lead 秒」時不抖、かつ相位ゼロクリア、避ける突然跳相位
    if (stateTimer_ > lead || stateTimer_ <= 0.0f) {
        if (stateTimer_ > lead) { preAttackJitterTime_ = 0.0f; }
        return;
    }

    // 最後の settle 秒に入ったら揺れを止め、少しだけ静止した瞬間を作ってから技を出す
    if (stateTimer_ <= settle) {
        preAttackJitterTime_ = 0.0f;
        return;
    }

    preAttackJitterTime_ += dt;

    // jitter 有効区間: (settle, lead]
    const float seg = (std::max)(0.0001f, lead - settle);
    float k = 1.0f - ((stateTimer_ - settle) / seg); // 0 -> 1
    k = std::clamp(k, 0.0f, 1.0f);

    const float ampX = preJitterAmpX_ * k;
    const float ampY = preJitterAmpY_ * k;
    const float t = preAttackJitterTime_;

    // 異なる周波数の正弦波を合成し、単振動ではなく「微振動」にする
    const float nx = std::sin(t * 97.0f) + 0.35f * std::sin(t * 211.0f + 1.1f);
    const float ny = std::sin(t * 131.0f + 2.7f) + 0.35f * std::sin(t * 233.0f + 0.2f);

    preAttackJitter_.x = nx * ampX;
    preAttackJitter_.y = ny * ampY;
    preAttackJitter_.z = 0.0f;
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
        // ブロックに当たったら消える
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
    // 空いているスロットを探す
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

    // 満杯なら破棄する（無限に増えないようにする）
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
            pr.active = false; // 命中後に弾丸を消費
            return true;
        }
    }

    return false;
}