#pragma once
#include "SceneManager.h"
#include <WinApp.h>
#include <DirectXCommon.h>
#include <Input.h>
#include <SrvManager.h>
#include "SpriteCommon.h"
class Framework {
public:
    virtual ~Framework() = default;

    virtual void Initialize();
    virtual void Update();
    virtual void Draw();
    virtual void Finalize();


    void Run();  // 主循环

protected:
    bool endRequest_ = false;
    std::unique_ptr<SceneManager> sceneManager_;
};
