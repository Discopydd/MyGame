#pragma once
#include "BaseScene.h"
#include <memory>

namespace MyEngine {

class SceneManager {
public:

    // 析构函数
    ~SceneManager();

    // 設定（予约）下一个场景
    void SetNextScene(std::unique_ptr<BaseScene> nextScene);


    // 一時シーンを設定（ローディングシーンなど）
    void SetOverlayScene(std::unique_ptr<BaseScene> overlayScene);
    void ClearOverlayScene();
    // 追加: 現在のオーバーレイシーンを取得（OverlayScene）
    BaseScene* GetOverlayScene() const { return overlayScene_.get(); }

    // 更新処理
    void Update();

    // 描画処理
    void Draw();

private:
    // 現在执行中的场景（すべて権あり）
    std::unique_ptr<BaseScene> scene_;

    // 准备切换的下一个场景（まだ Initialize 前）
    std::unique_ptr<BaseScene> nextScene_;

    // オーバーレイシーン（ローディングシーンなど）
    std::unique_ptr<BaseScene> overlayScene_;
};


} // namespace MyEngine
