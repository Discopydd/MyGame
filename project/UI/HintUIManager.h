#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Camera.h"
#include "MyMath.h"
#include <vector>

// GameScene 里已经有的结构体，可以直接搬到这里也用这个定义
struct HintSprite {
    Sprite* sprite = nullptr;
    Vector3 worldPos{};
};

// GameScene 里实现的 WorldToScreen，我们这里只声明一下
Vector3 WorldToScreen(const Vector3& worldPos, Camera* camera);

class HintUIManager {
public:
    HintUIManager() = default;
    ~HintUIManager() = default;

    void Initialize(SpriteCommon* spriteCommon, Camera* camera);
    void Finalize();

    // 让管理器知道 GameScene 里那几个 HintSprite / 容器
    void SetSpaceHint(HintSprite* spaceHint)   { spaceHint_ = spaceHint; }
    void SetShiftHint(HintSprite* shiftHint)   { shiftHint_ = shiftHint; }
    void SetSprintHint(HintSprite* sprintHint) { sprintHint_ = sprintHint; }
    void SetUpHints(std::vector<HintSprite>* upHints) { upHints_ = upHints; }

    // 每帧更新：计算上下浮动 + 用世界坐标算屏幕坐标
    void Update(float dt);

    // 在 GameScene::Draw() 的“中间层 Sprite”里调用
    void Draw();

private:
    SpriteCommon* spriteCommon_ = nullptr;
    Camera*       camera_       = nullptr;

    HintSprite* spaceHint_  = nullptr;
    HintSprite* shiftHint_  = nullptr;
    HintSprite* sprintHint_ = nullptr;
    std::vector<HintSprite>* upHints_ = nullptr;

    // 上下浮动用参数（原来 GameScene 里的那三个）
    float bobTime_      = 0.0f;
    float bobAmplitude_ = 6.0f;   // 位移像素（上下±6）
    float bobSpeed_     = 3.0f;   // 频率（越大晃得越快）
};
