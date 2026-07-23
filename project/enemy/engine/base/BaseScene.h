#pragma once
namespace MyEngine {

class SceneManager;
// シーンの基底クラス
/// <summary>
/// 各ゲームシーンに共通する初期化、更新、描画、終了処理のインターフェースを定義する基底クラス。
/// </summary>
class BaseScene {
public:
    /// <summary>
    /// BaseSceneが保持するリソースを破棄します。
    /// </summary>
    virtual ~BaseScene() = default;

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    virtual void Initialize() = 0;
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    virtual void Update() = 0;
    /// <summary>
    /// 現在の状態を画面へ描画します。
    /// </summary>
    virtual void Draw() = 0;
    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    virtual void Finalize() = 0;

    /// <summary>
    /// 負荷を分散するための遅延初期化処理を1段階進めます。
    /// </summary>
    virtual void UpdateInitialization() {}
    /// <summary>
    /// シーンの遅延初期化が完了しているかを判定します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    virtual bool IsInitializationComplete() const { return true; }

    /// <summary>
    /// Scene Managerを設定します。
    /// </summary>
    /// <param name="sceneManager">処理対象のオブジェクトへのポインタ。</param>
    virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }
    /// <summary>
    /// Scene Managerを取得します。
    /// </summary>
    /// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
    SceneManager* GetSceneManager() { return sceneManager_; }
protected:
    SceneManager* sceneManager_ = nullptr;
};


} // namespace MyEngine
