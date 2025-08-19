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
    loadingSprite_->SetPosition({ 400,300 });
    loadingSprite_->SetSize({ 256,128 });
    
 
}

void LoadingScene::Update() {

    loadingSprite_->Update();

}

void LoadingScene::Draw() {
    dxCommon_->Begin();
    srvManager_->PreDraw();
    spriteCommon_->CommonDraw();
    
    if (loadingSprite_) {
        loadingSprite_->Draw();
    }
  
    
    dxCommon_->End();
}

void LoadingScene::Finalize() {
    delete loadingSprite_;
    delete spriteCommon_;
}