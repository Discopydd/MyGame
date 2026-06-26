#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "WinApp.h"

#include <memory>
#include <vector>

class GameOverManager {
public:
    enum class State {
        None,
        PortalOpen,   // 暗転＋ポータル崩壊演出
        SlideTitle,   // タイトルが上から滑り込む
        Wait,         // タイトルが停止し、入力を待つ
        Done
    };

    GameOverManager() = default;
    ~GameOverManager() = default;

    void Initialize(MyEngine::SpriteCommon* spriteCommon);
    void Finalize();

    void Start();          // GameOver 演出を開始
    void Update(float dt); // GameScene::Update 内で呼び出す
    void Draw();           // GameScene::Draw 内で呼び出す

    bool IsPlaying() const {
        return (state_ != State::None && state_ != State::Done);
    }
    State GetState() const { return state_; }
    void SetTextVisible(bool visible) { showTexts_ = visible; }
    void SetDrawEnabled(bool enabled) { drawEnabled_ = enabled; }

private:
    struct PortalMote {
        std::unique_ptr<MyEngine::Sprite> sprite;
        float angle = 0.0f;
        float radius = 0.0f;
        float angularSpeed = 0.0f;
        float radialSpeed = 0.0f;
        float driftY = 0.0f;
        float life = 0.0f;
        float maxLife = 0.0f;
        float baseSize = 16.0f;
        MyEngine::Vector4 color = { 1,1,1,1 };
    };

private:
    State  state_ = State::None;
    float  t_     = 0.0f;

    std::unique_ptr<MyEngine::Sprite> backdropSprite_;
    std::unique_ptr<MyEngine::Sprite> portalRingSprite_;
    std::unique_ptr<MyEngine::Sprite> titleSprite_;
    std::unique_ptr<MyEngine::Sprite> promptSprite_;

    std::vector<PortalMote> motes_;
    float moteSpawnTimer_ = 0.0f;

    MyEngine::Vector2 titleSize_      = { 520.0f, 312.0f };
    MyEngine::Vector2 titleCenter_{};
    MyEngine::Vector2 titleStartPos_{};
    MyEngine::Vector2 titleEndPos_{};
    float   titleSlideTime_ = 0.65f;

    MyEngine::Vector2 ringCenter_{};
    MyEngine::Vector2 ringBaseSize_ = { 620.0f, 620.0f };
    float   ringRotation_ = 0.0f;
    float   ringPulseT_   = 0.0f;
    float   backdropAlpha_ = 0.0f;

    MyEngine::Vector2 promptBasePos_{};

    MyEngine::SpriteCommon* spriteCommon_ = nullptr;
    bool showTexts_ = true;
    bool drawEnabled_ = true;

private:
    void SpawnMote_();
    void UpdateMotes_(float dt);

    float Clamp01_(float t) {
        if (t < 0.0f) { return 0.0f; }
        if (t > 1.0f) { return 1.0f; }
        return t;
    }

    float EaseOutBack_(float t) {
        float c1 = 1.70158f;
        float c3 = c1 + 1.0f;
        float p  = t - 1.0f;
        return 1.0f + p * p * (c3 * p + c1);
    }

    float EaseOutCubic_(float t) {
        t = Clamp01_(t);
        return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
    }
};
