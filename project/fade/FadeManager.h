#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "WinApp.h"
#include <memory>
// 元の列挙名をそのまま使い、再利用しやすくする
enum class FadePhase { None, FadingOut, LoadingHold, FadingIn };

class FadeManager {
public:
    FadeManager() = default;
    ~FadeManager() = default;

    void Initialize(SpriteCommon* spriteCommon);
    void Finalize();

    void Update(float dt);
    void Draw();

    // Phase / Alpha へのアクセス
    FadePhase GetPhase() const { return phase_; }
    void      SetPhase(FadePhase p) { phase_ = p; }

    float GetAlpha() const { return alpha_; }
    void  SetAlpha(float a) {
        alpha_ = a;
        if (alpha_ < 0.0f) alpha_ = 0.0f;
        if (alpha_ > 1.0f) alpha_ = 1.0f;
        SyncVisual_();
    }
    }

    float GetSpeed() const { return speed_; }
    void  SetSpeed(float s) { speed_ = s; }

    Sprite* GetSprite() { return sprite_.get(); }

    // 簡易操作
    void StartFadeOut();   // 0 から 1 へフェード
    void StartFadeIn();    // 1 から 0 へフェード
    void SetBlack();       // 即座に全黒
    void Clear();          // 即座に透明

    // 現在のロジックで使っている「全黒到達 + 停留」もそのまま残す
    bool  ReachedBlack() const { return reachedBlack_; }
    void  SetReachedBlack(bool v) { reachedBlack_ = v; }

    int   GetBlackHoldFrames() const { return blackHoldFrames_; }
    void  SetBlackHoldFrames(int f) { blackHoldFrames_ = f; }

    bool  OverlayPushed() const { return overlayPushed_; }
    void  SetOverlayPushed(bool v) { overlayPushed_ = v; }

private:
    void SyncVisual_();

    SpriteCommon* spriteCommon_ = nullptr;
    std::unique_ptr<Sprite> sprite_;
    std::unique_ptr<Sprite> portalRingSprite_;

    float ringRotation_  = 0.0f;
    float ringPulseTime_ = 0.0f;

    FadePhase phase_   = FadePhase::None;
    float     alpha_   = 0.0f;
    float     speed_   = 0.16f;

    bool  reachedBlack_    = false;
    int   blackHoldFrames_ = 0;
    bool  overlayPushed_   = false;
};
