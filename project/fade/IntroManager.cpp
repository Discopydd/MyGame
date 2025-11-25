#include "IntroManager.h"
#include <algorithm>
#include <cmath>

void IntroManager::Initialize(SpriteCommon* spriteCommon, Input* input)
{
    spriteCommon_ = spriteCommon;
    input_        = input;

    const float W = (float)WinApp::kClientWidth;
    const float H = (float)WinApp::kClientHeight;

    // 电影黑边
    letterboxTop_ = new Sprite();
    letterboxTop_->Initialize(spriteCommon_, "Resources/black.png");
    letterboxTop_->SetSize({ W, H * 0.14f });
    letterboxTop_->SetPosition({ 0.0f, -H * 0.14f });
    letterboxTop_->SetColor({ 1,1,1,0 });

    letterboxBottom_ = new Sprite();
    letterboxBottom_->Initialize(spriteCommon_, "Resources/black.png");
    letterboxBottom_->SetSize({ W, H * 0.14f });
    letterboxBottom_->SetPosition({ 0.0f, H });
    letterboxBottom_->SetColor({ 1,1,1,0 });

    // 暗角
    vignette_ = new Sprite();
    vignette_->Initialize(spriteCommon_, "Resources/gray.png");
    vignette_->SetSize({ W, H });
    vignette_->SetPosition({ 0.0f, 0.0f });
    vignette_->SetColor({ 1,1,1,0.0f });

    // 标题
    introTitle_ = new Sprite();
    introTitle_->Initialize(spriteCommon_, "Resources/GameTitle.dds");
    introTitle_->SetPosition({ 0.0f, H * 0.15f });
    introTitle_->SetColor({ 1,1,1,0 });

    // Skip 提示
    skipHint_ = new Sprite();
    skipHint_->Initialize(spriteCommon_, "Resources/Start.dds");
    skipHint_->SetPosition({ W * 0.5f - 100.0f, H * 0.8f });
    skipHint_->SetColor({ 1,1,1,0 });
    skipHint_->SetVisible(false);

    state_    = State::None;
    t_        = 0.0f;
    started_  = false;
    skippable_ = true;
}

void IntroManager::Finalize()
{
    if (letterboxTop_)    { delete letterboxTop_;    letterboxTop_ = nullptr; }
    if (letterboxBottom_) { delete letterboxBottom_; letterboxBottom_ = nullptr; }
    if (vignette_)        { delete vignette_;        vignette_ = nullptr; }
    if (introTitle_)      { delete introTitle_;      introTitle_ = nullptr; }
    if (skipHint_)        { delete skipHint_;        skipHint_ = nullptr; }
}

void IntroManager::Start(const Vector3& playerPos)
{
    started_   = true;
    state_     = State::BarsIn;
    t_         = 0.0f;
    skippable_ = true;

    camPivot_     = playerPos;
    camOrbitDeg_  = -25.0f;
    shakeTime_    = 0.0f;
    shakeAmp_     = 0.0f;

    if (letterboxTop_)    letterboxTop_->SetColor({1,1,1,1});
    if (letterboxBottom_) letterboxBottom_->SetColor({1,1,1,1});
    if (vignette_)        vignette_->SetColor({1,1,1,0});
    if (introTitle_)      introTitle_->SetColor({1,1,1,0});
    if (skipHint_)        { skipHint_->SetColor({1,1,1,0}); skipHint_->SetVisible(false); }
}

void IntroManager::Update(float dt)
{
    if (state_ == State::None || state_ == State::Done) {
        return;
    }

    // 跳过（只在 BarsIn / OrbitZoom / TitleShow 这些阶段有效）
    if (skippable_ && input_) {
        if (input_->TriggerKey(DIK_SPACE) ||
            input_->TriggerKey(DIK_RETURN) ||
            input_->TriggerKey(DIK_E)) {
            skippable_ = false;
            state_     = State::BarsOut;
            t_         = 0.0f;
            shakeTime_ = 0.0f;
            shakeAmp_  = 0.0f;
            return;
        }
    }

    t_ += dt;

    switch (state_) {
    case State::BarsIn: {
        float d = (std::min)(t_ / 0.175f, 1.0f);
        d = EaseOutCubic_(d);

        const float H    = (float)WinApp::kClientHeight;
        const float barH = H * 0.25f;

        float topStart = -barH;
        float topEnd   = 0.0f;
        float botStart = H;
        float botEnd   = H - barH;

        float topY = topStart + (topEnd - topStart) * d;
        float botY = botStart + (botEnd - botStart) * d;

        if (letterboxTop_) {
            letterboxTop_->SetPosition({ 0.0f, topY });
            letterboxTop_->SetSize({ (float)WinApp::kClientWidth,  barH });
        }
        if (letterboxBottom_) {
            letterboxBottom_->SetPosition({ 0.0f, botY });
            letterboxBottom_->SetSize({ (float)WinApp::kClientWidth,  barH });
        }

        if (vignette_) vignette_->SetColor({ 1,1,1, 0.25f * d });

        if (t_ >= 0.35f) {
            state_ = State::OrbitZoom;
            t_ = 0.0f;
        }
        break;
    }

    case State::OrbitZoom: {
        float d = (std::min)(t_ / 0.5f, 1.0f);
        float e = EaseInOutSine_(d);
        (void)e; // 目前没有直接用到 e，你以后加镜头动可以用

        // 标题淡入
        if (introTitle_) {
            float titleAlpha = std::clamp((d - 0.35f) / 0.35f, 0.0f, 1.0f);
            introTitle_->SetColor({ 1,1,1, titleAlpha });
        }

        // Skip 提示闪烁
        if (skipHint_) {
            skipHint_->SetVisible(true);
            float blink = (std::sinf(t_ * 12.0f) * 0.5f + 0.5f) * 0.85f;
            skipHint_->SetColor({ 1,1,1, blink });
        }

        if (t_ >= 0.8f) {
            state_ = State::TitleShow;
            t_ = 0.0f;
        }
        break;
    }

    case State::TitleShow: {
        float d = (std::min)(t_ / 0.25f, 1.0f);
        if (vignette_) vignette_->SetColor({1,1,1, 0.25f + 0.10f * d});

        // 轻微震动（数值照搬原来的）
        if (t_ < 0.2f) {
            shakeTime_ = 0.2f;
            shakeAmp_  = 0.12f;
        }

        if (t_ >= 0.25f) {
            state_ = State::BarsOut;
            t_ = 0.0f;
        }
        break;
    }

    case State::BarsOut: {
        float d = (std::min)(t_ / 0.15f, 1.0f);
        d = EaseOutCubic_(d);

        const float H    = (float)WinApp::kClientHeight;
        const float barH = H * 0.25f;

        float topStart = 0.0f;
        float topEnd   = -barH;
        float botStart = H - barH;
        float botEnd   = H;

        float topY = topStart + (topEnd - topStart) * d;
        float botY = botStart + (botEnd - botStart) * d;

        if (letterboxTop_) {
            letterboxTop_->SetPosition({ 0.0f, topY });
            letterboxTop_->SetSize({ (float)WinApp::kClientWidth,  barH });
        }
        if (letterboxBottom_) {
            letterboxBottom_->SetPosition({ 0.0f, botY });
            letterboxBottom_->SetSize({ (float)WinApp::kClientWidth,  barH });
        }

        if (vignette_)   vignette_->SetColor({ 1,1,1, (1.0f - d) * 0.35f });
        if (introTitle_) introTitle_->SetColor({ 1,1,1, (1.0f - d) });
        if (skipHint_)   skipHint_->SetColor({ 1,1,1, (1.0f - d) });

        if (t_ >= 0.5f) {
            state_ = State::Done;
            t_ = 0.0f;
        }
        break;
    }

    default:
        break;
    }
    if (letterboxTop_)    letterboxTop_->Update();
    if (letterboxBottom_) letterboxBottom_->Update();
    if (vignette_)        vignette_->Update();
    if (introTitle_)      introTitle_->Update();
    // （如果以后想要镜头摇晃，可以在这里根据 shakeTime_/shakeAmp_ 返回一个偏移量）
}

void IntroManager::Draw()
{
    if (!spriteCommon_) return;
    if (state_ == State::None || state_ == State::Done) return;

    if (vignette_)        vignette_->Draw();
    if (introTitle_)      introTitle_->Draw();
    if (letterboxTop_)    letterboxTop_->Draw();
    if (letterboxBottom_) letterboxBottom_->Draw();
    // 如果想显示 Skip 提示，解开下面的注释
    // if (skipHint_ && skipHint_->IsVisible()) skipHint_->Draw();
}
