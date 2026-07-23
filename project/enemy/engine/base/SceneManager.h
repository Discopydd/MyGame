#pragma once
#include "BaseScene.h"
#include <memory>

namespace MyEngine {

/// <summary>
/// 現在のシーンを保持し、シーンの切り替え、更新、描画を管理するクラス。
/// </summary>
class SceneManager {
public:

    // デストラクタ
    ~SceneManager();

    // 次のシーンを設定（予約）する
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
    // 現在実行中のシーン（所有権あり）
    std::unique_ptr<BaseScene> scene_;

    // 切り替え準備中の次シーン（まだ Initialize 前）
    std::unique_ptr<BaseScene> nextScene_;

    // オーバーレイシーン（ローディングシーンなど）
    std::unique_ptr<BaseScene> overlayScene_;
};


} // namespace MyEngine
