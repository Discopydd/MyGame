#include "MyGame.h"
void MyGame::Initialize() {
    Framework::Initialize();
    sceneManager_->SetNextScene(std::make_unique<TitleScene>());
}

void MyGame::Update() {
    Framework::Update();
}

void MyGame::Draw() {
    Framework::Draw();
}

void MyGame::Finalize() {
    Framework::Finalize();
}