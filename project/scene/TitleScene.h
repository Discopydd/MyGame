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
        ShowingLoading
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
    Sprite* loadingSprite_ = nullptr;
    bool showLoading_ = false;

    State state_ = State::Idle;
    int   loadingHoldFrames_ = 0;     // 黑幕上显示 Loading 的停留帧数
};
