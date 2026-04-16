#include "Framework.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "SoundManager.h"
void Framework::Initialize() {
    sceneManager_ = std::make_unique<SceneManager>();
     WinApp::GetInstance()->Initialize();
    DirectXCommon::GetInstance()->Initialize(WinApp::GetInstance());
    Input::GetInstance()->Initialize(WinApp::GetInstance());
    SrvManager::GetInstance()->Initialize(DirectXCommon::GetInstance());
    SpriteCommon::GetInstance()->Initialize(DirectXCommon::GetInstance());
}
void Framework::Run() {
    Initialize();

    while (!endRequest_) {
        if (WinApp::GetInstance()->ProcessMessage()) {
            break;
        }
        Update();
        Draw();
    }

    Finalize();
}
void Framework::Update() {
    sceneManager_->Update();
}

void Framework::Draw() {
    sceneManager_->Draw();
}
void Framework::Finalize() {
    sceneManager_.reset();

    SoundManager::GetInstance()->Finalize();
    TextureManager::GetInstance()->Finalize();
    ModelManager::GetInstants()->Finalize();

    DirectXCommon::GetInstance()->Finalize();
    WinApp::GetInstance()->Finalize();
}