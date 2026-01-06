#pragma once

#include "Enemy.h"

#include <vector>
#include <memory>

// ------------------ Boss ------------------
class BossEnemy final : public Enemy {
public:
    void Initialize(Object3dCommon* common, Camera* camera, const Vector3& spawnPos, EnemyType type) override;
    void Update(float deltaTime, const MapChipField& map, const Player& player) override;
    void Draw() override;

    void OnStomp() override;
    bool CheckBossProjectileHit(const Player& player) override;

// ================== Boss battle 触发 / 血条显示 ==================
// 目的：
//  - 只有玩家“进入 Boss 区域/接近 Boss”后，Boss 才开始攻击
//  - 只有接近 Boss 时才显示 Boss 血条（由 GameScene 调用）
bool IsBattleTriggered() const { return battleTriggered_; }

    // GameScene 用：当玩家到达 Boss 触发点时，先播“镜头演出”，
    // 演出结束后再调用 TriggerBattleNow() 让 Boss 正式开打。
    // 这样可以保证：镜头推到 Boss → 显示名字 → 镜头回玩家 → Boss 开始 AI/攻击 + 血条出现。
    bool IsBattleTriggerReady(const Player& player, const MapChipField& map) const;
    void TriggerBattleNow();

// GameScene 用：决定是否显示 Boss 血条（你也可以把逻辑改成“触发后一直显示”）
bool ShouldShowBossHp(const Player& player, const MapChipField& map) const;

// 可选：修改“触发列”（0-based）。例如 AH 列是 33。
void SetBattleTriggerXIndex(uint32_t xIndex) { battleTriggerXIndex_ = xIndex; }


private:
    // ---------- 运动/碰撞（Boss 用：position += velocity） ----------
    Vector3 velocity_{ 0,0,0 };
    bool    isOnGround_ = false;
    float   gravityBase_ = -2.20f;

    // ---------- 判定（玩家交互）与地图碰撞分离 ----------
    // 目标：Dash 时缩小“与玩家的判定”，但不要影响 Boss 的地图碰撞（否则会出现穿墙/卡墙手感变化）。
    // 约定：
    //  - width_/height_：用于玩家交互（受伤/踩头/近战命中 等）
    //  - mapColliderW_/mapColliderH_：仅用于 ResolveMapCollision（和地图方块的碰撞）
    float mapColliderW_ = 0.0f;
    float mapColliderH_ = 0.0f;

    // 非 Dash 时的基础“玩家判定”尺寸（用于每帧恢复）
    float baseHurtW_ = 0.0f;
    float baseHurtH_ = 0.0f;

    // Dash 时缩小判定比例（想更难打中就再调小一点，比如 0.60/0.75）
    float dashHurtScaleX_ = 0.70f;
    float dashHurtScaleY_ = 0.85f;

// ---------- Boss 战斗触发（未触发前：不更新 AI/不发弹/不显示血条） ----------
// 默认：玩家 xIndex >= 33（AH列，0-based）且站在地面上才唤醒 Boss
bool     battleTriggered_ = false;
uint32_t battleTriggerXIndex_ = 33;              // AH (0-based)
bool     requirePlayerOnGroundToTrigger_ = true; // 触发时要求玩家“在地面上”

// “接近 Boss 才攻击/显示血条”的判定（防止玩家在天上/隔层就把 Boss 激活）
bool  requirePlayerOnGroundToEngage_ = true; // 接近判定也要求玩家在地面
float engageVerticalRange_ = 6.0f;           // 允许的 |dy|（世界坐标）

bool IsPlayerOnGround(const MapChipField& map, const Player& player) const;
bool IsInEngageRange(const MapChipField& map, const Player& player) const;


    // ---------- Boss AI ----------
    // 说明：尽量沿用原状态机，新增少量状态做“更炫酷”的招式
    enum class BossState {
        Idle,
        Chase,
        Windup,
        Dash,
        Shoot,
        Barrage,   // 旋转弹幕（站桩/小范围移动的“弹幕地狱”）
        Nova,      // 圆形爆发（多环环形弹幕，更“炸裂”）
        Jump,      // 跳起（准备砸地）
        Slam,      // 落地砸地（生成冲击波弹幕）
        Ultimate,
        Rest,
        Recover,
        Stunned
    };
    BossState bossState_ = BossState::Idle;

    enum class BossAttack { None, Dash, Ranged, Barrage, Nova, Slam, Ultimate };
    BossAttack queuedAttack_ = BossAttack::None;

    float stateTimer_    = 0.0f;   // 前摇/攻击/后摇/硬直计时
    float decisionTimer_ = 0.0f;   // 限制频繁换招
    int   facing_        = 1;      // 1右 -1左
    int   attackFacing_  = 1;      // 本次攻击锁朝向

    // Dash 的持续时间（Windup -> Dash へ渡す）
    float queuedDashDuration_ = 0.30f;
    float queuedDashSpeed_    = 0.45f;  // Dash 实际使用速度（Windup -> Dash）
    bool  isShortDash_         = false;  // 贴脸小冲刺/微冲刺：用于更短后摇

    // 记录上一帧与玩家的距离（可用于“风筝检测”等）
    float prevDistToPlayer_ = 1e9f;

    // 出招节奏控制
    float globalAttackCD_ = 0.0f;  // 任意攻击最小间隔
    float meleeCD_        = 0.0f;  // 近战冷却
    float dashCD_         = 0.0f;  // 冲刺冷却
    float microDashCD_    = 0.0f;  // 贴脸小冲刺冷却（不占用 dashCD_）
    float rangedCD_       = 0.0f;  // 远程冷却
    float barrageCD_      = 0.0f;  // 旋转弹幕冷却
    float slamCD_         = 0.0f;  // 砸地冷却
    float novaCD_         = 0.0f;  // 圆形爆发冷却
    float ultimateCD_     = 3.0f;  // 大招冷却（开场给一点延迟）
    int   ultimateBounces_ = 0;    // 大招：已反弹次数

    // ---------- 调参区 ----------
    float moveSpeed_  = 0.18f;  // 追击速度（按“每帧位移”理解）
    float dashSpeed_  = 0.45f;  // 冲刺速度（每帧位移）
    float jumpVel_    = 0.62f;

    float detectRange_  = 18.0f;
    float idealRange_   = 4.0f;
    float meleeRange_   = 2.2f;
    float dashMinRange_ = 4.0f;
    float dashMaxRange_ = 10.0f;

    // 远程攻击距离
    float rangedMinRange_ = 4.2f;
    float rangedMaxRange_ = 14.0f;

    // 近距离冲刺：玩家贴近时立刻冲刺，但不会一直冲（靠 dashCD_ 控制）
    float closeDashRange_    = 4.8f;
    float closeDashWindup_   = 0.12f;
    float closeDashDuration_ = 0.16f;
    float closeDashSpeed_    = 0.30f;  // 贴脸小冲刺速度（每帧位移）
    float microDashCooldown_ = 1.20f;  // dashCD_ 没好时也允许一次小冲刺的冷却

    // 远距离压制：距离很远时优先远程
    float farRangedPrefer_   = 9.0f;

    // 大招：来回左右冲刺（碰到方块/刺算一次“反弹”），结束后休息
    float ultimateSpeed_      = 0.35f;
    float ultimateDuration_   = 5.0f; // failsafe：最长持续
    int   ultimateMaxBounces_ = 4;    // 反弹次数（4=左右来回两趟）
    float restDuration_       = 1.8f;

    float leadTime_ = 0.25f;           // 追击预判：playerPos + playerVel * leadTime
    float projectileLeadTime_ = 0.35f; // 弹幕瞄准预判

    // ---------- Boss 弹幕（对象池） ----------
    struct BossProjectile {
        std::unique_ptr<Object3d> obj;
        Vector3 pos{ 0,0,0 };
        Vector3 vel{ 0,0,0 };
        float   life = 0.0f;
        float   radius = 0.35f;
        bool    active = false;
    };

    // 弹幕更密集会更炫酷，因此池子稍微加大一点
    static constexpr int kMaxBossProjectiles_ = 48;
    std::vector<BossProjectile> projectiles_;

    float projectileSpeed_ = 0.25f;  // 每帧位移（跟 dashSpeed_ 同量纲）
    float projectileLife_  = 2.20f;  // 秒

    // ---------- 新增：炫酷招式参数 ----------
    // ① 旋转弹幕（Barrage）：围绕 Boss 旋转发射
    float barrageDuration_      = 1.35f; // 秒
    float barrageFireInterval_  = 0.055f; // 秒
    float barrageFireTimer_     = 0.0f;
    float barrageAngle_         = 0.0f;  // 弧度
    float barrageAngularSpeed_  = 7.0f;  // rad/sec
    float barrageSpinDir_       = 1.0f;  // +1/-1：旋转方向
    float barrageProjectileSpd_ = 0.22f; // 每帧位移（可独立于 projectileSpeed_）
    float barrageProjectileLife_= 2.10f;

    // Barrage 期间追加“爆环”让画面更炫
    float barrageBurstInterval_ = 0.38f;
    float barrageBurstTimer_    = 0.0f;
    int   barrageBurstCount_    = 10;
    float barrageBurstSpeed_    = 0.18f;
    float barrageBurstLife_     = 1.45f;

    // ①.5 圆形爆发（Nova）：多环环形弹幕
    float novaWindup_         = 0.26f;
    float novaDuration_       = 0.95f;
    float novaRecover_        = 0.85f;
    int   novaRingsNormal_    = 2;
    int   novaRingsEnrage_    = 3;
    float novaRingInterval_   = 0.16f;
    int   novaBulletCount_    = 18;
    float novaProjectileSpd_  = 0.26f;
    float novaProjectileLife_ = 1.65f;
    float novaRingOffset_     = 0.0f;
    float novaFireTimer_      = 0.0f;
    int   novaRingsLeft_      = 0;

    // ② 砸地（Slam）：跳起后落地生成“左右冲击波 + 破片”
    float slamWindup_      = 0.32f;
    float slamJumpVel_     = 0.92f;
    float slamMaxAirTime_  = 1.60f; // failsafe
    float slamImpactHold_  = 0.18f; // 落地停顿
    float slamRecover_     = 0.75f;
    float slamWaveSpeed_   = 0.38f;
    float slamWaveLife_    = 1.70f;
    float slamShardSpeed_  = 0.30f;
    float slamShardLife_   = 1.40f;
    bool  slamSpawned_     = false;

    // Shoot 状态内部连射
    int   shotsLeft_      = 0;
    int   shotsTotal_     = 0;
    float shotInterval_   = 0.16f;  // 秒
    float shotTimer_      = 0.0f;   // 秒（<=0 就发射）

    // ③ 新增：扇形散射（用 Shoot 状态内的“开关”实现，不增加额外状态）
    bool  fanShot_        = false;
    int   fanCount_       = 5;      // 每波弹数量
    float fanHalfAngle_   = 0.52f;  // 半角（弧度）≈ 30°
    float fanProjectileSpd_= 0.24f;

    bool ultimateLocked_ = false;     // 是否处于“把 ultimateCD_ 拉到 399 的锁定期”

    // 通用：环形弹幕工具（复用对象池）
    void SpawnRadialBurst(const Vector3& center, int count, float speed, float life, float radius, float angleOffset = 0.0f);

private:
    void ResolveMapCollision(const MapChipField& map, float dt);
    void UpdateBossFacing(const Player& player);

    void UpdateBossProjectiles(float dt, const MapChipField& map);
    void SpawnBossProjectile(const Vector3& spawnPos, const Vector3& aimDir);
    void SpawnBossProjectileRaw(const Vector3& spawnPos, const Vector3& velocity, float life, float radius);
    void DrawBossProjectiles();

    void FinishUltimateCooldown();
};
