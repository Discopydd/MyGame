#include "MyGame.h"
void MyGame::Initialize() {
    Framework::Initialize();
    BaseScene* scene = new TitleScene();
    sceneManager_->SetNextScene(scene);
}

void MyGame::Update() {
    Framework::Update();
    sceneManager_->Update();
}

void MyGame::Draw() {
    sceneManager_->Draw();
}

void MyGame::Finalize() {
    Framework::Finalize();
}