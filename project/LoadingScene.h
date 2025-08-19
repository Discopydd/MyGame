#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "SoundManager.h"
#include "Camera.h"
#include <vector>
#include "BaseScene.h"
#include <thread>
#include <atomic>

class LoadingScene : public BaseScene {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;
    
    // 设置加载进度（0.0到1.0）
    void SetProgress(float progress) { progress_ = progress; }

private:
    WinApp* winApp_ = nullptr;
    DirectXCommon* dxCommon_ = nullptr;
    Input* input_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;
    Sprite* loadingSprite_ = nullptr;
    
    // 进度条相关
    float progress_ = 0.0f;
    Sprite* progressBar_ = nullptr;
    Sprite* progressBackground_ = nullptr;
};