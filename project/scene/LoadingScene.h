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
#include <memory>  // ★ 使用 std::unique_ptr

class LoadingScene : public BaseScene {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

    // 设置加载进度（0.0到1.0）
    void SetProgress(float progress) { progress_ = progress; }

private:
    // 非拥有：外部单例，保持裸指针即可
    WinApp*        winApp_    = nullptr;
    DirectXCommon* dxCommon_  = nullptr;
    Input*         input_     = nullptr;
    SrvManager*    srvManager_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;

    // 进度条相关
    float progress_ = 0.0f;
    std::unique_ptr<Sprite> progressBar_;
    std::unique_ptr<Sprite> progressBackground_;

    std::unique_ptr<Sprite> blackSprite_;

    // ===== Spinner (白点转圈) =====
    std::vector<std::unique_ptr<Sprite>> spinnerDots_;
    int   spinnerCount_     = 12;      // 点的数量
    float spinnerRadius_    = 20.0f;   // 半径(像素)
    float spinnerSize_      = 7.0f;    // 每个点的正方形尺寸(像素)
    float spinnerSpeed_     = 10.0f;   // 角速度(弧度/秒)
    float spinnerHeadAngle_ = 0.0f;    // 头部当前角度
    float spinnerTrailLen_  = 0.9f;    // 尾巴长度(0~π)，越大尾巴越长
    float spinnerMinAlpha_  = 0.18f;   // 尾端最小透明度
    std::string spinnerTexPath_ = "Resources/white_sphere.png";

    void CreateSpinner_();
};
