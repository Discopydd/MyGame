#include "HPBar3DManager.h"
#include "ModelManager.h"
#include <cmath>

// GameScene.cpp に既存のグローバル関数を宣言する（リンク時に実装が見つかる）
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
        auto seg = std::make_unique<Object3d>();
        seg->Initialize(object3dCommon_);
        seg->SetModel("hurd/hurd.obj");
        seg->SetCamera(camera_);
        seg->SetScale({ 0.001f, 0.001f, 0.001f });
        seg->SetEnableLighting(true);
        seg->SetDirectionalLightIntensity(2.0f);
        strips_.push_back(std::move(seg));
    }

    visibleCount_ = segments_;
}

void HPBar3DManager::Finalize()
{
    strips_.clear();
}

Vector3 HPBar3DManager::ScreenToWorld_(float sx, float sy, float ndcZ)
{
    // GameScene が提供するグローバル関数を直接呼び出す
    return ScreenToWorld(sx, sy, ndcZ, camera_);
}

void HPBar3DManager::Update(float dt)
{
    (void)dt; // 現状では時間を使用しない

    if (!camera_ || strips_.empty()) return;

    // 1) プレイヤーの現在 HP 比率から可視段数を計算
    float hpRatio = 1.0f;
    if (player_) {
        hpRatio = player_->GetHpRatio();
    }
    visibleCount_ = (int)std::ceil(hpRatio * (float)segments_);
    if (visibleCount_ < 0)          visibleCount_ = 0;
    if (visibleCount_ > segments_)  visibleCount_ = segments_;

    // 2) 画面上の開始位置を計算
    const float pad  = 16.0f;
    float baseX = pad + insetX_;
    float baseY = pad + insetY_;

    // 3) 各区間ごとにワールド座標を計算して更新する
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
    // 可視段数だけ描画
    for (int i = 0; i < visibleCount_; ++i) {
        if (strips_[i]) {
            strips_[i]->Draw();
        }
    }
}
