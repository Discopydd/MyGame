#pragma once
#include <vector>
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "ParticleEmitter.h"
#include <memory>
// すべての粒子エミッタを一括管理する
/// <summary>
/// ParticleManagerに関する処理と状態を管理するクラスです。
/// </summary>
class ParticleManager
{
public:
    /// <summary>
    /// ParticleManagerのインスタンスを生成します。
    /// </summary>
    ParticleManager() = default;
    /// <summary>
    /// ParticleManagerが保持するリソースを破棄します。
    /// </summary>
    ~ParticleManager();   // デストラクタで自動解放する

    // 初期化: 3D / 2D の共通オブジェクトを受け取る
    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="objectCommon">3Dオブジェクト生成に使用する共通描画処理。</param>
    /// <param name="spriteCommon">スプライト生成に使用する共通描画処理。</param>
    void Initialize(MyEngine::Object3dCommon* objectCommon, MyEngine::SpriteCommon* spriteCommon);

    // 新しいエミッタを生成し、マネージャがライフサイクルを管理する
    /// <summary>
    /// Emitterを生成します。
    /// </summary>
    /// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
    ParticleEmitter* CreateEmitter();

    // 毎フレームすべてのエミッタを更新する
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    void Update(float dt);

    // 3D 粒子（モデル粒子）を描画する
    /// <summary>
    /// 3D要素を画面へ描画します。
    /// </summary>
    void Draw3D();

    // 2D 粒子（MyEngine::Sprite 粒子）を描画する
    /// <summary>
    /// 2D要素を画面へ描画します。
    /// </summary>
    void Draw2D();

    // すべてのエミッタを手動でクリアする（任意。シーン切り替え時にも明示的に呼び出せる）
    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize();

private:
    MyEngine::Object3dCommon* objCommon_ = nullptr;
    MyEngine::SpriteCommon* sprCommon_   = nullptr;
    std::vector<std::unique_ptr<ParticleEmitter>> emitters_;
};
