#include "TitleScene.h"
#include "SceneManager.h"
#include "scene/GameScene.h"
#include "scene/LoadingScene.h"
#include <cmath>
#include <memory>

void TitleScene::Initialize() {
    winApp_   = WinApp::GetInstance();
    dxCommon_ = DirectXCommon::GetInstance();
    input_    = Input::GetInstance();
    srvManager_ = SrvManager::GetInstance();

    spriteCommon_ = SpriteCommon::GetInstance();

    TextureManager::GetInstance()->Initialize(dxCommon_, srvManager_);

    std::string textureFilePath[] = {
        "Resources/black.png",
        "Resources/GameTitle.dds",
        "Resources/Start.dds"
    };

    // --- 标题 Sprite ---
    titleSprite_ = std::make_unique<Sprite>();
    titleSprite_->Initialize(spriteCommon_, textureFilePath[1]);
    titleSprite_->SetPosition({ 0.0f, titleY_ });

    // --- Press Start Sprite ---
    startSprite_ = std::make_unique<Sprite>();
    startSprite_->Initialize(spriteCommon_, textureFilePath[2]);
    startSprite_->SetPosition({ 0.0f, 300.0f });

    // --- 淡出黑幕 Sprite ---
    fadeSprite_ = std::make_unique<Sprite>();
    fadeSprite_->Initialize(spriteCommon_, textureFilePath[0]);
    fadeSprite_->SetPosition({ 0.0f, 0.0f });
    fadeSprite_->SetSize({
        static_cast<float>(WinApp::kClientWidth),
        static_cast<float>(WinApp::kClientHeight)
    });
    fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    fadeSprite_->SetVisible(false);

    // === 背景 Sprite ===
    backgroundSprite_ = std::make_unique<Sprite>();
    backgroundSprite_->Initialize(spriteCommon_, "Resources/sky_bg.png");
    backgroundSprite_->SetAnchorPoint({ 0.0f, 0.0f });
    backgroundSprite_->SetPosition({ 0.0f, 0.0f });
    backgroundSprite_->SetSize({
        static_cast<float>(WinApp::kClientWidth),
        static_cast<float>(WinApp::kClientHeight)
    });

    state_      = State::Idle;
    fadeAlpha_  = 0.0f;
    frameCount_ = 0.0f;

    titleSettled_    = false;
    overlayPushed_   = false;
    reachedBlack_    = false;
    blackHoldFrames_ = 0;
}

void TitleScene::Update() {
    frameCount_++;
    input_->Update();

    // --- Title drop & bounce ---
    if (!titleSettled_) {
        // 重力
        titleVy_ += titleGravity_;
        titleY_  += titleVy_;

        // 触地判定与反弹
        if (titleY_ >= titleTargetY_) {
            titleY_ = titleTargetY_;
            titleVy_ = -titleVy_ * titleBounce_; // 反向并衰减

            // 速度很小时直接停住，避免细碎抖动
            if (std::fabs(titleVy_) < titleStopEps_) {
                titleVy_      = 0.0f;
                titleSettled_ = true;
            }
        }

        // 同步到精灵
        Vector2 pos = { 0.0f, titleY_ };
        if (titleSprite_) {
            titleSprite_->SetPosition(pos);
        }
    }

    // --- Start 字样闪烁 ---
    float blinkSpeed = 0.10f; // 数值越小闪烁越慢
    float alpha = (std::sin(frameCount_ * blinkSpeed) * 0.5f + 0.5f); // 0~1 波动
    if (startSprite_) {
        startSprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha });
    }

    // --- 状态机 ---
    switch (state_) {
    case State::Idle:
        // 按下 Space 开始淡出
        if (input_->TriggerKey(DIK_SPACE)) {
            state_ = State::FadingOut;
            fadeAlpha_ = 0.0f;
            if (fadeSprite_) {
                fadeSprite_->SetVisible(true);
            }
        }
        break;

    case State::FadingOut:
        // 递增透明度
        fadeAlpha_ += 0.04f; // 控制淡出速度
        if (fadeAlpha_ >= 1.0f) {
            fadeAlpha_ = 1.0f;
            if (!reachedBlack_) {
                reachedBlack_ = true;
                blackHoldFrames_ = 1;        // 先纯黑停 1 帧
                break;                       // 本帧先不叠加 Loading
            }
            if (blackHoldFrames_ > 0) {      // 消耗黑屏保留帧
                --blackHoldFrames_;
                break;
            }
            if (!overlayPushed_) {           // 现在才叠加 Loading
                // 使用 make_unique + release 避免直接写 new
                sceneManager_->SetOverlayScene(
                    std::unique_ptr<BaseScene>(std::make_unique<LoadingScene>().release())
                );
                overlayPushed_ = true;
            }
            sceneManager_->SetNextScene(
                std::unique_ptr<BaseScene>(std::make_unique<GameScene>().release())
            );
            return;
        }
        break;
    }

    // 更新精灵（若 Sprite 实现需要）
    if (backgroundSprite_) {
        backgroundSprite_->Update();
    }
    if (titleSprite_) {
        titleSprite_->Update();
    }
    if (startSprite_) {
        startSprite_->Update();
    }
    if (fadeSprite_) {
        fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, fadeAlpha_ });
        fadeSprite_->Update();
    }
}

void TitleScene::Draw() {
    dxCommon_->Begin();
    srvManager_->PreDraw();
    spriteCommon_->CommonDraw();

    if (backgroundSprite_) {
        backgroundSprite_->Draw();
    }

    // 1) 背景/标题
    if (titleSprite_) {
        titleSprite_->Draw();
    }

    if (startSprite_) {
        startSprite_->Draw();
    }

    // 2) 黑幕（根据 alpha 覆盖）
    if (fadeSprite_) {
        fadeSprite_->Draw();
    }

    dxCommon_->End();
}

void TitleScene::Finalize() {
    // 若 TextureManager 作为全局单例供后续场景继续使用，是否在这里 Finalize
    // 取决于你的整体设计，这里保持和原来一样
    TextureManager::GetInstance()->Finalize();

    // unique_ptr 自动释放资源，如需提前释放可主动 reset
    backgroundSprite_.reset();
    titleSprite_.reset();
    startSprite_.reset();
    fadeSprite_.reset();
}
