#pragma once

#include "Object3d.h"
#include "Object3dCommon.h"
#include "Camera.h"
#include "Vector3.h"

#include <cstdint>
#include <memory>

class MapChipField;
class Player;

// Enemy の種類（既存ロジック保持）
enum class EnemyType : uint8_t {
    Type0 = 0,   // 普通敵（見た目0）
    Type1 = 1,   // 普通敵（見た目1）
    Boss  = 2,   // Boss
};

// ============================================================
// Enemy 基类（接口不变）：GameScene 仍然只依赖 Enemy*
//  - 普通敌人：NormalEnemy
//  - Boss：BossEnemy
// ============================================================
class Enemy {
public:
    Enemy() = default;
    virtual ~Enemy() = default;

    virtual void Initialize(
        Object3dCommon* common,
        Camera* camera,
        const Vector3& spawnPos,
        EnemyType type
    ) = 0;

    // Boss 会用到 map/player；普通敌人也走同一个接口（保持原逻辑）
    virtual void Update(float deltaTime, const MapChipField& map, const Player& player) = 0;

    virtual void Draw() = 0;

    EnemyType GetType() const { return type_; }
    const Vector3& GetPosition() const { return position_; }

    float GetWidth()  const { return width_; }
    float GetHeight() const { return height_; }

    void StartHitReaction(float duration);
    bool IsHitReacting() const { return isHitReacting_; }

    // ====== 踩头伤害 / 死亡判定 ======
    // 普通敌人：保持原行为（只闪一下）；Boss：扣血/硬直/死亡
    virtual void OnStomp();
    bool IsDead() const { return isDead_; }
    int  GetHp()  const { return hp_; }

    // Boss：踩头无敌时间是否结束（用于 GameScene 避免重复扣血）
    virtual bool CanTakeStompDamage() const { return stompInvuln_ <= 0.0f; }

    // ====== Boss 远程弹幕命中检测（GameScene 调用）======
    // 命中则会“消耗”弹丸（inactive），返回 true
    virtual bool CheckBossProjectileHit(const Player& /*player*/) { return false; }

    // ====== 与玩家的碰撞/踩头判定（普通敌人/关卡逻辑可复用） ======
    struct PlayerContact {
        bool overlap = false; // AABB 重叠
        bool stomp   = false; // “踩头”判定成立（从上方下落命中）
    };

    // 不改变现有 GameScene 结构：需要时可直接调用。
    // 说明：
    //  - overlap：双方 AABB 是否重叠
    //  - stomp：在 overlap 的基础上，玩家处于下落且玩家底部高于敌人顶部（带 margin）
    PlayerContact CheckPlayerContact(const Player& player, float stompMargin = 0.10f) const;

protected:
    // --- 共享初始化 / 更新 / 绘制 ---
    void InitializeCommon(Object3dCommon* common, Camera* camera, const Vector3& spawnPos, EnemyType type);
    // 返回 false 表示已死亡（原逻辑：死亡后直接 return）
    bool UpdateCommon(float dt);
    // 只画本体（带受击闪烁逻辑）
    void DrawCommonBody();

protected:
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

    // --- 死亡/生命 ---
    bool  isDead_  = false;
    int   maxHp_   = 1;
    int   hp_      = 1;          // Boss: 30
    int   enrageHp_ = 0;         // Boss 残血阈值（用于加强攻击）
    float stompInvuln_ = 0.0f;  // 防止同一帧/连续重叠多次扣血
};
