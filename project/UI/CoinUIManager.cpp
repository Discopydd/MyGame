#include "CoinUIManager.h"
#include <cmath>

using namespace MyEngine;
Vector3 ScreenToWorld(float screenX, float screenY, float ndcZ, Camera* camera);

void CoinUIManager::Initialize(SpriteCommon* spriteCommon,
                               Object3dCommon* object3dCommon,
                               Camera* camera,
                               float hpNdcZ)
{
    spriteCommon_   = spriteCommon;
    object3dCommon_ = object3dCommon;
    camera_         = camera;
    hpNdcZ_         = hpNdcZ;

    // ==== 右上 Coin UI（3D モデル） ====
    coinObj_ = std::make_unique<Object3d>();
    coinObj_->Initialize(object3dCommon_);
    coinObj_->SetModel("coin_ui/coin_ui.obj");
    coinObj_->SetCamera(camera_);
    coinObj_->SetScale({ 0.0025f, 0.0025f, 0.0025f });
    coinObj_->SetEnableLighting(true);

    // ==== Coin 数字 UI (Sprite) ====
    colonSprite_ = std::make_unique<Sprite>();
    colonSprite_->Initialize(spriteCommon_, "Resources/numbers/colon.png");
    colonSprite_->SetSize({ 24.0f, 24.0f });

    // 最初はすべて 0.png で初期化し、その後 UpdateDigits_ 内でテクスチャを切り替える
    for (int i = 0; i < 3; ++i) {
        digitSprites_[i] = std::make_unique<Sprite>();
        digitSprites_[i]->Initialize(spriteCommon_, "Resources/numbers/0.png");
        digitSprites_[i]->SetSize({ 24.0f, 24.0f });
    }

    remainingCoin_ = 0;
    lastCoin_      = -1;
    UpdateDigits_();
}

void CoinUIManager::Finalize()
{
    colonSprite_.reset();
    for (int i = 0; i < 3; ++i) {
        digitSprites_[i].reset();
    }
    coinObj_.reset();
}

void CoinUIManager::SetRemainingCoin(int remaining)
{
    if (remaining < 0)   remaining = 0;
    if (remaining > 999) remaining = 999;
    remainingCoin_ = remaining;
    if (remainingCoin_ != lastCoin_) {
        UpdateDigits_();
    }
}

void CoinUIManager::Update(float dt)
{
    lightTime_ += dt;
}

void CoinUIManager::Draw3D()
{
    if (!coinObj_ || !camera_) { return; }

    const float uiPad    = 20.0f;
    const float digitW   = 24.0f;
    const float digitGap = 2.0f;

    // UpdateDigits_ と同じ計算式を使い、3 桁分の数字スペースを確保する
    float colonX = (float)WinApp::kClientWidth - uiPad - 3.0f * (digitW + digitGap);
    float colonY = uiPad + 10.0f;

    // coin はコロンの少し左側に配置する
    float coinScreenX = colonX - 20.0f;
    float coinScreenY = colonY + 12.0f;

    Vector3 coinWorld = ScreenToWorld(coinScreenX, coinScreenY, hpNdcZ_, camera_);
    coinObj_->SetTranslate(coinWorld);

    // ライトを点滅させる
    const float baseI  = 0.6f;
    const float ampI   = 1.4f;
    const float speedI = 6.0f;
    float wave = (std::sinf(lightTime_ * speedI) + 1.0f) * 0.5f;
    float dirIntensity = baseI + ampI * wave;

    coinObj_->SetDirectionalLightIntensity(dirIntensity);
    coinObj_->Update();
    coinObj_->Draw();
}

void CoinUIManager::Draw2D()
{
    if (!spriteCommon_) { return; }


    if (colonSprite_) {
        colonSprite_->Draw();
    }
    for (int i = 0; i < 3; ++i) {
        if (digitSprites_[i] && digitSprites_[i]->IsVisible()) {
            digitSprites_[i]->Draw();
        }
    }
}

void CoinUIManager::UpdateDigits_()
{
    if (!colonSprite_) { return; }

    const float uiPad    = 20.0f;
    const float digitW   = 24.0f;
    const float digitH   = 24.0f;
    const float digitGap = 2.0f;

    // 3 桁分の数字スペースを確保し、右上寄りに配置する
    float colonX = (float)WinApp::kClientWidth  - uiPad - 3.0f * (digitW + digitGap);
    float colonY = uiPad + 10.0f;

    // コロン位置
    colonSprite_->SetPosition({ colonX, colonY });
    colonSprite_->SetVisible(true);
    colonSprite_->Update();

    // 数字（整数値は 0～999 に制限）
    int c = remainingCoin_;
    if (c < 0)   c = 0;
    if (c > 999) c = 999;

    int d0 = c / 100;
    int d1 = (c / 10) % 10;
    int d2 = c % 10;

    int digits[3] = { d0, d1, d2 };
    int numDigits = (c >= 100) ? 3 : (c >= 10 ? 2 : 1);

    // いったんすべて非表示にする
    for (int i = 0; i < 3; ++i) {
        if (digitSprites_[i]) {
            digitSprites_[i]->SetVisible(false);
        }
    }

    // 数字テクスチャのパス
    const char* digitTex[10] = {
        "Resources/numbers/0.png","Resources/numbers/1.png","Resources/numbers/2.png",
        "Resources/numbers/3.png","Resources/numbers/4.png","Resources/numbers/5.png",
        "Resources/numbers/6.png","Resources/numbers/7.png","Resources/numbers/8.png",
        "Resources/numbers/9.png",
    };

    // 数字は右寄せで表示する（1 の位を右側に配置）
    float x = colonX + digitW + digitGap;
    float y = colonY;

    int startIndex = 3 - numDigits;
    int spriteIdx = 0;

    for (int i = startIndex; i < 3; ++i) {
        int d = digits[i];
        if (!digitSprites_[spriteIdx]) {
            ++spriteIdx;
            continue;
        }

        digitSprites_[spriteIdx]->Initialize(spriteCommon_, digitTex[d]);
        digitSprites_[spriteIdx]->SetSize({ digitW, digitH });
        digitSprites_[spriteIdx]->SetPosition({ x, y });
        digitSprites_[spriteIdx]->SetVisible(true);
        digitSprites_[spriteIdx]->Update();

        x += digitW + digitGap;
        ++spriteIdx;
    }

    lastCoin_ = c;
}
