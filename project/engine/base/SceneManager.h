#pragma once
#include "BaseScene.h"
#include <memory>

class SceneManager {
public:

    // 析构函数
    ~SceneManager();

    // 设置（预约）下一个场景
    void SetNextScene(std::unique_ptr<BaseScene> nextScene);


    // 设置临时场景（如加载场景）
    void SetOverlayScene(std::unique_ptr<BaseScene> overlayScene);
    void ClearOverlayScene();
    // 新增：获取当前覆盖场景（OverlayScene）
    BaseScene* GetOverlayScene() const { return overlayScene_.get(); }

    // 更新处理
    void Update();

    // 绘制处理
    void Draw();

private:
    // 当前执行中的场景（所有権あり）
    std::unique_ptr<BaseScene> scene_;

    // 准备切换的下一个场景（まだ Initialize 前）
    std::unique_ptr<BaseScene> nextScene_;

    // 覆盖场景（如加载场景）
    std::unique_ptr<BaseScene> overlayScene_;
};
