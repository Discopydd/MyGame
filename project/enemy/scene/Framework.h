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
    virtual ~Framework() = default;

    virtual void Initialize();
    virtual void Update();
    virtual void Draw();
    virtual void Finalize();


    void Run();  // メインループ

protected:
    bool endRequest_ = false;
    std::unique_ptr<MyEngine::SceneManager> sceneManager_;
};
