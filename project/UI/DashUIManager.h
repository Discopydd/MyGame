#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include <player/Player.h>
#include <memory>

/// <summary>
/// DashUIManagerに関する処理と状態を管理するクラスです。
/// </summary>
class DashUIManager {
public:
    /// <summary>
    /// DashUIManagerのインスタンスを生成します。
    /// </summary>
    DashUIManager() = default;
    /// <summary>
    /// DashUIManagerが保持するリソースを破棄します。
    /// </summary>
    ~DashUIManager() = default;

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="spriteCommon">スプライト生成に使用する共通描画処理。</param>
    /// <param name="player">判定または更新対象のプレイヤー。</param>
    void Initialize(MyEngine::SpriteCommon* spriteCommon, Player* player);
    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize();

    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    void Update(float dt);
    /// <summary>
    /// 現在の状態を画面へ描画します。
    /// </summary>
    void Draw();

private:
    MyEngine::SpriteCommon* spriteCommon_ = nullptr;
    Player*       player_       = nullptr;

    std::unique_ptr<MyEngine::Sprite> icon_;    // スキルアイコン
    std::unique_ptr<MyEngine::Sprite> overlay_; // クールダウンのグレーオーバーレイ

    MyEngine::Vector2 overlayFullTexSize_{ 0.0f, 0.0f };
};
