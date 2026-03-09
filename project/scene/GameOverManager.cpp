#include "GameOverManager.h"

void GameOverManager::Initialize(SpriteCommon* spriteCommon)
{
    spriteCommon_ = spriteCommon;

    titleSprite_ = std::make_unique<Sprite>();
    titleSprite_->Initialize(spriteCommon_, "Resources/GameOver.png");
    titleSprite_->SetSize(titleSize_);
    titleSprite_->SetVisible(false);

    state_ = State::None;
    t_     = 0.0f;
}

void GameOverManager::Finalize()
{
    titleSprite_.reset();
}

void GameOverManager::Start()
{
    if (state_ != State::None && state_ != State::Done) {
        return; // すでに GameOver 中
    }

    state_ = State::SlideTitle;
    t_     = 0.0f;

    float W = (float)WinApp::kClientWidth;
    float H = (float)WinApp::kClientHeight;

    // タイトル最終位置: 画面中央（サイズに応じて中央寄せ）
    titleEndPos_   = { (W - titleSize_.x) * 0.5f, (H - titleSize_.y) * 0.5f };
    // 開始位置: 画面上側から入る
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

        // 元の GameOver で使っていた easeOutBack の効果
        float e = EaseOutBack_(d);

        titlePos_.x = titleStartPos_.x + (titleEndPos_.x - titleStartPos_.x) * e;
        titlePos_.y = titleStartPos_.y + (titleEndPos_.y - titleStartPos_.y) * e;

        if (titleSprite_) {
            titleSprite_->SetPosition(titlePos_);
            titleSprite_->Update();
        }

        if (d >= 1.0f) {
            state_ = State::Wait; // 待機状態へ移行（Space でタイトルへ戻る）
            t_ = 0.0f;
        }
    }
    else if (state_ == State::Wait) {
        // 現状では何もしない。タイトルを中央に止めておくだけ
        // 実際のタイトル復帰は GameScene 側で Space 入力検出 + フェードアウト処理を行う
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
