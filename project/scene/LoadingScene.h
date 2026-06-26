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
#include <memory>  // ★ std::unique_ptr を使用

class LoadingScene : public MyEngine::BaseScene {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

    // 設定読み込み進捗（0.0 から 1.0）
    void SetProgress(float progress) { progress_ = progress; }

private:
    // 非所有: 外部シングルトンなので生ポインタのままでよい
    MyEngine::WinApp*        winApp_    = nullptr;
    MyEngine::DirectXCommon* dxCommon_  = nullptr;
    MyEngine::Input*         input_     = nullptr;
    MyEngine::SrvManager*    srvManager_ = nullptr;
    MyEngine::SpriteCommon* spriteCommon_ = nullptr;

    // プログレスバー関連
    float progress_ = 0.0f;
    std::unique_ptr<MyEngine::Sprite> progressBar_;
    std::unique_ptr<MyEngine::Sprite> progressBackground_;

    std::unique_ptr<MyEngine::Sprite> blackSprite_;
    std::unique_ptr<MyEngine::Sprite> portalRingSprite_;
    float portalRingRotation_ = 0.0f;

    // ===== Spinner（白い点が回転する表示） =====
    std::vector<std::unique_ptr<MyEngine::Sprite>> spinnerDots_;
    int   spinnerCount_     = 12;      // 点の数
    float spinnerRadius_    = 20.0f;   // 半径（ピクセル）
    float spinnerSize_      = 7.0f;    // 各点の正方形サイズ（ピクセル）
    float spinnerSpeed_     = 10.0f;   // 角速度（ラジアン/秒）
    float spinnerHeadAngle_ = 0.0f;    // ヘッドの現在角度
    float spinnerTrailLen_  = 0.9f;    // テール長（0〜π）。値が大きいほど尾が長くなる
    float spinnerMinAlpha_  = 0.18f;   // 尾端の最小透明度
    std::string spinnerTexPath_ = "Resources/white_sphere.png";

    void CreateSpinner_();
};
