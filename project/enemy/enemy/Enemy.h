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
/// <summary>
/// 通常敵とボス敵に共通する初期化、更新、描画、当たり判定のインターフェースを定義する基底クラス。
/// </summary>
class Enemy {
public:
    /// <summary>
    /// Enemyのインスタンスを生成します。
    /// </summary>
    Enemy() = default;
    /// <summary>
    /// Enemyが保持するリソースを破棄します。
    /// </summary>
    virtual ~Enemy() = default;

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="common">3Dオブジェクト生成に使用する共通描画処理。</param>
    /// <param name="camera">描画および座標変換に使用するカメラ。</param>
    /// <param name="spawnPos">生成時のワールド座標。</param>
    /// <param name="type">生成または設定する種類。</param>
    virtual void Initialize(
        MyEngine::Object3dCommon* common,
        MyEngine::Camera* camera,
        const MyEngine::Vector3& spawnPos,
        EnemyType type
    ) = 0;

    // Boss では map/player を使う。通常敵も同じインターフェースで処理し、元ロジックを維持する
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）。</param>
    /// <param name="map">地形情報と衝突判定に使用するマップデータ。</param>
    /// <param name="player">判定または更新対象のプレイヤー。</param>
    virtual void Update(float deltaTime, const MapChipField& map, const Player& player) = 0;

    /// <summary>
    /// 現在の状態を画面へ描画します。
    /// </summary>
    virtual void Draw() = 0;

    /// <summary>
    /// Typeを取得します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
    EnemyType GetType() const { return type_; }
    /// <summary>
    /// 現在のワールド座標を取得します。
    /// </summary>
    /// <returns>保持している値への参照。</returns>
    const MyEngine::Vector3& GetPosition() const { return position_; }

    /// <summary>
    /// 当たり判定の幅を取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    float GetWidth()  const { return width_; }
    /// <summary>
    /// 当たり判定の高さを取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    float GetHeight() const { return height_; }

    /// <summary>
    /// Hit Reactionを開始します。
    /// </summary>
    /// <param name="duration">継続時間（秒）。</param>
    void StartHitReaction(float duration);
    /// <summary>
    /// Hit Reactingかを判定します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool IsHitReacting() const { return isHitReacting_; }

    // ====== 踏みつけダメージ / 死亡判定 ======
    // 通常敵: 元の挙動を維持（1回だけ点滅）。Boss: ダメージ / 硬直 / 死亡を処理
    /// <summary>
    /// プレイヤーに踏まれた際のダメージと状態変化を処理します。
    /// </summary>
    virtual void OnStomp();
    /// <summary>
    /// 死亡状態かを取得します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool IsDead() const { return isDead_; }
    /// <summary>
    /// Hpを取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    int  GetHp()  const { return hp_; }

    /// <summary>
    /// 最大HPを取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    int  GetMaxHp() const { return maxHp_; }
    /// <summary>
    /// 最大HPに対する現在HPの割合を取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    float GetHpRatio() const { return (maxHp_ > 0) ? (static_cast<float>(hp_) / static_cast<float>(maxHp_)) : 0.0f; }

    // Boss: 踏みつけ無敵時間が終了したかどうか（GameScene 側で重複ダメージを避けるために使用）
    /// <summary>
    /// Take Stomp Damage可能かを判定します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    virtual bool CanTakeStompDamage() const { return stompInvuln_ <= 0.0f; }

    // ====== Boss 遠距離弾幕の命中判定（GameScene から呼び出す）======
    // 命中すると弾丸を消費し（inactive）、true を返す
    /// <summary>
    /// ボスの弾とプレイヤーの衝突を判定し、命中時の処理を行います。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    virtual bool CheckBossProjectileHit(const Player& /*player*/) { return false; }

    // ====== プレイヤーとの衝突/踏みつけ判定（通常敵／ステージロジックで再度利用可能） ======
    /// <summary>
    /// 敵とプレイヤーの接触判定結果および接触方向を保持する構造体。
    /// </summary>
    struct PlayerContact {
        bool overlap = false; // AABB 重叠
        bool stomp   = false; // 「踏みつけ」判定成立（上から落下して命中）
    };

    // 既存の GameScene 構造は変えず、必要なら直接呼び出せる。
    // 説明:
    // - overlap: 両者 AABB かどうか重なり
    // - stomp: overlap のうえで、プレイヤーが落下中かつプレイヤー下端が敵上端より高い位置にある（margin あり）
    /// <summary>
    /// Player Contactを判定します。
    /// </summary>
    /// <param name="player">判定または更新対象のプレイヤー。</param>
    /// <param name="stompMargin">処理に使用するstompMarginの値。</param>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    PlayerContact CheckPlayerContact(const Player& player, float stompMargin = 0.10f) const;

protected:
    // --- 共有初期化 / 更新 / 描画 ---
    /// <summary>
    /// Initialize Common処理を実行します。
    /// </summary>
    /// <param name="common">3Dオブジェクト生成に使用する共通描画処理。</param>
    /// <param name="camera">描画および座標変換に使用するカメラ。</param>
    /// <param name="spawnPos">生成時のワールド座標。</param>
    /// <param name="type">生成または設定する種類。</param>
    void InitializeCommon(MyEngine::Object3dCommon* common, MyEngine::Camera* camera, const MyEngine::Vector3& spawnPos, EnemyType type);
    // false を返した場合は死亡済みを示す（元ロジック: 死亡後は即 return）
    /// <summary>
    /// Commonを更新します。
    /// </summary>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    /// <returns>判定結果。</returns>
    bool UpdateCommon(float dt);
    // 本体のみを描画する（被弾点滅ロジック付き）
    /// <summary>
    /// Common Bodyを描画します。
    /// </summary>
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
