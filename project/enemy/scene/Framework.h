#pragma once
#include "SceneManager.h"
#include <WinApp.h>
#include <DirectXCommon.h>
#include <Input.h>
#include <SrvManager.h>
#include "SpriteCommon.h"
/// <summary>
/// ゲームループと各エンジン機能の初期化、更新、描画、終了処理を統括する基底クラス。
/// </summary>
class Framework {
public:
    /// <summary>
    /// Frameworkが保持するリソースを破棄します。
    /// </summary>
    virtual ~Framework() = default;

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    virtual void Initialize();
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    virtual void Update();
    /// <summary>
    /// 現在の状態を画面へ描画します。
    /// </summary>
    virtual void Draw();
    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    virtual void Finalize();


    /// <summary>
    /// Run処理を実行します。
    /// </summary>
    void Run();  // メインループ

protected:
    bool endRequest_ = false;
    std::unique_ptr<MyEngine::SceneManager> sceneManager_;
};
