#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Input.h"
#include "WinApp.h"
#include "MyMath.h"

class IntroManager {
public:
    enum class State { None, BarsIn, OrbitZoom, TitleShow, BarsOut, Done };

    IntroManager() = default;
    ~IntroManager() = default;

    void Initialize(SpriteCommon* spriteCommon, Input* input);
    void Finalize();

    // 开始 Intro，playerPos 用来当环绕中心（现在只是存着，之后你想加镜头动也可以用）
    void Start(const Vector3& playerPos);

    // 每帧更新（在 GameScene::Update 里调用）
    void Update(float dt);

    // 绘制 Intro UI（在 GameScene::Draw 里调用）
    void Draw();

    // 状态查询
    bool IsPlaying() const {
        return (state_ != State::None && state_ != State::Done);
    }
    bool HasStarted() const { return started_; }
    State GetState() const { return state_; }

private:
    // 状态
    State state_   = State::None;
    float t_       = 0.0f;
    bool  skippable_ = true;   // 是否允许按键跳过
    bool  started_   = false;  // 是否已经启动过一次

    // Sprite 们
    Sprite* letterboxTop_    = nullptr;
    Sprite* letterboxBottom_ = nullptr;
    Sprite* vignette_        = nullptr;
    Sprite* introTitle_      = nullptr;
    Sprite* skipHint_        = nullptr;

    // 摄像机相关（目前只保存，不在这里直接动相机）
    Vector3 camStartPos_{ 0, 12, -85 };
    Vector3 camTargetPos_{ 0,  8, -38 };
    Vector3 camPivot_{ 0, 0, 0 };
    float   camOrbitDeg_ = 0.0f;

    // 屏幕震动（如果以后想用，可以从这里对 camera 做位移）
    float shakeTime_ = 0.0f;
    float shakeAmp_  = 0.0f;

    SpriteCommon* spriteCommon_ = nullptr;
    Input*        input_        = nullptr;

    // Easing（照搬你 GameScene 里的实现）
    float EaseOutCubic_(float t) { return 1.0f - powf(1.0f - t, 3.0f); }
    float EaseInOutSine_(float t) { return 0.5f * (1.0f - cosf(3.1415926f * t)); }
};
