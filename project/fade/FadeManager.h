#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "WinApp.h"

// 和你原来的一样的枚举名字，方便复用
enum class FadePhase { None, FadingOut, LoadingHold, FadingIn };

class FadeManager {
public:
    FadeManager() = default;
    ~FadeManager() = default;

    void Initialize(SpriteCommon* spriteCommon);
    void Finalize();

    void Update(float dt);
    void Draw();

    // Phase / Alpha 访问
    FadePhase GetPhase() const { return phase_; }
    void      SetPhase(FadePhase p) { phase_ = p; }

    float GetAlpha() const { return alpha_; }
    void  SetAlpha(float a) {
        alpha_ = a;
        if (alpha_ < 0.0f) alpha_ = 0.0f;
        if (alpha_ > 1.0f) alpha_ = 1.0f;
        if (sprite_) {
            sprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha_ });
            sprite_->Update();
        }
    }

    float GetSpeed() const { return speed_; }
    void  SetSpeed(float s) { speed_ = s; }

    Sprite* GetSprite() { return sprite_; }

    // 快捷操作
    void StartFadeOut();   // 从 0 淡到 1
    void StartFadeIn();    // 从 1 淡到 0
    void SetBlack();       // 直接全黑
    void Clear();          // 直接透明

    // 你目前逻辑用到的「黑到头 + 停留」也保留
    bool  ReachedBlack() const { return reachedBlack_; }
    void  SetReachedBlack(bool v) { reachedBlack_ = v; }

    int   GetBlackHoldFrames() const { return blackHoldFrames_; }
    void  SetBlackHoldFrames(int f) { blackHoldFrames_ = f; }

    bool  OverlayPushed() const { return overlayPushed_; }
    void  SetOverlayPushed(bool v) { overlayPushed_ = v; }

private:
    SpriteCommon* spriteCommon_ = nullptr;
    Sprite*       sprite_       = nullptr;

    FadePhase phase_   = FadePhase::None;
    float     alpha_   = 0.0f;
    float     speed_   = 0.16f;

    bool  reachedBlack_    = false;
    int   blackHoldFrames_ = 0;
    bool  overlayPushed_   = false;
};
