#include "SceneManager.h"

SceneManager::~SceneManager() {
    // 最后一个场景的终止和释放
    if (scene_) {
        scene_->Finalize();
    }
    if (overlayScene_) {
        overlayScene_->Finalize();
    }
}
void SceneManager::SetNextScene(std::unique_ptr<BaseScene> nextScene) {
    // ここではまだ Initialize せず、Update() で切り替える
    nextScene_ = std::move(nextScene);
}

void SceneManager::SetOverlayScene(std::unique_ptr<BaseScene> overlayScene) {
    // 既存の overlay を終了
    if (overlayScene_) {
        overlayScene_->Finalize();
    }

    overlayScene_ = std::move(overlayScene);

    if (overlayScene_) {
        overlayScene_->SetSceneManager(this);
        overlayScene_->Initialize();
    }
}

void SceneManager::ClearOverlayScene() {
    if (overlayScene_) {
        overlayScene_->Finalize();
        overlayScene_.reset();
    }
}

void SceneManager::Update() {
    // 切换场景（如果有预约）
    if (nextScene_) {
        // 当前场景の终止
        if (scene_) {
            scene_->Finalize();
        }

        // 切换到新场景
        scene_ = std::move(nextScene_);
        if (scene_) {
            scene_->SetSceneManager(this);
            scene_->Initialize();
        }
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