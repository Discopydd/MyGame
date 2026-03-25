#pragma once
class SceneManager;
// シーンの基底クラス
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
