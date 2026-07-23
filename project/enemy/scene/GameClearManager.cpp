#include "GameClearManager.h"

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

    Vector4 PickClearPortalColor()
    {
        const int c = rand() % 5;
        switch (c) {
        case 0: return { 0.35f, 0.95f, 1.00f, 1.0f }; // cyan
        case 1: return { 0.48f, 0.72f, 1.00f, 1.0f }; // blue
        case 2: return { 0.80f, 0.50f, 1.00f, 1.0f }; // purple
        case 3: return { 1.00f, 0.60f, 0.86f, 1.0f }; // pink
        default:return { 1.00f, 0.86f, 0.45f, 1.0f }; // gold
        }
    }
}

void GameClearManager::Initialize(SpriteCommon* spriteCommon,
                                  Object3dCommon* object3dCommon,
                                  Camera* camera,
                                  float hpNdcZ) {
    spriteCommon_   = spriteCommon;
    object3dCommon_ = object3dCommon;
    camera_         = camera;
    hpNdcZ_         = hpNdcZ;

    const float W = static_cast<float>(WinApp::kClientWidth);
    const float H = static_cast<float>(WinApp::kClientHeight);

    backdropSprite_ = std::make_unique<Sprite>();
    backdropSprite_->Initialize(spriteCommon_, "Resources/black.png");
    backdropSprite_->SetAnchorPoint({ 0.0f, 0.0f });
    backdropSprite_->SetPosition({ 0.0f, 0.0f });
    backdropSprite_->SetSize({ W, H });
    backdropSprite_->SetColor({ 0.06f, 0.03f, 0.10f, 0.0f });
    backdropSprite_->SetVisible(false);

    portalRingSprite_ = std::make_unique<Sprite>();
    portalRingSprite_->Initialize(spriteCommon_, "Resources/portal_ring.png");
    portalRingSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    portalRingSprite_->SetPosition({ W * 0.5f, H * 0.38f });
    portalRingSprite_->SetSize(ringBaseSize_);
    portalRingSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    portalRingSprite_->SetVisible(false);

    titleSprite_ = std::make_unique<Sprite>();
    titleSprite_->Initialize(spriteCommon_, "Resources/GameClear.png");
    titleSprite_->SetSize(titleSize_);
    titleSprite_->SetVisible(false);

    panelSprite_ = std::make_unique<Sprite>();
    panelSprite_->Initialize(spriteCommon_, "Resources/title_panel.png");
    panelSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    panelSprite_->SetPosition({ W * 0.5f, H * 0.82f });
    panelSprite_->SetSize({ 760.0f, 112.0f });
    panelSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    panelSprite_->SetVisible(false);

    promptSprite_ = std::make_unique<Sprite>();
    promptSprite_->Initialize(spriteCommon_, "Resources/press_space_portal.png");
    promptSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    promptSprite_->SetPosition({ W * 0.5f, H * 0.82f });
    promptSprite_->SetSize({ 520.0f, 130.0f });
    promptSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    promptSprite_->SetVisible(false);

    state_ = State::None;
    t_ = 0.0f;
    ringRotation_ = 0.0f;
    ringPulseT_ = 0.0f;
    uiFloatT_ = 0.0f;
    moteSpawnTimer_ = 0.0f;
    motes_.clear();
}

void GameClearManager::Finalize() {
    motes_.clear();
    promptSprite_.reset();
    panelSprite_.reset();
    titleSprite_.reset();
    portalRingSprite_.reset();
    backdropSprite_.reset();
}

void GameClearManager::Start() {
    if (state_ != State::None && state_ != State::Done) {
        return;
    }

    state_ = State::SlideTitle;
    t_ = 0.0f;
    ringRotation_ = 0.0f;
    ringPulseT_ = 0.0f;
    uiFloatT_ = 0.0f;
    moteSpawnTimer_ = 0.0f;
    motes_.clear();

    const float W = static_cast<float>(WinApp::kClientWidth);
    const float H = static_cast<float>(WinApp::kClientHeight);

    titleEndPos_   = { (W - titleSize_.x) * 0.5f, (H - titleSize_.y) * 0.5f };
    titleStartPos_ = { titleEndPos_.x, -titleSize_.y - 40.0f };
    titlePos_      = titleStartPos_;

    ringCenter_    = { W * 0.5f, H * 0.40f };
    panelBasePos_  = { W * 0.5f, H * 0.82f };
    promptBasePos_ = panelBasePos_;

    if (backdropSprite_) {
        backdropSprite_->SetVisible(true);
        backdropSprite_->SetColor({ 0.06f, 0.03f, 0.10f, 0.35f });
        backdropSprite_->Update();
    }

    if (portalRingSprite_) {
        portalRingSprite_->SetVisible(true);
        portalRingSprite_->SetPosition(ringCenter_);
        portalRingSprite_->SetSize({ 260.0f, 260.0f });
        portalRingSprite_->SetRotation(0.0f);
        portalRingSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
        portalRingSprite_->Update();
    }

    if (titleSprite_) {
        titleSprite_->SetVisible(true);
        titleSprite_->SetPosition(titlePos_);
        titleSprite_->Update();
    }

    if (panelSprite_) {
        panelSprite_->SetVisible(false);
        panelSprite_->SetPosition(panelBasePos_);
        panelSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
        panelSprite_->Update();
    }

    if (promptSprite_) {
        promptSprite_->SetVisible(false);
        promptSprite_->SetPosition(promptBasePos_);
        promptSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
        promptSprite_->Update();
    }

    for (int i = 0; i < 20; ++i) {
        SpawnMote_();
    }
}

void GameClearManager::SpawnMote_()
{
    if (!spriteCommon_) { return; }
    if (motes_.size() >= 36) { return; }

    PortalMote mote{};
    mote.sprite = std::make_unique<Sprite>();
    mote.sprite->Initialize(spriteCommon_, "Resources/portal_mote.png");
    mote.sprite->SetAnchorPoint({ 0.5f, 0.5f });

    // GameOver と同じ波動感
    mote.angle = RandRange(0.0f, kPi * 2.0f);
    mote.radius = RandRange(135.0f, 260.0f);
    mote.angularSpeed = RandRange(-2.0f, 2.0f);
    if (std::fabs(mote.angularSpeed) < 0.65f) {
        mote.angularSpeed = (mote.angularSpeed < 0.0f ? -0.85f : 0.85f);
    }
    mote.radialSpeed = RandRange(-14.0f, 24.0f);
    mote.driftY = RandRange(-18.0f, -2.0f);
    mote.maxLife = RandRange(1.2f, 2.6f);
    mote.life = mote.maxLife;
    mote.baseSize = RandRange(12.0f, 28.0f);
    mote.color = PickClearPortalColor();
    mote.color.w = RandRange(0.55f, 0.95f);

    const float x = ringCenter_.x + std::cos(mote.angle) * mote.radius;
    const float y = ringCenter_.y + std::sin(mote.angle) * mote.radius;
    mote.sprite->SetPosition({ x, y });
    mote.sprite->SetSize({ mote.baseSize, mote.baseSize });
    mote.sprite->SetColor(mote.color);
    mote.sprite->Update();

    motes_.push_back(std::move(mote));
}
void GameClearManager::UpdateMotes_(float dt)
{
    moteSpawnTimer_ -= dt;
    while (moteSpawnTimer_ <= 0.0f) {
        // GameOver と同じ生成テンポ
        const float interval = (state_ == State::PlayerShow) ? 0.12f : 0.06f;
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
        m.angle += m.angularSpeed * dt * 0.75f;
        m.sprite->SetRotation(m.angle * 0.18f);

        // GameOver と同じ揺れ / 点滅
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
void GameClearManager::Update(float dt) {
    if (state_ == State::None || state_ == State::Done) {
        return;
    }

    t_ += dt;

    // 波動速度を GameOver と合わせる
    ringPulseT_ += dt;
    ringRotation_ += dt * 0.45f;
    if (ringRotation_ > kPi * 2.0f) {
        ringRotation_ -= kPi * 2.0f;
    }

    UpdateMotes_(dt);

    switch (state_) {
    case State::SlideTitle: {
        // ===== タイトルは従来の Clear 演出を維持する: スライドインのみで呼吸スケールは行わない =====
        float d = Clamp01_(t_ / titleSlideTime_);
        float e = EaseOutBack_(d);

        titlePos_.x = titleStartPos_.x + (titleEndPos_.x - titleStartPos_.x) * e;
        titlePos_.y = titleStartPos_.y + (titleEndPos_.y - titleStartPos_.y) * e;

        // ===== 背景 / 円環の波動を GameOver と同じテンポに合わせる =====
        const float ringSize = ringBaseSize_.x * (1.0f + 0.04f * std::sin(ringPulseT_ * 3.1f));
        const float backdropAlpha = 0.58f + 0.05f * std::sin(ringPulseT_ * 2.2f);

        if (backdropSprite_) {
            backdropSprite_->SetColor({ 0.06f, 0.03f, 0.10f, backdropAlpha });
            backdropSprite_->Update();
        }

        if (portalRingSprite_) {
            portalRingSprite_->SetPosition(ringCenter_);
            portalRingSprite_->SetSize({ ringSize, ringSize });
            portalRingSprite_->SetRotation(ringRotation_);
            portalRingSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.72f });
            portalRingSprite_->Update();
        }

        if (titleSprite_) {
            titleSprite_->SetPosition(titlePos_);
            titleSprite_->SetSize(titleSize_);   // 元のサイズを維持
            titleSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
            titleSprite_->Update();
        }

        if (promptSprite_) {
            promptSprite_->SetVisible(false);
        }

        if (d >= 1.0f) {
            state_ = State::PlayerShow;
            t_ = 0.0f;
        }
        break;
    }

    case State::PlayerShow: {
        // ===== タイトルは従来どおり、浮遊も拡縮もしない =====
        const float ringPulse = 1.0f + 0.05f * std::sin(ringPulseT_ * 2.5f);
        const float promptAlpha = 0.35f + 0.65f * (std::sin(ringPulseT_ * 4.2f) * 0.5f + 0.5f);
        const float promptFloat = std::sin(ringPulseT_ * 2.1f) * 6.0f;
        const float backdropAlpha = 0.60f + 0.07f * (std::sin(ringPulseT_ * 1.55f) * 0.5f + 0.5f);

        if (backdropSprite_) {
            backdropSprite_->SetColor({ 0.06f, 0.03f, 0.10f, backdropAlpha });
            backdropSprite_->Update();
        }

        if (portalRingSprite_) {
            portalRingSprite_->SetPosition(ringCenter_);
            portalRingSprite_->SetSize({ ringBaseSize_.x * ringPulse, ringBaseSize_.y * ringPulse });
            portalRingSprite_->SetRotation(ringRotation_);
            portalRingSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.78f });
            portalRingSprite_->Update();
        }

        if (titleSprite_) {
            titleSprite_->SetPosition(titleEndPos_);  // 固定し、動かさない
            titleSprite_->SetSize(titleSize_);        // 固定し、拡縮しない
            titleSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
            titleSprite_->Update();
        }

        if (promptSprite_) {
            promptSprite_->SetVisible(true);
            promptSprite_->SetPosition({ promptBasePos_.x, promptBasePos_.y + promptFloat });
            promptSprite_->SetColor({ 1.0f, 1.0f, 1.0f, promptAlpha });
            promptSprite_->Update();
        }

        break;
    }

    default:
        break;
    }
}
void GameClearManager::DrawTitle() {
    if (!spriteCommon_) { return; }
    if (state_ == State::None || state_ == State::Done) { return; }

    if (backdropSprite_ && backdropSprite_->IsVisible()) {
        backdropSprite_->Draw();
    }
    if (portalRingSprite_ && portalRingSprite_->IsVisible()) {
        portalRingSprite_->Draw();
    }
    for (auto& mote : motes_) {
        if (mote.sprite && mote.sprite->IsVisible()) {
            mote.sprite->Draw();
        }
    }
    if (titleSprite_ && titleSprite_->IsVisible()) {
        titleSprite_->Draw();
    }
    /*if (panelSprite_ && panelSprite_->IsVisible()) {
        panelSprite_->Draw();
    }*/
    if (promptSprite_ && promptSprite_->IsVisible()) {
        promptSprite_->Draw();
    }
}

