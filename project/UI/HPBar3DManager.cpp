#include "HPBar3DManager.h"
#include "ModelManager.h"
#include <cmath>

// 声明 GameScene.cpp 里已有的全局函数（链接时会找到实现）
Vector3 ScreenToWorld(float screenX, float screenY, float ndcZ, Camera* camera);

void HPBar3DManager::Initialize(Object3dCommon* objCommon,
                                Camera* camera,
                                Player* player,
                                float hpNdcZ)
{
    object3dCommon_ = objCommon;
    camera_         = camera;
    player_         = player;
    hpNdcZ_         = hpNdcZ;

    strips_.clear();
    strips_.reserve(segments_);

    for (int i = 0; i < segments_; ++i) {
        Object3d* seg = new Object3d();
        seg->Initialize(object3dCommon_);
        seg->SetModel("hurd/hurd.obj");
        seg->SetCamera(camera_);
        seg->SetScale({ 0.001f, 0.001f, 0.001f });   // 和你原来的一样
        seg->SetEnableLighting(true);
        seg->SetDirectionalLightIntensity(2.0f);
        strips_.push_back(seg);
    }

    visibleCount_ = segments_;
}

void HPBar3DManager::Finalize()
{
    for (auto* seg : strips_) {
        delete seg;
    }
    strips_.clear();
}

Vector3 HPBar3DManager::ScreenToWorld_(float sx, float sy, float ndcZ)
{
    // 直接调用 GameScene 提供的全局函数
    return ScreenToWorld(sx, sy, ndcZ, camera_);
}

void HPBar3DManager::Update(float dt)
{
    (void)dt; // 目前没用到时间

    if (!camera_ || strips_.empty()) return;

    // 1) 根据玩家当前 HP 比例计算可见段数
    float hpRatio = 1.0f;
    if (player_) {
        hpRatio = player_->GetHpRatio();
    }
    visibleCount_ = (int)std::ceil(hpRatio * (float)segments_);
    if (visibleCount_ < 0)          visibleCount_ = 0;
    if (visibleCount_ > segments_)  visibleCount_ = segments_;

    // 2) 计算屏幕起点
    const float pad  = 16.0f;
    float baseX = pad + insetX_;
    float baseY = pad + insetY_;

    // 3) 为每一段计算世界坐标并更新
    for (int i = 0; i < segments_; ++i) {
        float sx = baseX + i * (segPixelW_ + gapPixel_);
        float sy = baseY;

        Vector3 world = ScreenToWorld_(sx, sy, hpNdcZ_);

        strips_[i]->SetTranslate(world);
        strips_[i]->Update();
    }
}

void HPBar3DManager::Draw3D()
{
    // 只画可见段数
    for (int i = 0; i < visibleCount_; ++i) {
        if (strips_[i]) {
            strips_[i]->Draw();
        }
    }
}
