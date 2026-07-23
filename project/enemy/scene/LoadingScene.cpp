#include "LoadingScene.h"
#include <algorithm>
#include <cmath>

using namespace MyEngine;
void LoadingScene::Initialize() {
    winApp_    = WinApp::GetInstance();
    dxCommon_  = DirectXCommon::GetInstance();
    input_     = Input::GetInstance();
    srvManager_ = SrvManager::GetInstance();

    spriteCommon_ = SpriteCommon::GetInstance();

    auto* tm = TextureManager::GetInstance();
    tm->Initialize(dxCommon_, srvManager_);

    TextureManager::GetInstance()->LoadTexture("Resources/black.png");
    TextureManager::GetInstance()->LoadTexture("Resources/portal_ring.png");

    // 黒幕
    blackSprite_ = std::make_unique<Sprite>();
    blackSprite_->Initialize(spriteCommon_, "Resources/black.png");
    blackSprite_->SetPosition({ 0.0f, 0.0f });
    blackSprite_->SetSize({
        static_cast<float>(WinApp::kClientWidth),
        static_cast<float>(WinApp::kClientHeight)
    });
    blackSprite_->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
    blackSprite_->Update();

    // 遷移用の portal_ring（タイトル画面と同じ素材）
    portalRingSprite_ = std::make_unique<Sprite>();
    portalRingSprite_->Initialize(spriteCommon_, "Resources/portal_ring.png");
    portalRingSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    portalRingSprite_->SetPosition({
        static_cast<float>(WinApp::kClientWidth) * 0.5f,
        static_cast<float>(WinApp::kClientHeight) * 0.5f
    });
    portalRingSprite_->SetSize({ 600.0f, 600.0f });
    portalRingSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.82f });
    portalRingSprite_->Update();
    portalRingRotation_ = 0.0f;

    TextureManager::GetInstance()->LoadTexture(spinnerTexPath_.c_str());

    CreateSpinner_();
}

void LoadingScene::Update() {
    if (blackSprite_) {
        blackSprite_->Update();
    }
    if (portalRingSprite_) {
        const float dtRing = 1.0f / 60.0f;
        portalRingRotation_ += 0.90f * dtRing;
        if (portalRingRotation_ > 6.2831852f) { portalRingRotation_ -= 6.2831852f; }
        const float pulse = 1.0f + 0.035f * std::sin(portalRingRotation_ * 3.0f);
        portalRingSprite_->SetRotation(portalRingRotation_);
        portalRingSprite_->SetSize({ 600.0f * pulse, 600.0f * pulse });
        portalRingSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.82f });
        portalRingSprite_->Update();
    }

    // ===== Spinner の角度更新（60fps 換算。グローバル deltaTime がある場合は置き換える）=====
    const float dt = 1.0f / 60.0f;
    spinnerHeadAngle_ += spinnerSpeed_ * dt;

    // 最小角度差を計算しやすいように [-π, π] に正規化する
    auto WrapPi = [](float a) {
        while (a >  3.1415926f) a -= 6.2831852f;
        while (a < -3.1415926f) a += 6.2831852f;
        return a;
    };

    // 画面右下
    const float pad = 24.0f; // 余白は調整可能
    const float cx = WinApp::kClientWidth  - pad - spinnerRadius_;
    const float cy = WinApp::kClientHeight - pad - spinnerRadius_;

    for (int i = 0; i < static_cast<int>(spinnerDots_.size()); ++i) {
        float baseAngle = (6.2831852f * i) / static_cast<float>(spinnerCount_);
        float px = cx + std::cos(baseAngle) * spinnerRadius_;
        float py = cy + std::sin(baseAngle) * spinnerRadius_;

        // 「ヘッド」角度との差の最小値を求め、テールのフェードに使う
        float d = WrapPi(baseAngle - spinnerHeadAngle_);
        d = std::fabs(d);

        // [0, spinnerTrailLen_] の範囲では 1.0 から spinnerMinAlpha_ まで減衰し、超えた分は最小透明度にする
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
    if (portalRingSprite_) {
        portalRingSprite_->Draw();
    }

    for (auto& dot : spinnerDots_) {
        dot->Draw();
    }

    dxCommon_->End();
}

void LoadingScene::Finalize() {
    blackSprite_.reset();
    portalRingSprite_.reset();
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
        // いったん画面中央に配置し、Update 時に円周上の位置へ上書きする
        dot->SetPosition({ cx, cy });
        dot->SetColor({ 1.0f, 1.0f, 1.0f, spinnerMinAlpha_ });
        spinnerDots_.push_back(std::move(dot));
    }
}
