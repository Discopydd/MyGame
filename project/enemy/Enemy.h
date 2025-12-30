#pragma once
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Camera.h"
#include "Vector3.h"

#include <cstdint>
#include <memory>
#include <vector>

class MapChipField;
class Player;

enum class EnemyType : uint8_t {
    Type0 = 0,   // 比如普通敌人
    Type1 = 1,   // 比如另一种造型
    Boss  = 2,   // Boss
};

class Enemy {
public:
    Enemy() = default;
    ~Enemy() = default;

    void Initialize(
        Object3dCommon* common,
        Camera* camera,
        const Vector3& spawnPos,
        EnemyType type
    );

    // Boss 会用到 map/player；普通敌人目前也走同一个接口
    void Update(float deltaTime, const MapChipField& map, const Player& player);

    void Draw();

    EnemyType GetType() const { return type_; }
    const Vector3& GetPosition() const { return position_; }

    float GetWidth()  const { return width_; }
    float GetHeight() const { return height_; }

    void StartHitReaction(float duration);
    bool IsHitReacting() const { return isHitReacting_; }

    // ====== 踩头伤害 / 死亡判定 ======
    void OnStomp();                 // 玩家从上方踩到时调用
    bool IsDead() const { return isDead_; }
    int  GetHp()   const { return hp_; }   // Boss: 30->0

    // Boss：踩头无敌时间是否结束（用于 GameScene 避免重复扣血）
    bool CanTakeStompDamage() const { return stompInvuln_ <= 0.0f; }

    // ====== Boss 远程弹幕命中检测（GameScene 调用）======
    // 命中则会“消耗”弹丸（inactive），返回 true
    bool CheckBossProjectileHit(const Player& player);

private:
    // ---------- 共通 ----------
    std::unique_ptr<Object3d> obj_;
    Vector3   position_{};
    EnemyType type_{ EnemyType::Type0 };

    // 碰撞体积
    float width_  = 1.5f;
    float height_ = 1.5f;

    // --- 受击闪烁 ---
    bool  isHitReacting_       = false;
    float hitReactTimer_       = 0.0f;
    float damageBlinkTimer_    = 0.0f;
    float damageBlinkInterval_ = 0.08f;
    bool  damageBlinkVisible_  = true;

    // --- 死亡 ---
    bool  isDead_  = false;
    int   maxHp_   = 1;
    int   hp_      = 1;          // Boss: 30
    int   enrageHp_ = 0;         // Boss 残血阈值（用于加强攻击）
    float stompInvuln_ = 0.0f;  // 防止同一帧/连续重叠多次扣血

    // ---------- 运动/碰撞（Boss 用：position += velocity） ----------
    Vector3 velocity_{ 0,0,0 };
    bool    isOnGround_ = false;
    float   gravityBase_ = -2.20f;

    // ---------- Boss AI ----------
    enum class BossState { Idle, Chase, Windup, Dash, Shoot, Ultimate, Rest, Recover, Stunned };
    BossState bossState_ = BossState::Idle;

    enum class BossAttack { None, Dash, Ranged, Ultimate };
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

    static constexpr int kMaxBossProjectiles_ = 12;
    std::vector<BossProjectile> projectiles_;

    float projectileSpeed_ = 0.25f;  // 每帧位移（跟 dashSpeed_ 同量纲）
    float projectileLife_  = 2.20f;  // 秒

    // Shoot 状态内部连射
    int   shotsLeft_      = 0;
    int   shotsTotal_     = 0;
    float shotInterval_   = 0.16f;  // 秒
    float shotTimer_      = 0.0f;   // 秒（<=0 就发射）

    bool ultimateLocked_ = false;     // 是否处于“把 ultimateCD_ 拉到 399 的锁定期”
   
private:
    void ResolveMapCollision(const MapChipField& map, float dt);
    void UpdateBossFacing(const Player& player);

    void UpdateBossProjectiles(float dt, const MapChipField& map);
    void SpawnBossProjectile(const Vector3& spawnPos, const Vector3& aimDir);
    void DrawBossProjectiles();

    void FinishUltimateCooldown();    // 把 ultimateCD_ 从 399 结算成正常随机CD
};
