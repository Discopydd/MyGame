#include "LoadingScene.h"
#include <algorithm>

void LoadingScene::Initialize() {
    winApp_ = WinApp::GetInstance();
    dxCommon_ = DirectXCommon::GetInstance();
    input_ = Input::GetInstance();
    srvManager_ = SrvManager::GetInstance();

    spriteCommon_ = new SpriteCommon();
    spriteCommon_->Initialize(dxCommon_);

    auto* tm = TextureManager::GetInstance();
    tm->Initialize(dxCommon_, srvManager_);
    TextureManager::GetInstance()->LoadTexture("Resources/loading.png");

    loadingSprite_ = new Sprite();
    loadingSprite_->Initialize(spriteCommon_, "Resources/loading.png");
    loadingSprite_->SetPosition({ 0,0 });
    loadingSprite_->SetSize({ (float)WinApp::kClientWidth, (float)WinApp::kClientHeight });

    TextureManager::GetInstance()->LoadTexture("Resources/black.png");

    // 黑幕
    blackSprite_ = new Sprite();
    blackSprite_->Initialize(spriteCommon_, "Resources/black.png");
    blackSprite_->SetPosition({ 0,0 });
    blackSprite_->SetSize({ (float)WinApp::kClientWidth, (float)WinApp::kClientHeight });

    TextureManager::GetInstance()->LoadTexture(spinnerTexPath_.c_str());

    CreateSpinner_();;
}

void LoadingScene::Update() {
    if (showDelayFrames_ > 0) {
        --showDelayFrames_;  // 递减延迟计数
    }

    blackSprite_->Update();
    loadingSprite_->Update();
     // ===== Spinner 角度推进（按60fps估算；若你有全局deltaTime请替换）=====
    const float dt = 1.0f / 60.0f;
    spinnerHeadAngle_ += spinnerSpeed_ * dt;
    // 归一化到 [-π, π] 以便计算最小角差
    auto WrapPi = [](float a) {
        while (a >  3.1415926f) a -= 6.2831852f;
        while (a < -3.1415926f) a += 6.2831852f;
        return a;
    };

    // 屏幕中心
    const float cx = WinApp::kClientWidth  * 0.5f;
    const float cy = WinApp::kClientHeight * 0.5f;

    for (int i = 0; i < (int)spinnerDots_.size(); ++i) {
        float baseAngle = (6.2831852f * i) / (float)spinnerCount_;
        float px = cx + std::cos(baseAngle) * spinnerRadius_;
        float py = cy + std::sin(baseAngle) * spinnerRadius_;

        // 和“头部”角度的最小角差，用来做尾巴渐隐
        float d = WrapPi(baseAngle - spinnerHeadAngle_);
        d = std::fabs(d);

        // 在 [0, spinnerTrailLen_] 内从 1.0 渐变到 spinnerMinAlpha_；超出即最小透明
        float t = std::clamp(1.0f - (d / spinnerTrailLen_), 0.0f, 1.0f);
        float a = spinnerMinAlpha_ + (1.0f - spinnerMinAlpha_) * t;

        spinnerDots_[i]->SetPosition({ px - spinnerSize_*0.5f, py - spinnerSize_*0.5f });
        spinnerDots_[i]->SetColor({ 1.0f, 1.0f, 1.0f, a });
        spinnerDots_[i]->Update();
    }
}

void LoadingScene::Draw() {
    dxCommon_->Begin();
    srvManager_->PreDraw();
    spriteCommon_->CommonDraw();
    if (blackSprite_) {
        blackSprite_->Draw();
    }
    if (showDelayFrames_ <= 0) {
        for (auto* dot : spinnerDots_) dot->Draw();
    }

    dxCommon_->End();
}

void LoadingScene::Finalize() {
    delete loadingSprite_;
    delete blackSprite_;
    delete spriteCommon_;
    for (auto* s : spinnerDots_) delete s;
    spinnerDots_.clear();
}

void LoadingScene::CreateSpinner_() {
    spinnerDots_.reserve(spinnerCount_);
    for (int i = 0; i < spinnerCount_; ++i) {
        auto* s = new Sprite();
        s->Initialize(spriteCommon_, spinnerTexPath_.c_str());
        s->SetSize({ spinnerSize_, spinnerSize_ });
        // 先放到屏幕中心，Update时会被覆盖为圆周位置
        s->SetPosition({ WinApp::kClientWidth * 0.5f, WinApp::kClientHeight * 0.5f });
        s->SetColor({ 1, 1, 1, spinnerMinAlpha_ });
        spinnerDots_.push_back(s);
    }
}
