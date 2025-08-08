#include "SceneManager.h"

SceneManager::~SceneManager() {
    // 最后一个场景的终止和释放
    if (scene_) {
        scene_->Finalize();
        delete scene_;
    }
     if (nextScene_) {
        delete nextScene_;
        nextScene_ = nullptr;
    }
}

void SceneManager::SetNextScene(BaseScene* nextScene) {
    nextScene_ = nextScene;
}

void SceneManager::Update() {
    // 切换场景（如果有预约）
    if (nextScene_) {
        // 当前场景的终止和释放
        if (scene_) {
            scene_->Finalize();
            delete scene_;
        }

        // 切换到新场景
        scene_ = nextScene_;
        nextScene_ = nullptr;
        scene_->SetSceneManager(this);
        // 初始化新场景
        scene_->Initialize();
    }

    // 更新当前场景
    if (scene_) {
        scene_->Update();
    }
}

void SceneManager::Draw() {
    // 调用当前场景的绘制
    if (scene_) {
        scene_->Draw();
    }
}
