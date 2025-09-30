#include "LoadingScene.h"

void LoadingScene::Initialize() {
    winApp_ = WinApp::GetInstance();
    dxCommon_ = DirectXCommon::GetInstance();
    input_ = Input::GetInstance();
    srvManager_ = SrvManager::GetInstance();

    spriteCommon_ = new SpriteCommon();
    spriteCommon_->Initialize(dxCommon_);

    TextureManager::GetInstance()->Initialize(dxCommon_, srvManager_);
    TextureManager::GetInstance()->LoadTexture("Resources/loading.png");

    loadingSprite_ = new Sprite();
    loadingSprite_->Initialize(spriteCommon_, "Resources/loading.png");
    loadingSprite_->SetPosition({ 0,0 });
    loadingSprite_->SetSize({ (float)WinApp::kClientWidth, (float)WinApp::kClientHeight });

    TextureManager::GetInstance()->LoadTexture("Resources/black.png");

    // 黑幕
    blackSprite_ = new Sprite();
    blackSprite_->Initialize(spriteCommon_, "Resources/black.png");
    blackSprite_->SetPosition({ 0,0 });
    blackSprite_->SetSize({ (float)WinApp::kClientWidth, (float)WinApp::kClientHeight });

}

void LoadingScene::Update() {
    blackSprite_->Update();
    loadingSprite_->Update();

}

void LoadingScene::Draw() {
    dxCommon_->Begin();
    srvManager_->PreDraw();
    spriteCommon_->CommonDraw();
    if (blackSprite_) {
        blackSprite_->Draw();
    }
    if (loadingSprite_) {
        loadingSprite_->Draw();
    }
  
    
    dxCommon_->End();
}

void LoadingScene::Finalize() {
    delete loadingSprite_;
    delete blackSprite_;
    delete spriteCommon_;
}