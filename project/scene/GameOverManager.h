#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "WinApp.h"

class GameOverManager {
public:
    enum class State {
        None,
        SlideTitle,   // タイトルが上から滑り込む
        Wait,         // タイトルが中央で停止し、プレイヤー入力を待つ
        Done
    };

    GameOverManager() = default;
    ~GameOverManager() = default;

    void Initialize(SpriteCommon* spriteCommon);
    void Finalize();

    void Start();          // GameOver 演出を開始
    void Update(float dt); // GameScene::Update 内で呼び出す
    void Draw();           // GameScene::Draw 内で呼び出す

    bool IsPlaying() const {
        return (state_ != State::None && state_ != State::Done);
    }
    State GetState() const { return state_; }

private:
    State  state_ = State::None;
    float  t_     = 0.0f;      // タイマー

    std::unique_ptr<Sprite> titleSprite_;
    // 従来どおり: 画面全体に大きく表示する GameOver
    Vector2 titleSize_   = { 500.0f, 300.0f };
    Vector2 titlePos_{};
    Vector2 titleStartPos_{};
    Vector2 titleEndPos_{};
    float   titleSlideTime_ = 0.65f;   // タイトル滑入時間

    SpriteCommon* spriteCommon_ = nullptr;

    // 元の GameOver と同じ easeOutBack を使用
    float EaseOutBack_(float t) {
        float c1 = 1.70158f;
        float c3 = c1 + 1.0f;
        float p  = t - 1.0f;
        return 1.0f + p * p * (c3 * p + c1);
    }
};
