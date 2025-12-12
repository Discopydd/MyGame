#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "Camera.h"
#include "WinApp.h"
#include <memory>
class CoinUIManager {
public:
    CoinUIManager() = default;
    ~CoinUIManager() = default;

    void Initialize(SpriteCommon* spriteCommon,
                    Object3dCommon* object3dCommon,
                    Camera* camera,
                    float hpNdcZ);

    void Finalize();

    // 设置当前总金币数（0~999），会自动刷新 UI
    void SetTotalCoin(int total);
    int  GetTotalCoin() const { return totalCoin_; }

    // 每帧更新（灯光闪烁计时）
    void Update(float dt);

    // 在 GameScene::Draw() 里调用
    void Draw3D();  // 3D coin 模型
    void Draw2D();  // 冒号 + 数字

private:
    SpriteCommon*   spriteCommon_   = nullptr;
    Object3dCommon* object3dCommon_ = nullptr;
    Camera*         camera_         = nullptr;
    float           hpNdcZ_         = 0.08f;

    // 3D coin 模型
    std::unique_ptr<Object3d> coinObj_;
    std::unique_ptr<Sprite>   colonSprite_;
    std::unique_ptr<Sprite>   digitSprites_[3] = {
        nullptr, nullptr, nullptr
    };

    int   totalCoin_ = 0;
    int   lastCoin_  = -1;
    float lightTime_ = 0.0f;

    // 刷新右上角数字 UI（原来的 UpdateCoinCountUI_）
    void UpdateDigits_();
};
