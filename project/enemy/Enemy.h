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
    Type0 = 0,   // 普通敵（enemy0）
    Type1 = 1,   // 普通敵（enemy2）
    Type2 = 2,   // 普通敵（enemy3）
    Boss  = 3,   // Boss
};

// ============================================================
// Enemy 基底クラス（インターフェースはそのまま）: GameScene 引き続き依存するのは Enemy*
// - 通常敵: NormalEnemy
// - Boss: BossEnemy
// ============================================================
class Enemy {
public:
    Enemy() = default;
    virtual ~Enemy() = default;

    virtual void Initialize(
        MyEngine::Object3dCommon* common,
        MyEngine::Camera* camera,
        const MyEngine::Vector3& spawnPos,
        EnemyType type
    ) = 0;

    // Boss では map/player を使う。通常敵も同じインターフェースで処理し、元ロジックを維持する
    virtual void Update(float deltaTime, const MapChipField& map, const Player& player) = 0;

    virtual void Draw() = 0;

    EnemyType GetType() const { return type_; }
    const MyEngine::Vector3& GetPosition() const { return position_; }

    float GetWidth()  const { return width_; }
    float GetHeight() const { return height_; }

    void StartHitReaction(float duration);
    bool IsHitReacting() const { return isHitReacting_; }

    // ====== 踏みつけダメージ / 死亡判定 ======
    // 通常敵: 元の挙動を維持（1回だけ点滅）。Boss: ダメージ / 硬直 / 死亡を処理
    virtual void OnStomp();
    bool IsDead() const { return isDead_; }
    int  GetHp()  const { return hp_; }

    int  GetMaxHp() const { return maxHp_; }
    float GetHpRatio() const { return (maxHp_ > 0) ? (static_cast<float>(hp_) / static_cast<float>(maxHp_)) : 0.0f; }

    // Boss: 踏みつけ無敵時間が終了したかどうか（GameScene 側で重複ダメージを避けるために使用）
    virtual bool CanTakeStompDamage() const { return stompInvuln_ <= 0.0f; }

    // ====== Boss 远程弾幕命中判定（GameScene 呼び出す）======
    // 命中すると弾丸を消費し（inactive）、true を返す
    virtual bool CheckBossProjectileHit(const Player& /*player*/) { return false; }

    // ====== プレイヤーとの衝突/踏みつけ判定（通常敵／ステージロジックで再度利用可能） ======
    struct PlayerContact {
        bool overlap = false; // AABB 重叠
        bool stomp   = false; // 「踏みつけ」判定成立（上から落下して命中）
    };

    // 既存の GameScene 構造は変えず、必要なら直接呼び出せる。
    // 説明:
    // - overlap: 両者 AABB かどうか重なり
    // - stomp: overlap のうえで、プレイヤーが落下中かつプレイヤー下端が敵上端より高い位置にある（margin あり）
    PlayerContact CheckPlayerContact(const Player& player, float stompMargin = 0.10f) const;

protected:
    // --- 共有初期化 / より新 / 绘制 ---
    void InitializeCommon(MyEngine::Object3dCommon* common, MyEngine::Camera* camera, const MyEngine::Vector3& spawnPos, EnemyType type);
    // false を返した場合は死亡済みを示す（元ロジック: 死亡後は即 return）
    bool UpdateCommon(float dt);
    // 本体のみを描画する（被弾点滅ロジック付き）
    void DrawCommonBody();

protected:
    // ---------- 共通 ----------
    std::unique_ptr<MyEngine::Object3d> obj_;
    MyEngine::Vector3   position_{};
    EnemyType type_{ EnemyType::Type0 };

    // 当たり判定サイズ
    float width_  = 1.5f;
    float height_ = 1.5f;

    // --- 被弾点滅 ---
    bool  isHitReacting_       = false;
    float hitReactTimer_       = 0.0f;
    float damageBlinkTimer_    = 0.0f;
    float damageBlinkInterval_ = 0.08f;
    bool  damageBlinkVisible_  = true;

    // --- 死亡/生命 ---
    bool  isDead_  = false;
    int   maxHp_   = 1;
    int   hp_      = 1;          // Boss: 30
    int   enrageHp_ = 0;         // Boss 低HP閾値（攻撃強化用）
    float stompInvuln_ = 0.0f;  // 同一フレーム / 連続重なりで何度もダメージしないようにする
};
