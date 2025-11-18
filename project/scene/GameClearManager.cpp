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

    // 标题 Sprite
    titleSprite_ = new Sprite();
    titleSprite_->Initialize(spriteCommon_, "Resources/GameClear.png");
    titleSprite_->SetSize(titleSize_);
    titleSprite_->SetVisible(false);

    // GameClear 玩家模型（先创建，Start 时再设位置）
    clearPlayerObj_ = new Object3d();
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
    if (titleSprite_) {
        delete titleSprite_;
        titleSprite_ = nullptr;
    }
    if (clearPlayerObj_) {
        delete clearPlayerObj_;
        clearPlayerObj_ = nullptr;
    }
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

    // 标题从屏幕外上方滑到中间（基本照搬你原来的逻辑）
    titleEndPos_   = { (W - titleSize_.x) * 0.5f, (H - titleSize_.y) * 0.5f };
    titleStartPos_ = { titleEndPos_.x, -titleSize_.y - 40.0f };
    titlePos_      = titleStartPos_;

    if (titleSprite_) {
        titleSprite_->SetVisible(true);
        titleSprite_->SetPosition(titlePos_);
        titleSprite_->Update();
    }

    // 用屏幕坐标算「目标位置：画面中央偏下」
    Vector3 centerWorld = ScreenToWorld(
        W * 0.5f,
        H * 0.6f,
        hpNdcZ_,
        camera_);
    clearPlayerBasePos_ = centerWorld;

    // 起点：屏幕左侧内（用你当前的 0.2f，你可以之后再改成屏幕外）
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
        // 标题从上滑入（沿用你原来 GameOver 的 easeOutBack）
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

        const float moveDuration   = 1.0f; // 从左滑到中间
        const float rotateDuration = 1.0f; // 原地转头所用时间

        const float W = (float)WinApp::kClientWidth;
        const float H = (float)WinApp::kClientHeight;

        // 起点/终点（屏幕左侧 -> 屏幕中央）
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
            // 阶段1：从左边滑到屏幕中央
            float u = clearPlayerSpinT_ / moveDuration;
            float e = EaseOutCubic_(u);
            pos.x = startPos.x + (endPos.x - startPos.x) * e;
            pos.y = startPos.y + (endPos.y - startPos.y) * e;
            pos.z = startPos.z + (endPos.z - startPos.z) * e;
            // 这里你可以保持默认朝向
            yaw = 0.0f;
        } else {
            // 阶段2：已经在中央，慢慢转头
            pos = endPos;
            float rTime = clearPlayerSpinT_ - moveDuration;
            if (rTime > rotateDuration) { rTime = rotateDuration; }
            float v  = rotateDuration > 0.0f ? (rTime / rotateDuration) : 1.0f;
            float e  = EaseOutCubic_(v);
            float startYaw = 0.0f;
            float endYaw   = std::numbers::pi_v<float> * 0.5f; // 向右看一点
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
