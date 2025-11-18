#include "GameOverManager.h"

void GameOverManager::Initialize(SpriteCommon* spriteCommon)
{
    spriteCommon_ = spriteCommon;

    titleSprite_ = new Sprite();
    titleSprite_->Initialize(spriteCommon_, "Resources/GameOver.png");
    titleSprite_->SetSize(titleSize_);
    titleSprite_->SetVisible(false);

    state_ = State::None;
    t_     = 0.0f;
}

void GameOverManager::Finalize()
{
    if (titleSprite_) {
        delete titleSprite_;
        titleSprite_ = nullptr;
    }
}

void GameOverManager::Start()
{
    if (state_ != State::None && state_ != State::Done) {
        return; // 已经在 GameOver 里了
    }

    state_ = State::SlideTitle;
    t_     = 0.0f;

    float W = (float)WinApp::kClientWidth;
    float H = (float)WinApp::kClientHeight;

    // 标题最终位置：屏幕中央（根据大小居中）
    titleEndPos_   = { (W - titleSize_.x) * 0.5f, (H - titleSize_.y) * 0.5f };
    // 起点：从屏幕上方移入
    titleStartPos_ = { titleEndPos_.x, -titleSize_.y - 40.0f };
    titlePos_      = titleStartPos_;

    if (titleSprite_) {
        titleSprite_->SetVisible(true);
        titleSprite_->SetPosition(titlePos_);
        titleSprite_->Update();
    }
}

void GameOverManager::Update(float dt)
{
    if (state_ == State::None || state_ == State::Done) {
        return;
    }

    t_ += dt;

    if (state_ == State::SlideTitle) {
        float d = t_ / titleSlideTime_;
        if (d > 1.0f) d = 1.0f;

        // 你原来 GameOver 用的 easeOutBack 效果
        float e = EaseOutBack_(d);

        titlePos_.x = titleStartPos_.x + (titleEndPos_.x - titleStartPos_.x) * e;
        titlePos_.y = titleStartPos_.y + (titleEndPos_.y - titleStartPos_.y) * e;

        if (titleSprite_) {
            titleSprite_->SetPosition(titlePos_);
            titleSprite_->Update();
        }

        if (d >= 1.0f) {
            state_ = State::Wait; // 进入等待状态（按 Space 回标题）
            t_ = 0.0f;
        }
    }
    else if (state_ == State::Wait) {
        // 目前什么都不做，只是标题停在中间
        // 真正回标题由 GameScene 里检测 Space + 淡出处理
    }
}

void GameOverManager::Draw()
{
    if (!spriteCommon_ || !titleSprite_) { return; }
    if (state_ == State::None || state_ == State::Done) { return; }

    if (titleSprite_->IsVisible()) {
        titleSprite_->Draw();
    }
}
