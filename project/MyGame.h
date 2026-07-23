#pragma once

#include "scene/Framework.h"
#include "scene/GameScene.h"
#include "scene/TitleScene.h"
#include "BaseScene.h"

/// <summary>
/// MyGameに関する処理と状態を管理するクラスです。
/// </summary>
class MyGame : public Framework {
public:
    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    void Initialize() override;
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    void Update() override;
    /// <summary>
    /// 現在の状態を画面へ描画します。
    /// </summary>
    void Draw() override;
    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize() override;
};
