#include "SceneManager.h"

SceneManager::~SceneManager() {
    // 最後一个场景的終止和放つ
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
    // シーン切替（予約がある場合）
    if (nextScene_) {
        // 現在のシーンの終止
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

    // 初期化未完了のシーンは通常 Update ではなく、分割初期化を進める
    if (scene_) {
        if (!scene_->IsInitializationComplete()) {
            scene_->UpdateInitialization();
        } else {
            scene_->Update();
        }
    }
    if (overlayScene_) {
        overlayScene_->Update();
    }
}

void SceneManager::Draw() {
    // 現在のシーンの描画を呼ぶ
    if (overlayScene_) {
        overlayScene_->Draw();
    } else if (scene_ && scene_->IsInitializationComplete()) {
        // オーバーレイシーンがない時だけ現在のシーンを描画
        scene_->Draw();
    }
}