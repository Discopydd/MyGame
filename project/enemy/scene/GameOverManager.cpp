#include "GameOverManager.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

using namespace MyEngine;
namespace {
    constexpr float kPi = 3.14159265358979323846f;

    float RandRange(float a, float b)
    {
        float t = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return a + (b - a) * t;
    }

    Vector4 PickGameOverPortalColor()
    {
        const int c = rand() % 5;
        switch (c) {
        case 0: return { 1.00f, 0.38f, 0.50f, 1.0f }; // red pink
        case 1: return { 0.92f, 0.35f, 0.98f, 1.0f }; // magenta
        case 2: return { 0.38f, 0.86f, 1.00f, 1.0f }; // cyan
        case 3: return { 1.00f, 0.78f, 0.35f, 1.0f }; // amber
        default:return { 0.62f, 0.50f, 1.00f, 1.0f }; // purple
        }
    }
}

void GameOverManager::Initialize(SpriteCommon* spriteCommon)
{
    spriteCommon_ = spriteCommon;

    const float W = static_cast<float>(WinApp::kClientWidth);
    const float H = static_cast<float>(WinApp::kClientHeight);

    backdropSprite_ = std::make_unique<Sprite>();
    backdropSprite_->Initialize(spriteCommon_, "Resources/black.png");
    backdropSprite_->SetAnchorPoint({ 0.0f, 0.0f });
    backdropSprite_->SetPosition({ 0.0f, 0.0f });
    backdropSprite_->SetSize({ W, H });
    backdropSprite_->SetColor({ 0.03f, 0.02f, 0.05f, 0.0f });
    backdropSprite_->SetVisible(false);

    portalRingSprite_ = std::make_unique<Sprite>();
    portalRingSprite_->Initialize(spriteCommon_, "Resources/portal_ring.png");
    portalRingSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    portalRingSprite_->SetPosition({ W * 0.5f, H * 0.38f });
    portalRingSprite_->SetSize({ 220.0f, 220.0f });
    portalRingSprite_->SetColor({ 1.0f, 0.55f, 0.80f, 0.0f });
    portalRingSprite_->SetVisible(false);

    titleSprite_ = std::make_unique<Sprite>();
    titleSprite_->Initialize(spriteCommon_, "Resources/GameOver.png");
    titleSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    titleSprite_->SetSize(titleSize_);
    titleSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    titleSprite_->SetVisible(false);

    promptSprite_ = std::make_unique<Sprite>();
    promptSprite_->Initialize(spriteCommon_, "Resources/press_space_portal.png");
    promptSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    promptSprite_->SetPosition({ W * 0.5f, H * 0.80f });
    promptSprite_->SetSize({ 500.0f, 125.0f });
    promptSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    promptSprite_->SetVisible(false);

    state_ = State::None;
    t_ = 0.0f;
    ringRotation_ = 0.0f;
    ringPulseT_ = 0.0f;
    backdropAlpha_ = 0.0f;
    moteSpawnTimer_ = 0.0f;
    motes_.clear();
    showTexts_ = true;
    drawEnabled_ = true;
}

void GameOverManager::Finalize()
{
    motes_.clear();
    promptSprite_.reset();
    titleSprite_.reset();
    portalRingSprite_.reset();
    backdropSprite_.reset();
}

void GameOverManager::Start()
{
    if (state_ != State::None && state_ != State::Done) {
        return;
    }

    state_ = State::PortalOpen;
    t_     = 0.0f;
    ringRotation_ = 0.0f;
    ringPulseT_ = 0.0f;
    backdropAlpha_ = 0.0f;
    moteSpawnTimer_ = 0.0f;
    motes_.clear();
    showTexts_ = true;
    drawEnabled_ = true;

    const float W = static_cast<float>(WinApp::kClientWidth);
    const float H = static_cast<float>(WinApp::kClientHeight);

    ringCenter_ = { W * 0.5f, H * 0.38f };
    titleEndPos_   = { W * 0.5f, H * 0.38f };
    titleStartPos_ = { W * 0.5f, -titleSize_.y * 0.5f - 50.0f };
    titleCenter_   = titleStartPos_;
    promptBasePos_ = { W * 0.5f, H * 0.80f };

    if (backdropSprite_) {
        backdropSprite_->SetVisible(true);
        backdropSprite_->SetColor({ 0.03f, 0.02f, 0.05f, 0.0f });
        backdropSprite_->Update();
    }

    if (portalRingSprite_) {
        portalRingSprite_->SetVisible(true);
        portalRingSprite_->SetPosition(ringCenter_);
        portalRingSprite_->SetSize({ 200.0f, 200.0f });
        portalRingSprite_->SetRotation(0.0f);
        portalRingSprite_->SetColor({ 1.0f, 0.55f, 0.80f, 0.0f });
        portalRingSprite_->Update();
    }

    if (titleSprite_) {
        titleSprite_->SetVisible(false);
        titleSprite_->SetPosition(titleCenter_);
        titleSprite_->SetSize(titleSize_);
        titleSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
        titleSprite_->Update();
    }

    if (promptSprite_) {
        promptSprite_->SetVisible(false);
        promptSprite_->SetPosition(promptBasePos_);
        promptSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
        promptSprite_->Update();
    }

    for (int i = 0; i < 18; ++i) {
        SpawnMote_();
    }
}

void GameOverManager::SpawnMote_()
{
    if (!spriteCommon_) { return; }
    if (motes_.size() >= 36) { return; }

    PortalMote mote{};
    mote.sprite = std::make_unique<Sprite>();
    mote.sprite->Initialize(spriteCommon_, "Resources/portal_mote.png");
    mote.sprite->SetAnchorPoint({ 0.5f, 0.5f });

    mote.angle = RandRange(0.0f, kPi * 2.0f);
    mote.radius = RandRange(135.0f, 260.0f);
    mote.angularSpeed = RandRange(-2.8f, 2.8f);
    if (std::fabs(mote.angularSpeed) < 0.95f) {
        mote.angularSpeed = (mote.angularSpeed < 0.0f ? -1.15f : 1.15f);
    }
    mote.radialSpeed = RandRange(-14.0f, 24.0f);
    mote.driftY = RandRange(-18.0f, -2.0f);
    mote.maxLife = RandRange(1.2f, 2.6f);
    mote.life = mote.maxLife;
    mote.baseSize = RandRange(12.0f, 28.0f);
    mote.color = PickGameOverPortalColor();
    mote.color.w = RandRange(0.55f, 0.95f);

    const float x = ringCenter_.x + std::cos(mote.angle) * mote.radius;
    const float y = ringCenter_.y + std::sin(mote.angle) * mote.radius;
    mote.sprite->SetPosition({ x, y });
    mote.sprite->SetSize({ mote.baseSize, mote.baseSize });
    mote.sprite->SetColor(mote.color);
    mote.sprite->Update();

    motes_.push_back(std::move(mote));
}

void GameOverManager::UpdateMotes_(float dt)
{
    moteSpawnTimer_ -= dt;
    while (moteSpawnTimer_ <= 0.0f) {
        const float interval = (state_ == State::Wait) ? 0.12f : 0.06f;
        moteSpawnTimer_ += interval;
        SpawnMote_();
    }

    for (size_t i = 0; i < motes_.size();) {
        PortalMote& m = motes_[i];
        m.life -= dt;
        if (m.life <= 0.0f) {
            motes_.erase(motes_.begin() + i);
            continue;
        }

        const float age01 = 1.0f - Clamp01_(m.life / m.maxLife);
        m.angle += m.angularSpeed * dt;
        m.radius += m.radialSpeed * dt;

        const float shrink = 1.0f - age01 * 0.45f;
        const float flicker = 0.85f + 0.15f * std::sin(ringPulseT_ * 9.0f + m.angle * 2.0f);
        const float x = ringCenter_.x + std::cos(m.angle) * m.radius;
        const float y = ringCenter_.y + std::sin(m.angle) * m.radius + m.driftY * age01;
        const float s = m.baseSize * shrink;
        const float alphaIn = (age01 < 0.18f) ? (age01 / 0.18f) : 1.0f;
        const float alphaOut = (age01 > 0.72f) ? ((1.0f - age01) / 0.28f) : 1.0f;
        const float alpha = (std::max)(0.0f, alphaIn * alphaOut) * m.color.w * flicker;

        m.sprite->SetPosition({ x, y });
        m.sprite->SetSize({ s, s });
        m.sprite->SetRotation(m.angle * 0.25f);
        m.sprite->SetColor({ m.color.x, m.color.y, m.color.z, alpha });
        m.sprite->Update();

        ++i;
    }
}

void GameOverManager::Update(float dt)
{
    if (state_ == State::None || state_ == State::Done) {
        return;
    }

    t_ += dt;
    ringPulseT_ += dt;
    ringRotation_ += dt * 0.95f;
    if (ringRotation_ > kPi * 2.0f) {
        ringRotation_ -= kPi * 2.0f;
    }

    UpdateMotes_(dt);

    if (state_ == State::PortalOpen) {
        const float duration = 0.48f;
        const float d = Clamp01_(t_ / duration);
        const float e = EaseOutCubic_(d);

        backdropAlpha_ = 0.18f + 0.45f * e;

        const float size = 200.0f + (ringBaseSize_.x - 200.0f) * e;
        const float alpha = 0.78f * e;
        const float rot = -0.8f + ringRotation_;

        if (backdropSprite_) {
            backdropSprite_->SetColor({ 0.03f, 0.02f, 0.05f, backdropAlpha_ });
            backdropSprite_->Update();
        }
        if (portalRingSprite_) {
            portalRingSprite_->SetPosition(ringCenter_);
            portalRingSprite_->SetSize({ size, size });
            portalRingSprite_->SetRotation(rot);
            portalRingSprite_->SetColor({ 1.0f, 0.55f, 0.80f, alpha });
            portalRingSprite_->Update();
        }

        if (d >= 1.0f) {
            state_ = State::SlideTitle;
            t_ = 0.0f;
            if (titleSprite_) {
                titleSprite_->SetVisible(true);
            }
        }
    }
    else if (state_ == State::SlideTitle) {
        const float d = Clamp01_(t_ / titleSlideTime_);
        const float e = EaseOutBack_(d);

        titleCenter_.x = titleStartPos_.x + (titleEndPos_.x - titleStartPos_.x) * e;
        titleCenter_.y = titleStartPos_.y + (titleEndPos_.y - titleStartPos_.y) * e;

        const float titleScale = 1.0f + 0.06f * (1.0f - d);
        const float pulse = 1.0f + 0.03f * std::sin(ringPulseT_ * 6.0f);
        const float ringSize = ringBaseSize_.x * (1.0f + 0.04f * std::sin(ringPulseT_ * 3.1f));

        backdropAlpha_ = 0.58f + 0.05f * std::sin(ringPulseT_ * 2.2f);

        if (backdropSprite_) {
            backdropSprite_->SetColor({ 0.03f, 0.02f, 0.05f, backdropAlpha_ });
            backdropSprite_->Update();
        }
        if (portalRingSprite_) {
            portalRingSprite_->SetPosition(ringCenter_);
            portalRingSprite_->SetSize({ ringSize, ringSize });
            portalRingSprite_->SetRotation(ringRotation_);
            portalRingSprite_->SetColor({ 0.98f, 0.48f, 0.76f, 0.72f });
            portalRingSprite_->Update();
        }
        if (titleSprite_) {
            titleSprite_->SetPosition(titleCenter_);
            titleSprite_->SetSize({ titleSize_.x * titleScale * pulse, titleSize_.y * titleScale * pulse });
            titleSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.30f + 0.70f * d });
            titleSprite_->Update();
        }

        if (d >= 1.0f) {
            state_ = State::Wait;
            t_ = 0.0f;
            if (promptSprite_) {
                promptSprite_->SetVisible(true);
            }
        }
    }
    else if (state_ == State::Wait) {
        const float titleFloat = std::sin(ringPulseT_ * 1.8f) * 5.0f;
        const float titleScale = 1.0f + 0.02f * std::sin(ringPulseT_ * 3.0f);
        const float ringPulse = 1.0f + 0.05f * std::sin(ringPulseT_ * 2.5f);
        const float promptAlpha = 0.35f + 0.65f * (std::sin(ringPulseT_ * 4.2f) * 0.5f + 0.5f);
        const float promptFloat = std::sin(ringPulseT_ * 2.1f) * 6.0f;

        backdropAlpha_ = 0.60f + 0.07f * (std::sin(ringPulseT_ * 1.55f) * 0.5f + 0.5f);

        if (backdropSprite_) {
            backdropSprite_->SetColor({ 0.03f, 0.02f, 0.05f, backdropAlpha_ });
            backdropSprite_->Update();
        }
        if (portalRingSprite_) {
            portalRingSprite_->SetPosition(ringCenter_);
            portalRingSprite_->SetSize({ ringBaseSize_.x * ringPulse, ringBaseSize_.y * ringPulse });
            portalRingSprite_->SetRotation(ringRotation_);
            portalRingSprite_->SetColor({ 0.98f, 0.46f, 0.80f, 0.78f });
            portalRingSprite_->Update();
        }
        if (titleSprite_) {
            titleSprite_->SetPosition({ titleEndPos_.x, titleEndPos_.y + titleFloat });
            titleSprite_->SetSize({ titleSize_.x * titleScale, titleSize_.y * titleScale });
            titleSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
            titleSprite_->Update();
        }
        if (promptSprite_) {
            promptSprite_->SetPosition({ promptBasePos_.x, promptBasePos_.y + promptFloat });
            promptSprite_->SetColor({ 1.0f, 1.0f, 1.0f, promptAlpha });
            promptSprite_->Update();
        }
    }
}

void GameOverManager::Draw()
{
    if (!spriteCommon_) { return; }
    if (state_ == State::None || state_ == State::Done) { return; }
    if (!drawEnabled_) { return; }

    if (backdropSprite_ && backdropSprite_->IsVisible()) {
        backdropSprite_->Draw();
    }
    for (auto& mote : motes_) {
        if (mote.sprite && mote.sprite->IsVisible()) {
            mote.sprite->Draw();
        }
    }
    if (portalRingSprite_ && portalRingSprite_->IsVisible()) {
        portalRingSprite_->Draw();
    }
    if (showTexts_) {
        if (titleSprite_ && titleSprite_->IsVisible()) {
            titleSprite_->Draw();
        }
        if (promptSprite_ && promptSprite_->IsVisible()) {
            promptSprite_->Draw();
        }
    }
}
