#include "Framework.h"
void Framework::Initialize() {
    sceneManager_ = new SceneManager();
     WinApp::GetInstance()->Initialize();
    DirectXCommon::GetInstance()->Initialize(WinApp::GetInstance());
    Input::GetInstance()->Initialize(WinApp::GetInstance());
    SrvManager::GetInstance()->Initialize(DirectXCommon::GetInstance());
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
    delete sceneManager_;
    sceneManager_ = nullptr;

    DirectXCommon::GetInstance()->Finalize();
    WinApp::GetInstance()->Finalize();
}