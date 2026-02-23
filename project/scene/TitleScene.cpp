#include "TitleScene.h"
#include "SceneManager.h"
#include "scene/GameScene.h"
#include "scene/LoadingScene.h"

#include <cmath>
#include <cstdlib>
#include <memory>

namespace {
    constexpr float kPi = 3.14159265358979323846f;

    float RandRangeFloat(float a, float b)
    {
        float t = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return a + (b - a) * t;
    }

    float Clamp01(float t)
    {
        if (t < 0.0f) return 0.0f;
        if (t > 1.0f) return 1.0f;
        return t;
    }

    // Fade-in / hold / fade-out curve based on age [0..1]
    float LifeAlpha01(float age01, float inPortion = 0.18f, float outPortion = 0.18f)
    {
        age01 = Clamp01(age01);
        if (age01 < inPortion) {
            return age01 / inPortion;
        }
        if (age01 > 1.0f - outPortion) {
            return (1.0f - age01) / outPortion;
        }
        return 1.0f;
    }

    Vector4 PickPortalColor()
    {
        // Cyan / Blue / Purple / Pink palette
        const int c = rand() % 5;
        switch (c) {
        case 0: return { 0.35f, 0.95f, 1.00f, 1.0f }; // cyan
        case 1: return { 0.45f, 0.70f, 1.00f, 1.0f }; // blue
        case 2: return { 0.75f, 0.45f, 1.00f, 1.0f }; // purple
        case 3: return { 1.00f, 0.55f, 0.85f, 1.0f }; // pink
        default:return { 0.70f, 1.00f, 0.65f, 1.0f }; // green-ish
        }
    }
}

void TitleScene::Initialize()
{
    winApp_      = WinApp::GetInstance();
    dxCommon_    = DirectXCommon::GetInstance();
    input_       = Input::GetInstance();
    srvManager_  = SrvManager::GetInstance();
    spriteCommon_ = SpriteCommon::GetInstance();

    TextureManager::GetInstance()->Initialize(dxCommon_, srvManager_);

    // Pre-load (avoid first-frame hitch)
    auto* tm = TextureManager::GetInstance();
    tm->LoadTexture("Resources/black.png");
    tm->LoadTexture("Resources/title_bg_portal.png");
    tm->LoadTexture("Resources/portal_ring.png");
    tm->LoadTexture("Resources/title_panel.png");
    tm->LoadTexture("Resources/title_logo_portal_leap.png");
    tm->LoadTexture("Resources/white_sphere.png");
    tm->LoadTexture("Resources/portal_mote.png");
    tm->LoadTexture("Resources/press_space_portal.png");

    const float w = static_cast<float>(WinApp::kClientWidth);
    const float h = static_cast<float>(WinApp::kClientHeight);
    const float cx = w * 0.5f;

    // --- Background ---
    backgroundSprite_ = std::make_unique<Sprite>();
    backgroundSprite_->Initialize(spriteCommon_, "Resources/title_bg_portal.png");
    backgroundSprite_->SetAnchorPoint({ 0.0f, 0.0f });
    backgroundSprite_->SetPosition({ 0.0f, 0.0f });
    backgroundSprite_->SetSize({ w, h });

    // --- Portal ring (behind title) ---
    portalRingSprite_ = std::make_unique<Sprite>();
    portalRingSprite_->Initialize(spriteCommon_, "Resources/portal_ring.png");
    portalRingSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    portalRingSprite_->SetPosition({ cx, titleY_ + 10.0f });
    portalRingSprite_->SetSize({ 560.0f, 560.0f });
    portalRingSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.85f });

    // --- Title panel (UI frame) ---
    titlePanelSprite_ = std::make_unique<Sprite>();
    titlePanelSprite_->Initialize(spriteCommon_, "Resources/title_panel.png");
    titlePanelSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    titlePanelSprite_->SetPosition({ cx, 570.0f });
    titlePanelSprite_->SetSize({ 760.0f, 110.0f });
    titlePanelSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.92f });

    // --- Title logo (ポータル・リープ) ---
    titleLogoSprite_ = std::make_unique<Sprite>();
    titleLogoSprite_->Initialize(spriteCommon_, "Resources/title_logo_portal_leap.png");
    titleLogoSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    titleLogoSprite_->SetPosition({ cx, titleY_ });
    titleLogoSprite_->SetSize({ 860.0f, 215.0f });

    // --- Press Start ---
    startSprite_ = std::make_unique<Sprite>();
    startSprite_->Initialize(spriteCommon_, "Resources/press_space_portal.png");
    startSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    startSprite_->SetPosition({ cx, 570.0f });
    startSprite_->SetSize({ 720.0f, 180.0f });

    // --- Fade overlay ---
    fadeSprite_ = std::make_unique<Sprite>();
    fadeSprite_->Initialize(spriteCommon_, "Resources/black.png");
    fadeSprite_->SetAnchorPoint({ 0.0f, 0.0f });
    fadeSprite_->SetPosition({ 0.0f, 0.0f });
    fadeSprite_->SetSize({ w, h });
    fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    fadeSprite_->SetVisible(false);

    // --- Particle system (sparkles) ---
    particleMgr_ = std::make_unique<ParticleManager>();
    particleMgr_->Initialize(nullptr, spriteCommon_);
    sparkleEmitter_ = particleMgr_->CreateEmitter();
    sparkleEmitter_->SetMaxParticles(220);
    sparkleEmitter_->SetUseOriginalSpriteSize(false);

    // Reset state
    state_ = State::Idle;
    fadeAlpha_ = 0.0f;
    frameCount_ = 0.0f;

    portalMotes_.clear();
    moteSpawnTimer_ = 0.0f;
    uiFloatTimer_ = 0.0f;
    sparkleSpawnTimer_ = 0.0f;

    ringRotation_ = 0.0f;

    titleSettled_ = false;
    overlayPushed_ = false;
    reachedBlack_ = false;
    blackHoldFrames_ = 0;
}

void TitleScene::Update()
{
    constexpr float dt = 1.0f / 60.0f;

    frameCount_ += 1.0f;
    uiFloatTimer_ += dt;

    input_->Update();

    const float w = static_cast<float>(WinApp::kClientWidth);
    const float cx = w * 0.5f;

    // --- Title drop & bounce ---
    if (!titleSettled_) {
        titleVy_ += titleGravity_;
        titleY_ += titleVy_;

        if (titleY_ >= titleTargetY_) {
            titleY_ = titleTargetY_;
            titleVy_ = -titleVy_ * titleBounce_;

            if (std::fabs(titleVy_) < titleStopEps_) {
                titleVy_ = 0.0f;
                titleSettled_ = true;
            }
        }
    }

    // --- Portal ring animation ---
    ringRotation_ += 0.65f * dt;
    if (ringRotation_ > 2.0f * kPi) ringRotation_ -= 2.0f * kPi;

    // Ring center follows the title (so the drop-in feels cohesive)
    const Vector2 ringCenter = { cx, titleY_ + 12.0f };

    if (portalRingSprite_) {
        portalRingSprite_->SetPosition(ringCenter);
        portalRingSprite_->SetRotation(ringRotation_);
    }

    // --- UI gentle float ---
    const float uiFloat = std::sin(uiFloatTimer_ * 1.65f) * 3.5f;

    if (titlePanelSprite_) {
        titlePanelSprite_->SetPosition({ cx, 575.0f + uiFloat });
    }
    if (titleLogoSprite_) {
        titleLogoSprite_->SetPosition({ cx, titleY_ + uiFloat * 0.35f });
    }
    if (startSprite_) {
        startSprite_->SetPosition({ cx, 570.0f + uiFloat * 0.55f });
    }

    // --- Start blink ---
    if (startSprite_) {
        const float blinkSpeed = 0.10f; // per frame
        float s = std::sin(frameCount_ * blinkSpeed) * 0.5f + 0.5f; // 0..1
        float alpha = 0.35f + 0.65f * s;
        startSprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha });
    }

    // --- Spawn & update portal motes (custom particles) ---
    moteSpawnTimer_ -= dt;
    while (moteSpawnTimer_ <= 0.0f && portalMotes_.size() < 48) {
        moteSpawnTimer_ += 0.055f; // ~18 motes/sec until capped

        PortalMote m{};
        m.sprite = std::make_unique<Sprite>();
        m.sprite->Initialize(spriteCommon_, "Resources/portal_mote.png");
        m.sprite->SetAnchorPoint({ 0.5f, 0.5f });

        m.angle = RandRangeFloat(0.0f, 2.0f * kPi);
        m.radius = RandRangeFloat(120.0f, 255.0f);
        m.angularSpeed = RandRangeFloat(-2.4f, 2.4f);
        if (std::fabs(m.angularSpeed) < 0.75f) {
            m.angularSpeed = (m.angularSpeed < 0.0f ? -0.75f : 0.75f);
        }

        m.radialSpeed = RandRangeFloat(-18.0f, 25.0f);
        m.ySpeed = RandRangeFloat(-18.0f, -4.0f);
        m.yOffset = 0.0f;

        m.life = RandRangeFloat(1.6f, 3.2f);
        m.maxLife = m.life;

        m.baseSize = RandRangeFloat(10.0f, 26.0f);

        m.color = PickPortalColor();
        m.color.w = RandRangeFloat(0.55f, 0.9f);

        // Initial placement
        float x = ringCenter.x + std::cos(m.angle) * m.radius;
        float y = ringCenter.y + std::sin(m.angle) * m.radius;
        m.sprite->SetPosition({ x, y });
        m.sprite->SetSize({ m.baseSize, m.baseSize });
        m.sprite->SetColor(m.color);
        m.sprite->Update();

        portalMotes_.push_back(std::move(m));
    }

    for (size_t i = 0; i < portalMotes_.size();) {
        PortalMote& m = portalMotes_[i];

        m.life -= dt;
        if (m.life <= 0.0f) {
            portalMotes_.erase(portalMotes_.begin() + static_cast<long long>(i));
            continue;
        }

        float age = m.maxLife - m.life;
        float age01 = (m.maxLife > 0.0f) ? (age / m.maxLife) : 1.0f;
        float a = LifeAlpha01(age01, 0.18f, 0.18f);

        m.angle += m.angularSpeed * dt;
        m.radius += m.radialSpeed * dt;
        m.yOffset += m.ySpeed * dt;

        // Keep within a reasonable ring band
        if (m.radius < 95.0f)  m.radius = 95.0f;
        if (m.radius > 285.0f) m.radius = 285.0f;

        float pulse = 1.0f + 0.14f * std::sin(age * 8.0f + m.angle * 0.7f);
        float size = m.baseSize * pulse;

        float x = ringCenter.x + std::cos(m.angle) * m.radius;
        float y = ringCenter.y + std::sin(m.angle) * m.radius + m.yOffset;

        if (m.sprite) {
            m.sprite->SetPosition({ x, y });
            m.sprite->SetSize({ size, size });
            m.sprite->SetRotation(m.angle * 0.10f);

            Vector4 c = m.color;
            c.w = m.color.w * a;
            m.sprite->SetColor(c);
            m.sprite->Update();
        }

        ++i;
    }

    // --- Sparkle particles (using the existing ParticleEmitter) ---
    if (sparkleEmitter_) {
        sparkleSpawnTimer_ -= dt;
        while (sparkleSpawnTimer_ <= 0.0f) {
            sparkleSpawnTimer_ += 0.035f;

            Vector3 spawnPos = {
                ringCenter.x + RandRangeFloat(-90.0f, 90.0f),
                ringCenter.y + RandRangeFloat(-45.0f, 65.0f),
                0.0f
            };

            sparkleEmitter_->Emit(
                1,
                ParticleType::Sprite2D,
                "Resources/white_sphere.png",
                spawnPos,
                50.0f, 170.0f,   // speed (px/s)
                0.70f, 1.35f     // life (s)
            );
        }
    }

    if (particleMgr_) {
        particleMgr_->Update(dt);
    }

    // --- State machine ---
    switch (state_) {
    case State::Idle:
        if (input_->TriggerKey(DIK_SPACE)) {
            state_ = State::FadingOut;
            fadeAlpha_ = 0.0f;
            if (fadeSprite_) {
                fadeSprite_->SetVisible(true);
            }
        }
        break;

    case State::FadingOut:
        fadeAlpha_ += 0.04f; // per-frame, matches existing feel
        if (fadeAlpha_ >= 1.0f) {
            fadeAlpha_ = 1.0f;

            if (!reachedBlack_) {
                reachedBlack_ = true;
                blackHoldFrames_ = 1;
                break;
            }
            if (blackHoldFrames_ > 0) {
                --blackHoldFrames_;
                break;
            }
            if (!overlayPushed_) {
                sceneManager_->SetOverlayScene(
                    std::unique_ptr<BaseScene>(std::make_unique<LoadingScene>().release())
                );
                overlayPushed_ = true;
            }
            sceneManager_->SetNextScene(
                std::unique_ptr<BaseScene>(std::make_unique<GameScene>().release())
            );
            return;
        }
        break;
    }

    // --- Update sprites ---
    if (backgroundSprite_) backgroundSprite_->Update();
    if (portalRingSprite_) portalRingSprite_->Update();
    if (titlePanelSprite_) titlePanelSprite_->Update();
    if (titleLogoSprite_)  titleLogoSprite_->Update();
    if (startSprite_)      startSprite_->Update();

    if (fadeSprite_) {
        fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, fadeAlpha_ });
        fadeSprite_->Update();
    }
}

void TitleScene::Draw()
{
    dxCommon_->Begin();
    srvManager_->PreDraw();
    spriteCommon_->CommonDraw();

    if (backgroundSprite_) {
        backgroundSprite_->Draw();
    }

    // --- Portal layer ---
    if (portalRingSprite_) {
        portalRingSprite_->Draw();
    }

    for (auto& m : portalMotes_) {
        if (m.sprite) {
            m.sprite->Draw();
        }
    }

    if (particleMgr_) {
        particleMgr_->Draw2D();
    }

    // --- Title UI layer ---
    if (titlePanelSprite_) {
        titlePanelSprite_->Draw();
    }
    if (titleLogoSprite_) {
        titleLogoSprite_->Draw();
    }
    if (startSprite_) {
        startSprite_->Draw();
    }

    // --- Fade overlay ---
    if (fadeSprite_) {
        fadeSprite_->Draw();
    }

    dxCommon_->End();
}

void TitleScene::Finalize()
{
    if (particleMgr_) {
        particleMgr_->Finalize();
        particleMgr_.reset();
    }
    sparkleEmitter_ = nullptr;

    portalMotes_.clear();

    backgroundSprite_.reset();
    portalRingSprite_.reset();
    titlePanelSprite_.reset();
    titleLogoSprite_.reset();
    startSprite_.reset();
    fadeSprite_.reset();

    TextureManager::GetInstance()->Finalize();
}
