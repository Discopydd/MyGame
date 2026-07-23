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
    /// <summary>
    /// SceneManagerが保持するリソースを破棄します。
    /// </summary>
    ~SceneManager();

    // 次のシーンを設定（予約）する
    /// <summary>
    /// Next Sceneを設定します。
    /// </summary>
    /// <param name="nextScene">処理に使用するnextSceneの値。</param>
    void SetNextScene(std::unique_ptr<BaseScene> nextScene);


    // 一時シーンを設定（ローディングシーンなど）
    /// <summary>
    /// Overlay Sceneを設定します。
    /// </summary>
    /// <param name="overlayScene">処理に使用するoverlaySceneの値。</param>
    void SetOverlayScene(std::unique_ptr<BaseScene> overlayScene);
    /// <summary>
    /// Overlay Sceneをクリアします。
    /// </summary>
    void ClearOverlayScene();
    // 追加: 現在のオーバーレイシーンを取得（OverlayScene）
    /// <summary>
    /// Overlay Sceneを取得します。
    /// </summary>
    /// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
    BaseScene* GetOverlayScene() const { return overlayScene_.get(); }

    // 更新処理
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    void Update();

    // 描画処理
    /// <summary>
    /// 現在の状態を画面へ描画します。
    /// </summary>
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
