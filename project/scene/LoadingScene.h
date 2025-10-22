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

    Sprite* blackSprite_ = nullptr;

    // ===== Spinner (白点转圈) =====
    std::vector<Sprite*> spinnerDots_ = {};
    int   spinnerCount_ = 12;     // 点的数量
    float spinnerRadius_ = 32.0f;  // 半径(像素)
    float spinnerSize_ = 10.0f;  // 每个点的正方形尺寸(像素)
    float spinnerSpeed_ = 3.6f;   // 角速度(弧度/秒) ~ 206度/秒
    float spinnerHeadAngle_ = 0.0f;   // 头部当前角度
    float spinnerTrailLen_ = 0.9f;   // 尾巴长度(0~π)，越大尾巴越长
    float spinnerMinAlpha_ = 0.18f;  // 尾端最小透明度
    std::string spinnerTexPath_ = "Resources/white.png"; // 若没有white.png，可先用gray.png

    void CreateSpinner_();
    int showDelayFrames_ = 0;
};