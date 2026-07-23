#include "FadeManager.h"

#include <algorithm>
#include <cmath>

using namespace MyEngine;
namespace {
    constexpr float kPi = 3.14159265358979323846f;
}

void FadeManager::Initialize(SpriteCommon* spriteCommon)
{
    spriteCommon_ = spriteCommon;

    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize(spriteCommon_, "Resources/black.png");
    sprite_->SetPosition({ 0.0f, 0.0f });
    sprite_->SetSize({
        (float)WinApp::kClientWidth,
        (float)WinApp::kClientHeight
    });
    sprite_->SetVisible(false);
    sprite_->SetColor({ 0.0f, 0.0f, 0.0f, 0.0f });
    sprite_->Update();

    // タイトル画面と同じ portal_ring を使用し、すべての遷移演出に使う
    portalRingSprite_ = std::make_unique<Sprite>();
    portalRingSprite_->Initialize(spriteCommon_, "Resources/portal_ring.png");
    portalRingSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    portalRingSprite_->SetPosition({
        (float)WinApp::kClientWidth * 0.5f,
        (float)WinApp::kClientHeight * 0.5f
    });
    portalRingSprite_->SetSize({ 560.0f, 560.0f });
    portalRingSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    portalRingSprite_->SetVisible(false);
    portalRingSprite_->Update();

    phase_   = FadePhase::None;
    alpha_   = 0.0f;
    speed_   = 0.16f;
    reachedBlack_    = false;
    blackHoldFrames_ = 0;
    overlayPushed_   = false;
    ringRotation_    = 0.0f;
    ringPulseTime_   = 0.0f;
}

void FadeManager::Finalize()
{
    portalRingSprite_.reset();
    sprite_.reset();
}

void FadeManager::StartFadeOut()
{
    phase_         = FadePhase::FadingOut;
    alpha_         = 0.0f;
    reachedBlack_  = false;
    blackHoldFrames_ = 0;
    overlayPushed_ = false;
    ringRotation_  = 0.0f;
    ringPulseTime_ = 0.0f;

    if (sprite_) {
        sprite_->SetVisible(true);
    }
    SyncVisual_();
}

void FadeManager::StartFadeIn()
{
    phase_ = FadePhase::FadingIn;
    alpha_ = 1.0f;
    ringRotation_  = 0.0f;
    ringPulseTime_ = 0.0f;

    if (sprite_) {
        sprite_->SetVisible(true);
    }
    SyncVisual_();
}

void FadeManager::SetBlack()
{
    phase_ = FadePhase::None;
    alpha_ = 1.0f;
    reachedBlack_ = true;
    blackHoldFrames_ = 0;

    if (sprite_) {
        sprite_->SetVisible(true);
    }
    SyncVisual_();
}

void FadeManager::Clear()
{
    phase_ = FadePhase::None;
    alpha_ = 0.0f;
    reachedBlack_ = false;
    blackHoldFrames_ = 0;
    ringRotation_  = 0.0f;
    ringPulseTime_ = 0.0f;

    if (sprite_) {
        sprite_->SetVisible(false);
    }
    SyncVisual_();
}

void FadeManager::Update(float dt)
{
    if (!sprite_) { return; }

    // 遷移中は portal_ring を軽く回転・脈動させる（タイトル画面と同じ素材）
    if (alpha_ > 0.001f) {
        ringRotation_ += 0.95f * dt;
        if (ringRotation_ > 2.0f * kPi) {
            ringRotation_ -= 2.0f * kPi;
        }
        ringPulseTime_ += dt;
    }

    SyncVisual_();
}

void FadeManager::Draw()
{
    if (sprite_ && sprite_->IsVisible()) {
        sprite_->Draw();
    }
    if (portalRingSprite_ && portalRingSprite_->IsVisible()) {
        portalRingSprite_->Draw();
    }
}

void FadeManager::SyncVisual_()
{
    alpha_ = std::clamp(alpha_, 0.0f, 1.0f);

    if (sprite_) {
        sprite_->SetColor({ 0.0f, 0.0f, 0.0f, alpha_ });
        sprite_->Update();
    }

    if (!portalRingSprite_) {
        return;
    }

    if (alpha_ <= 0.001f) {
        portalRingSprite_->SetVisible(false);
        portalRingSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
        portalRingSprite_->Update();
        return;
    }

    const float cx = (float)WinApp::kClientWidth * 0.5f;
    const float cy = (float)WinApp::kClientHeight * 0.5f;

    // alpha が高いほど ring を目立たせる。フェードアウト時は少し拡大し、フェードイン時はやや元に戻す
    const float pulse = 1.0f + 0.035f * std::sin(ringPulseTime_ * 6.0f);
    const float size = (520.0f + 140.0f * alpha_) * pulse;
    const float ringAlpha = std::clamp(0.18f + alpha_ * 0.72f, 0.0f, 0.92f);

    portalRingSprite_->SetVisible(true);
    portalRingSprite_->SetPosition({ cx, cy });
    portalRingSprite_->SetSize({ size, size });
    portalRingSprite_->SetRotation(ringRotation_);
    portalRingSprite_->SetColor({ 1.0f, 1.0f, 1.0f, ringAlpha });
    portalRingSprite_->Update();
}
