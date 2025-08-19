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
    if (overlayScene_) {
        overlayScene_->Finalize();
        delete overlayScene_;
        overlayScene_ = nullptr;
    }
}
void SceneManager::SetOverlayScene(BaseScene* overlayScene) {
    if (overlayScene_) {
        overlayScene_->Finalize();
        delete overlayScene_;
    }
    overlayScene_ = overlayScene;
    if (overlayScene_) {
        overlayScene_->SetSceneManager(this);
        overlayScene_->Initialize();
    }
}

void SceneManager::ClearOverlayScene() {
    if (overlayScene_) {
        overlayScene_->Finalize();
        delete overlayScene_;
        overlayScene_ = nullptr;
    }
}
void SceneManager::SetNextScene(BaseScene* nextScene) {
    nextScene_ = nextScene;
     if (nextScene_) {
        nextScene_->SetSceneManager(this);
    }
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
    if (overlayScene_) {
        overlayScene_->Update();
    }
}

void SceneManager::Draw() {
    // 调用当前场景的绘制
   if (overlayScene_) {
        overlayScene_->Draw();
    } else if (scene_) {
        // 没有覆盖场景时，才绘制当前场景
        scene_->Draw();
    }
}
