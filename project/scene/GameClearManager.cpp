#include "GameClearManager.h"
#include <numbers>

Vector3 ScreenToWorld(float screenX, float screenY, float ndcZ, Camera* camera);

void GameClearManager::Initialize(SpriteCommon* spriteCommon,
                                  Object3dCommon* object3dCommon,
                                  Camera* camera,
                                  float hpNdcZ) {
    spriteCommon_   = spriteCommon;
    object3dCommon_ = object3dCommon;
    camera_         = camera;
    hpNdcZ_         = hpNdcZ;

    // タイトル Sprite
    titleSprite_ = std::make_unique<Sprite>();
    titleSprite_->Initialize(spriteCommon_, "Resources/GameClear.png");
    titleSprite_->SetSize(titleSize_);
    titleSprite_->SetVisible(false);

    // GameClear 用プレイヤーモデル（先に生成し、Start 時に位置を設定）
    clearPlayerObj_ = std::make_unique<Object3d>();
    clearPlayerObj_->Initialize(object3dCommon_);
    clearPlayerObj_->SetModel("player/player.obj");
    clearPlayerObj_->SetCamera(camera_);
    clearPlayerObj_->SetScale({ 0.004f, 0.004f, 0.004f });
    clearPlayerObj_->SetRotate({ 0.0f, 0.0f, 0.0f });

    state_ = State::None;
    t_     = 0.0f;
    clearPlayerSpinT_ = 0.0f;
}

void GameClearManager::Finalize() {
    titleSprite_.reset();
    clearPlayerObj_.reset();
}

void GameClearManager::Start() {
    if (state_ != State::None && state_ != State::Done) {
        return;
    }

    state_ = State::SlideTitle;
    t_ = 0.0f;
    clearPlayerSpinT_ = 0.0f;

    const float W = (float)WinApp::kClientWidth;
    const float H = (float)WinApp::kClientHeight;

    // タイトルが画面外の上側から中央へ滑り込む（基本的に元のロジックを踏襲）
    titleEndPos_   = { (W - titleSize_.x) * 0.5f, (H - titleSize_.y) * 0.5f };
    titleStartPos_ = { titleEndPos_.x, -titleSize_.y - 40.0f };
    titlePos_      = titleStartPos_;

    if (titleSprite_) {
        titleSprite_->SetVisible(true);
        titleSprite_->SetPosition(titlePos_);
        titleSprite_->Update();
    }

    // 画面座標から「目標位置: 画面中央やや下」を計算する
    Vector3 centerWorld = ScreenToWorld(
        W * 0.5f,
        H * 0.6f,
        hpNdcZ_,
        camera_);
    clearPlayerBasePos_ = centerWorld;

    // 開始位置: 画面左側の内側（現在の 0.2f を使用。後で画面外に変更してもよい）
    Vector3 fromLeftWorld = ScreenToWorld(
        W * -0.2f,
        H * 0.6f,
        hpNdcZ_,
        camera_);
    clearPlayerStartPos_ = fromLeftWorld;

    if (clearPlayerObj_) {
        clearPlayerObj_->SetTranslate(clearPlayerStartPos_);
        clearPlayerObj_->Update();
    }
}

void GameClearManager::Update(float dt) {
    if (state_ == State::None || state_ == State::Done) {
        return;
    }

    t_ += dt;

    switch (state_) {
    case State::SlideTitle: {
        // タイトルを上から滑り込ませる（元の GameOver の easeOutBack を流用）
        float d = (std::min)(1.0f, t_ / titleSlideTime_);
        float s = 1.70158f;
        float p = d - 1.0f;
        float e = 1.0f + (p * p * ((s + 1.0f) * p + s));

        titlePos_.x = titleStartPos_.x + (titleEndPos_.x - titleStartPos_.x) * e;
        titlePos_.y = titleStartPos_.y + (titleEndPos_.y - titleStartPos_.y) * e;

        if (titleSprite_) {
            titleSprite_->SetPosition(titlePos_);
            titleSprite_->Update();
        }

        if (d >= 1.0f) {
            state_ = State::PlayerShow;
            t_ = 0.0f;
            clearPlayerSpinT_ = 0.0f;
        }
        break;
    }
    case State::PlayerShow: {
        if (!clearPlayerObj_) { break; }

        clearPlayerSpinT_ += dt;

        const float moveDuration   = 1.0f; // 左から中央まで滑らせる
        const float rotateDuration = 1.0f; // その場で向きを変える時間

        const float W = (float)WinApp::kClientWidth;
        const float H = (float)WinApp::kClientHeight;

        // 開始位置／終点（画面左側 -> 画面中央）
        Vector3 startPos = ScreenToWorld(
            W * -0.2f,
            H * 0.6f,
            hpNdcZ_,
            camera_);
        Vector3 endPos = ScreenToWorld(
            W * 0.5f,
            H * 0.6f,
            hpNdcZ_,
            camera_);

        Vector3 pos{};
        float   yaw = 0.0f;

        if (clearPlayerSpinT_ < moveDuration) {
            // 段階1: 左側から画面中央へ滑らせる
            float u = clearPlayerSpinT_ / moveDuration;
            float e = EaseOutCubic_(u);
            pos.x = startPos.x + (endPos.x - startPos.x) * e;
            pos.y = startPos.y + (endPos.y - startPos.y) * e;
            pos.z = startPos.z + (endPos.z - startPos.z) * e;
            // ここではデフォルトの向きを維持してよい
            yaw = 0.0f;
        } else {
            // 段階2: すでに中央に到達しているので、ゆっくり向きを変える
            pos = endPos;
            float rTime = clearPlayerSpinT_ - moveDuration;
            if (rTime > rotateDuration) { rTime = rotateDuration; }
            float v  = rotateDuration > 0.0f ? (rTime / rotateDuration) : 1.0f;
            float e  = EaseOutCubic_(v);
            float startYaw = 0.0f;
            float endYaw   = std::numbers::pi_v<float> * 0.5f; // 少し右を見る
            yaw = startYaw + (endYaw - startYaw) * e;
        }

        clearPlayerObj_->SetTranslate(pos);
        clearPlayerObj_->SetRotate({ 0.0f, yaw, 0.0f });
        clearPlayerObj_->Update();
        break;
    }
    default:
        break;
    }
}

void GameClearManager::DrawTitle() {
    if (!spriteCommon_ || !titleSprite_) { return; }
    if (state_ == State::None || state_ == State::Done) { return; }

    if (titleSprite_->IsVisible()) {
        titleSprite_->Draw();
    }
}

void GameClearManager::DrawPlayer() {
    if (!clearPlayerObj_) { return; }
    if (state_ == State::None || state_ == State::Done) { return; }

    clearPlayerObj_->Draw();
}
