#include "GameScene.h"
#include <numbers>

void GameScene::Initialize() {
    winApp_ = WinApp::GetInstance();
    dxCommon_ = DirectXCommon::GetInstance();
    input_ = Input::GetInstance();
    srvManager_ = SrvManager::GetInstance();

    spriteCommon_ = new SpriteCommon();
    spriteCommon_->Initialize(dxCommon_);


    TextureManager::GetInstance()->Initialize(dxCommon_, srvManager_);

    imguiManager_ = new ImGuiManager();
    imguiManager_->Initialize(winApp_, dxCommon_, srvManager_);

    object3dCommon_ = new Object3dCommon();
    object3dCommon_->Initialize(dxCommon_);

    ModelManager::GetInstants()->Initialize(dxCommon_);
    SoundManager* soundMgr = SoundManager::GetInstance();
    soundMgr->Initialize();
    soundMgr->LoadWav("fanfare", "resources/fanfare.wav");

    camera_ = new Camera();
    camera_->SetRotate({ 0, 0, 0 });
    camera_->SetTranslate({ 0, 0, -10 });
    object3dCommon_->SetDefaultCamera(camera_);

    ModelManager::GetInstants()->LoadModel("cube.obj");
    box_ = new Object3d();
    box_->Initialize(object3dCommon_);
    box_->SetModel("box.obj");
    box_->SetCamera(camera_);
    box_->SetTranslate({ 0.0f, -1.0f,5.0f });

}

void GameScene::Update() {
    camera_->Update();
    imguiManager_->Begin();
    input_->Update();

    if (input_->TriggerKey(DIK_SPACE)) {
        SoundManager::GetInstance()->Play("fanfare", false, 1.0f);
    }

#ifdef USE_IMGUI
    ImGui::Begin("Scene Controller");



    ImGui::End();
#endif


    imguiManager_->End();
}

void GameScene::Draw() {
    dxCommon_->Begin();
    srvManager_->PreDraw();
    object3dCommon_->CommonDraw();

    box_->Draw();
    spriteCommon_->CommonDraw();
    for (auto* sprite : sprites_) {
        sprite->Draw();
    }
    // ParticleManager::GetInstance()->Draw();
    imguiManager_->Draw();
    dxCommon_->End();
}

void GameScene::Finalize() {
    ParticleManager::GetInstance()->Finalize();
    TextureManager::GetInstance()->Finalize();
    ModelManager::GetInstants()->Finalize();
    imguiManager_->Finalize();

    delete camera_;
    delete spriteCommon_;
    delete object3dCommon_;
    delete box_;
    delete imguiManager_;
    delete particleEmitter_;

    for (auto* sprite : sprites_) {
        delete sprite;
    }
}
