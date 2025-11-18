#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "WinApp.h"

class GameOverManager {
public:
    enum class State {
        None,
        SlideTitle,   // 标题从上滑入
        Wait,         // 标题停在中间，等待玩家输入
        Done
    };

    GameOverManager() = default;
    ~GameOverManager() = default;

    void Initialize(SpriteCommon* spriteCommon);
    void Finalize();

    void Start();          // 开始 GameOver 演出
    void Update(float dt); // 在 GameScene::Update 里调用
    void Draw();           // 在 GameScene::Draw 里调用

    bool IsPlaying() const {
        return (state_ != State::None && state_ != State::Done);
    }
    State GetState() const { return state_; }

private:
    State  state_ = State::None;
    float  t_     = 0.0f;      // 计时

    Sprite* titleSprite_ = nullptr;
    // 和原来一样：整屏大字 GameOver
    Vector2 titleSize_   = { 500.0f, 300.0f };
    Vector2 titlePos_{};
    Vector2 titleStartPos_{};
    Vector2 titleEndPos_{};
    float   titleSlideTime_ = 0.65f;   // 标题滑入时间

    SpriteCommon* spriteCommon_ = nullptr;

    // 和你原来 GameOver 用的一样的 easeOutBack
    float EaseOutBack_(float t) {
        float c1 = 1.70158f;
        float c3 = c1 + 1.0f;
        float p  = t - 1.0f;
        return 1.0f + p * p * (c3 * p + c1);
    }
};
