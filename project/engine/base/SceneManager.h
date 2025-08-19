#pragma once
#include "BaseScene.h"

class SceneManager {
public:

    // 析构函数
    ~SceneManager();

    // 设置（预约）下一个场景
    void SetNextScene(BaseScene* nextScene);


    // 设置临时场景（如加载场景）
    void SetOverlayScene(BaseScene* overlayScene);
    void ClearOverlayScene();
    // 新增：获取当前覆盖场景（OverlayScene）
    BaseScene* GetOverlayScene() const { return overlayScene_; }

    // 更新处理
    void Update();

    // 绘制处理
    void Draw();

private:
    // 当前执行中的场景
    BaseScene* scene_ = nullptr;

    // 准备切换的下一个场景
    BaseScene* nextScene_ = nullptr;
    // 覆盖场景（如加载场景）
    BaseScene* overlayScene_ = nullptr;
};
