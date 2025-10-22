#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelManager.h"
#include "MyMath.h"
#include "ImGuiManager.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "SoundManager.h"
#include "Camera.h"
#include <vector>
#include "BaseScene.h"

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
    WinApp* winApp_ = nullptr;
    DirectXCommon* dxCommon_ = nullptr;
    Input* input_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;
    std::vector<Sprite*> sprites_;
    Vector2 rotation_{};

    bool isFadingOut_ = false;       // 是否正在淡出
    float fadeAlpha_ = 0.0f;         // 当前淡出透明度 (0=透明,1=全黑)
    Sprite* fadeSprite_ = nullptr;   // 用于覆盖全屏的淡出精灵

    Sprite* titleSprite_ = nullptr;
    Sprite* startSprite_ = nullptr;

    State state_ = State::Idle;

    // --- Title drop & bounce (tuned) ---
    float titleY_ = -260.0f;  // 起始更高一点，入场更明显
    float titleTargetY_ = 0.0f;    // 着陆位置
    float titleVy_ = 0.0f;
    float titleGravity_ = 0.90f;    // 重力稍小，弹跳节奏更柔
    float titleBounce_ = 0.72f;    // 弹性更大 → 弹跳次数更多
    float titleStopEps_ = 0.35f;    // 终止阈值更小 → 不会太早停
    bool  titleSettled_ = false;

    float frameCount_ = 0;

    bool overlayPushed_ = false;

    bool reachedBlack_ = false;
    int  blackHoldFrames_ = 0;
};
