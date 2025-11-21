#pragma once
#include <vector>
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "ParticleEmitter.h"

// 统一管理所有粒子发射器
class ParticleManager
{
public:
    ParticleManager() = default;
    ~ParticleManager();   // 析构里自动释放

    // 初始化：把 3D / 2D 的共通对象传进来
    void Initialize(Object3dCommon* objectCommon, SpriteCommon* spriteCommon);

    // 创建一个新的发射器，由管理器托管生命周期
    ParticleEmitter* CreateEmitter();

    // 每帧更新所有发射器
    void Update(float dt);

    // 3D 粒子（模型粒子）绘制
    void Draw3D();

    // 2D 粒子（Sprite 粒子）绘制
    void Draw2D();

    // 手动清空所有发射器（可选，在切场景时也可以显式调用）
    void Finalize();

private:
    Object3dCommon* objCommon_ = nullptr;
    SpriteCommon* sprCommon_   = nullptr;
    std::vector<ParticleEmitter*> emitters_;
};
