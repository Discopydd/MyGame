#pragma once
namespace MyEngine {

class SceneManager;
// シーンの基底クラス
/// <summary>
/// 各ゲームシーンに共通する初期化、更新、描画、終了処理のインターフェースを定義する基底クラス。
/// </summary>
class BaseScene {
public:
    virtual ~BaseScene() = default;

    virtual void Initialize() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual void Finalize() = 0;

    virtual void UpdateInitialization() {}
    virtual bool IsInitializationComplete() const { return true; }

    virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }
    SceneManager* GetSceneManager() { return sceneManager_; }
protected:
    SceneManager* sceneManager_ = nullptr;
};


} // namespace MyEngine
