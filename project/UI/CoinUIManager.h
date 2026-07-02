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

    void Initialize(MyEngine::SpriteCommon* spriteCommon,
                    MyEngine::Object3dCommon* object3dCommon,
                    MyEngine::Camera* camera,
                    float hpNdcZ);

    void Finalize();

    // 現在のマップに残っているコイン数を設定する（0～999）。UI は自動で更新される
    void SetRemainingCoin(int remaining);
    int  GetRemainingCoin() const { return remainingCoin_; }

    // 毎フレーム更新（ライト点滅用のタイマー）
    void Update(float dt);

    // GameScene::Draw() 内で呼び出す
    void Draw3D();  // 3D coin モデル
    void Draw2D();  // コロン + 数字

private:
    MyEngine::SpriteCommon*   spriteCommon_   = nullptr;
    MyEngine::Object3dCommon* object3dCommon_ = nullptr;
    MyEngine::Camera*         camera_         = nullptr;
    float           hpNdcZ_         = 0.08f;

    // 3D coin モデル
    std::unique_ptr<MyEngine::Object3d> coinObj_;
    std::unique_ptr<MyEngine::Sprite>   colonSprite_;
    std::unique_ptr<MyEngine::Sprite>   digitSprites_[3] = {
        nullptr, nullptr, nullptr
    };

    int   remainingCoin_ = 0;
    int   lastCoin_      = -1;
    float lightTime_ = 0.0f;

    // 右上の数字 UI を更新する（旧 UpdateCoinCountUI_）
    void UpdateDigits_();
};
