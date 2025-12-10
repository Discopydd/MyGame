#include "LoadingScene.h"
#include <algorithm>
#include <cmath>

void LoadingScene::Initialize() {
    winApp_    = WinApp::GetInstance();
    dxCommon_  = DirectXCommon::GetInstance();
    input_     = Input::GetInstance();
    srvManager_ = SrvManager::GetInstance();

    spriteCommon_ = SpriteCommon::GetInstance();

    auto* tm = TextureManager::GetInstance();
    tm->Initialize(dxCommon_, srvManager_);

    TextureManager::GetInstance()->LoadTexture("Resources/black.png");

    // 黑幕
    blackSprite_ = std::make_unique<Sprite>();
    blackSprite_->Initialize(spriteCommon_, "Resources/black.png");
    blackSprite_->SetPosition({ 0.0f, 0.0f });
    blackSprite_->SetSize({
        static_cast<float>(WinApp::kClientWidth),
        static_cast<float>(WinApp::kClientHeight)
    });

    TextureManager::GetInstance()->LoadTexture(spinnerTexPath_.c_str());

    CreateSpinner_();
}

void LoadingScene::Update() {
    if (blackSprite_) {
        blackSprite_->Update();
    }

    // ===== Spinner 角度推进（按60fps估算；若你有全局deltaTime请替换）=====
    const float dt = 1.0f / 60.0f;
    spinnerHeadAngle_ += spinnerSpeed_ * dt;

    // 归一化到 [-π, π] 以便计算最小角差
    auto WrapPi = [](float a) {
        while (a >  3.1415926f) a -= 6.2831852f;
        while (a < -3.1415926f) a += 6.2831852f;
        return a;
    };

    // 屏幕右下
    const float pad = 24.0f; // 边距可调
    const float cx = WinApp::kClientWidth  - pad - spinnerRadius_;
    const float cy = WinApp::kClientHeight - pad - spinnerRadius_;

    for (int i = 0; i < static_cast<int>(spinnerDots_.size()); ++i) {
        float baseAngle = (6.2831852f * i) / static_cast<float>(spinnerCount_);
        float px = cx + std::cos(baseAngle) * spinnerRadius_;
        float py = cy + std::sin(baseAngle) * spinnerRadius_;

        // 和“头部”角度的最小角差，用来做尾巴渐隐
        float d = WrapPi(baseAngle - spinnerHeadAngle_);
        d = std::fabs(d);

        // 在 [0, spinnerTrailLen_] 内从 1.0 渐变到 spinnerMinAlpha_；超出即最小透明
        float t = std::clamp(1.0f - (d / spinnerTrailLen_), 0.0f, 1.0f);
        float a = spinnerMinAlpha_ + (1.0f - spinnerMinAlpha_) * t;

        auto& dot = spinnerDots_[i];
        dot->SetPosition({ px - spinnerSize_ * 0.5f, py - spinnerSize_ * 0.5f });
        dot->SetColor({ 1.0f, 1.0f, 1.0f, a });
        dot->Update();
    }
}

void LoadingScene::Draw() {
    dxCommon_->Begin();
    srvManager_->PreDraw();
    spriteCommon_->CommonDraw();

    if (blackSprite_) {
        blackSprite_->Draw();
    }

    for (auto& dot : spinnerDots_) {
        dot->Draw();
    }

    dxCommon_->End();
}

void LoadingScene::Finalize() {
    blackSprite_.reset();
    progressBar_.reset();
    progressBackground_.reset();
    spinnerDots_.clear();
}

void LoadingScene::CreateSpinner_() {
    spinnerDots_.clear();
    spinnerDots_.reserve(spinnerCount_);

    const float pad = 24.0f;
    const float cx = WinApp::kClientWidth  - pad - spinnerRadius_;
    const float cy = WinApp::kClientHeight - pad - spinnerRadius_;

    for (int i = 0; i < spinnerCount_; ++i) {
        auto dot = std::make_unique<Sprite>();
        dot->Initialize(spriteCommon_, spinnerTexPath_.c_str());
        dot->SetSize({ spinnerSize_, spinnerSize_ });
        // 先放到屏幕中心，Update时会被覆盖为圆周位置
        dot->SetPosition({ cx, cy });
        dot->SetColor({ 1.0f, 1.0f, 1.0f, spinnerMinAlpha_ });
        spinnerDots_.push_back(std::move(dot));
    }
}
