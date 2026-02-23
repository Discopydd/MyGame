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

class TitleScene : public BaseScene {
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
        std::unique_ptr<Sprite> sprite;
        float angle = 0.0f;         // rad
        float radius = 0.0f;        // px
        float angularSpeed = 0.0f;  // rad/s
        float radialSpeed = 0.0f;   // px/s
        float ySpeed = 0.0f;        // px/s (negative = up)
        float yOffset = 0.0f;       // px
        float life = 0.0f;          // s
        float maxLife = 0.0f;       // s
        float baseSize = 16.0f;     // px
        Vector4 color = { 1,1,1,1 };
    };

    // Non-owning pointers
    WinApp*        winApp_       = nullptr;
    DirectXCommon* dxCommon_     = nullptr;
    Input*         input_        = nullptr;
    SrvManager*    srvManager_   = nullptr;
    SpriteCommon*  spriteCommon_ = nullptr;

    // Sprites (owned)
    std::unique_ptr<Sprite> backgroundSprite_;
    std::unique_ptr<Sprite> portalRingSprite_;
    std::unique_ptr<Sprite> titlePanelSprite_;
    std::unique_ptr<Sprite> titleLogoSprite_;
    std::unique_ptr<Sprite> startSprite_;
    std::unique_ptr<Sprite> fadeSprite_;

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
