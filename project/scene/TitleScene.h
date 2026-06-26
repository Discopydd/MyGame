#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "MyMath.h"
#include "ImGuiManager.h"
#include "SoundManager.h"
#include "Camera.h"

#include <vector>
#include <memory>

#include "BaseScene.h"
#include "../particle/ParticleManager.h"

class TitleScene : public MyEngine::BaseScene {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    enum class State {
        Idle,
        FadingOut,
    };

    // Title portal motes (custom 2D particles for the title screen)
    struct PortalMote {
        std::unique_ptr<MyEngine::Sprite> sprite;
        float angle = 0.0f;         // rad
        float radius = 0.0f;        // px
        float angularSpeed = 0.0f;  // rad/s
        float radialSpeed = 0.0f;   // px/s
        float ySpeed = 0.0f;        // px/s (negative = up)
        float yOffset = 0.0f;       // px
        float life = 0.0f;          // s
        float maxLife = 0.0f;       // s
        float baseSize = 16.0f;     // px
        MyEngine::Vector4 color = { 1,1,1,1 };
    };

    // Non-owning pointers
    MyEngine::WinApp*        winApp_       = nullptr;
    MyEngine::DirectXCommon* dxCommon_     = nullptr;
    MyEngine::Input*         input_        = nullptr;
    MyEngine::SrvManager*    srvManager_   = nullptr;
    MyEngine::SpriteCommon*  spriteCommon_ = nullptr;

    // Sprites (owned)
    std::unique_ptr<MyEngine::Sprite> backgroundSprite_;
    std::unique_ptr<MyEngine::Sprite> portalRingSprite_;
    std::unique_ptr<MyEngine::Sprite> titlePanelSprite_;
    std::unique_ptr<MyEngine::Sprite> titleLogoSprite_;
    std::unique_ptr<MyEngine::Sprite> startSprite_;
    std::unique_ptr<MyEngine::Sprite> fadeSprite_;
    std::unique_ptr<MyEngine::Sprite> transitionRingSprite_;

    // Title screen particles
    std::vector<PortalMote> portalMotes_;
    float moteSpawnTimer_   = 0.0f;
    float uiFloatTimer_     = 0.0f;

    // Optional: use existing particle system for small sparkle trails
    std::unique_ptr<ParticleManager> particleMgr_;
    ParticleEmitter* sparkleEmitter_ = nullptr;
    float sparkleSpawnTimer_ = 0.0f;

    // Ring rotation
    float ringRotation_ = 0.0f;

    // Fade out
    float transitionRingRotation_ = 0.0f;
    float transitionRingPulseTime_ = 0.0f;
    float fadeAlpha_ = 0.0f;
    State state_ = State::Idle;
    bool overlayPushed_ = false;
    bool reachedBlack_ = false;
    int  blackHoldFrames_ = 0;

    // Title drop & bounce
    float titleY_        = -260.0f;
    float titleTargetY_  = 160.0f;
    float titleVy_       = 0.0f;
    float titleGravity_  = 3.6f;
    float titleBounce_   = 0.72f;
    float titleStopEps_  = 0.35f;
    bool  titleSettled_  = false;

    float frameCount_ = 0.0f;
};
