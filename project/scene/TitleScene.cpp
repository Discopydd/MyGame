#include "TitleScene.h"
#include "SceneManager.h"
#include "scene/GameScene.h"
void TitleScene::Initialize() {
    winApp_ = WinApp::GetInstance();
    dxCommon_ = DirectXCommon::GetInstance();
    input_ = Input::GetInstance();
    srvManager_ = SrvManager::GetInstance();


    spriteCommon_ = new SpriteCommon();
    spriteCommon_->Initialize(dxCommon_);

    TextureManager::GetInstance()->Initialize(dxCommon_, srvManager_);


    std::string textureFilePath[] = { "Resources/black.png", "Resources/monsterBall.png" };
    titleSprite_ = new Sprite();
    titleSprite_->Initialize(spriteCommon_, textureFilePath[1]); // 一张纯黑/灰色贴图
    titleSprite_->SetPosition({ 0.0f, 0.0f });
    titleSprite_->SetSize({ (float)WinApp::kClientWidth, (float)WinApp::kClientHeight });
    fadeSprite_ = new Sprite();
    fadeSprite_->Initialize(spriteCommon_, textureFilePath[0]); // 一张纯黑/灰色贴图
    fadeSprite_->SetPosition({ 0.0f, 0.0f });
    fadeSprite_->SetSize({ (float)WinApp::kClientWidth, (float)WinApp::kClientHeight });
    fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    fadeSprite_->SetVisible(false);

    TextureManager::GetInstance()->LoadTexture("Resources/loading.png");
    loadingSprite_ = new Sprite();
    loadingSprite_->Initialize(spriteCommon_, "Resources/loading.png");
    loadingSprite_->SetPosition({ 0.0f, 0.0f });
    loadingSprite_->SetSize({ (float)WinApp::kClientWidth, (float)WinApp::kClientHeight });
    loadingSprite_->SetVisible(false);

    state_ = State::Idle;
    fadeAlpha_ = 0.0f;
    loadingHoldFrames_ = 0;
}

void TitleScene::Update() {
    input_->Update();

    switch (state_) {
    case State::Idle:
        // 按下 Space 开始淡出
        if (input_->TriggerKey(DIK_SPACE)) {
            state_ = State::FadingOut;
            fadeAlpha_ = 0.0f;
            if (fadeSprite_) fadeSprite_->SetVisible(true);
        }
        break;

    case State::FadingOut:
        // 递增透明度
        fadeAlpha_ += 0.02f; // 控制淡出速度
        if (fadeAlpha_ >= 1.0f) {
            fadeAlpha_ = 1.0f;
            // 进入“黑屏+Loading”阶段，先停留一段时间以确保能看到 Loading
            state_ = State::ShowingLoading;
            loadingHoldFrames_ = 30; // 约0.5秒@60fps，可自行调整或改成异步准备信号
            if (loadingSprite_) loadingSprite_->SetVisible(true);
        }
        break;

    case State::ShowingLoading:
        // 这里你也可以触发资源加载或预创建 GameScene
        // 为简单起见，停留一定帧后再切场景
        if (loadingHoldFrames_ > 0) {
            --loadingHoldFrames_;
        } else {
            BaseScene* next = new GameScene();
            sceneManager_->SetNextScene(next);
        }
        break;
    }

    // 更新精灵（若你的 Sprite 实现需要）
    titleSprite_->Update();
    fadeSprite_->SetColor({1.0f, 1.0f, 1.0f, fadeAlpha_});
    fadeSprite_->Update();
    loadingSprite_->Update();
}

void TitleScene::Draw() {
    dxCommon_->Begin();
    srvManager_->PreDraw();
    spriteCommon_->CommonDraw();

    // 1) 背景/标题
    if (titleSprite_) {
        titleSprite_->Draw();
    }

    // 2) 黑幕（根据 alpha 覆盖）
    if (fadeSprite_) {
        fadeSprite_->Draw();
    }

    // 3) 在“黑屏期间”画 Loading（要在黑幕之后画，才能显示在黑上面）
    if (state_ == State::ShowingLoading && loadingSprite_) {
        loadingSprite_->Draw();
    }

    dxCommon_->End();
}

void TitleScene::Finalize() {
    // 若 TextureManager 作为全局单例供后续场景继续使用，建议不要在这里 Finalize
    // TextureManager::GetInstance()->Finalize();

    delete spriteCommon_;  spriteCommon_ = nullptr;
    delete titleSprite_;   titleSprite_ = nullptr;
    delete fadeSprite_;    fadeSprite_ = nullptr;
    delete loadingSprite_; loadingSprite_ = nullptr;
}

