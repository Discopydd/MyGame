#include "FadeManager.h"

void FadeManager::Initialize(SpriteCommon* spriteCommon)
{
    spriteCommon_ = spriteCommon;

    sprite_ = new Sprite();
    sprite_->Initialize(spriteCommon_, "Resources/black.png");
    sprite_->SetPosition({ 0.0f, 0.0f });
    sprite_->SetSize({
        (float)WinApp::kClientWidth,
        (float)WinApp::kClientHeight
    });
    sprite_->SetVisible(false);
    sprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });

    phase_   = FadePhase::None;
    alpha_   = 0.0f;
    speed_   = 0.16f;
    reachedBlack_    = false;
    blackHoldFrames_ = 0;
    overlayPushed_   = false;
}

void FadeManager::Finalize()
{
    if (sprite_) {
        delete sprite_;
        sprite_ = nullptr;
    }
}

void FadeManager::StartFadeOut()
{
    phase_         = FadePhase::FadingOut;
    alpha_         = 0.0f;
    reachedBlack_  = false;
    blackHoldFrames_ = 0;
    overlayPushed_ = false;

    if (sprite_) {
        sprite_->SetVisible(true);
        sprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha_ });
    }
}

void FadeManager::StartFadeIn()
{
    phase_ = FadePhase::FadingIn;
    alpha_ = 1.0f;

    if (sprite_) {
        sprite_->SetVisible(true);
        sprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha_ });
    }
}

void FadeManager::SetBlack()
{
    phase_ = FadePhase::None;
    alpha_ = 1.0f;
    reachedBlack_ = true;
    blackHoldFrames_ = 0;

    if (sprite_) {
        sprite_->SetVisible(true);
        sprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha_ });
    }
}

void FadeManager::Clear()
{
    phase_ = FadePhase::None;
    alpha_ = 0.0f;
    reachedBlack_ = false;
    blackHoldFrames_ = 0;

    if (sprite_) {
        sprite_->SetVisible(false);
        sprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    }
}

void FadeManager::Update(float dt)
{
    (void)dt; // 目前我们还是在 GameScene 里控制 alpha/phase，这里只同步到 sprite

    if (!sprite_) { return; }

    sprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha_ });
    sprite_->Update();
}

void FadeManager::Draw()
{
    if (sprite_ && sprite_->IsVisible()) {
        sprite_->Draw();
    }
}
