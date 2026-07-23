#include "HintUIManager.h"
#include <cmath>

using namespace MyEngine;
void HintUIManager::Initialize(SpriteCommon* spriteCommon, Camera* camera)
{
    spriteCommon_ = spriteCommon;
    camera_       = camera;

    bobTime_      = 0.0f;
    bobAmplitude_ = 6.0f;
    bobSpeed_     = 3.0f;

    moveKeyA_ = std::make_unique<Sprite>();
    moveKeyA_->Initialize(spriteCommon_, "Resources/key_A.png");
    moveKeyA_->SetSize({ 32.0f, 32.0f });

    moveKeyD_ = std::make_unique<Sprite>();
    moveKeyD_->Initialize(spriteCommon_, "Resources/key_D.png");
    moveKeyD_->SetSize({ 32.0f, 32.0f });

    moveArrowL_ = std::make_unique<Sprite>();
    moveArrowL_->Initialize(spriteCommon_, "Resources/arrow_left.png");
    moveArrowL_->SetSize({ 32.0f, 32.0f });

    moveArrowR_ = std::make_unique<Sprite>();
    moveArrowR_->Initialize(spriteCommon_, "Resources/arrow_right.png");
    moveArrowR_->SetSize({ 32.0f, 32.0f });
}

void HintUIManager::Finalize()
{

}

void HintUIManager::Update(float dt)
{
    if (!camera_) return;

    bobTime_ += dt;
    float offset = std::sinf(bobTime_ * bobSpeed_) * bobAmplitude_;

    // Space
    if (spaceHint_ && spaceHint_->sprite) {
        Vector3 s = WorldToScreen(spaceHint_->worldPos, camera_);
        spaceHint_->sprite->SetPosition({ s.x, s.y + offset + 8.0f });
        spaceHint_->sprite->Update();
    }
    if (spaceHint_ && spaceHint_->sprite && moveKeyA_ && moveKeyD_ && moveArrowL_ && moveArrowR_) {
        Vector3 s = WorldToScreen(spaceHint_->worldPos, camera_);

        constexpr float kSpaceSize     = 64.0f;
        constexpr float kIconSize      = 48.0f;
        constexpr float kGap           = 6.0f;   // between icons
        constexpr float kPadFromSpace  = 10.0f;  // distance from Space hint (pixels)

        float y = s.y + offset + (kSpaceSize - kIconSize) * 0.5f;

        // Layout: [A][←][D][→]
        float totalW = kIconSize * 4.0f + kGap * 3.0f;
        float x = s.x - kPadFromSpace - totalW;

        moveKeyA_->SetPosition({ x, y });
        moveKeyA_->Update();
        x += kIconSize + kGap;

        moveArrowL_->SetPosition({ x, y });
        moveArrowL_->Update();
        x += kIconSize + kGap;

        moveKeyD_->SetPosition({ x, y });
        moveKeyD_->Update();
        x += kIconSize + kGap;

        moveArrowR_->SetPosition({ x, y });
        moveArrowR_->Update();
    }


    // Up ヒント一式
    if (upHints_) {
        for (auto& h : *upHints_) {
            if (!h.sprite) continue;
            Vector3 s = WorldToScreen(h.worldPos, camera_);
            h.sprite->SetPosition({ s.x, s.y + offset - 8.0f });
            h.sprite->Update();
        }
    }

    // Shift
    if (shiftHint_ && shiftHint_->sprite) {
        Vector3 s = WorldToScreen(shiftHint_->worldPos, camera_);
        shiftHint_->sprite->SetPosition({ s.x, s.y + offset });
        shiftHint_->sprite->Update();
    }

    // Sprint
    if (sprintHint_ && sprintHint_->sprite) {
        Vector3 s = WorldToScreen(sprintHint_->worldPos, camera_);
        sprintHint_->sprite->SetPosition({ s.x, s.y + offset });
        sprintHint_->sprite->Update();
    }
}

void HintUIManager::Draw()
{
    // ここではデフォルト GameScene すでに呼び出している spriteCommon_->CommonDraw()

    if (spaceHint_ && spaceHint_->sprite) {
        // Move hint icons placed left of Space hint
        if (moveKeyA_ && moveKeyD_ && moveArrowL_ && moveArrowR_) {
            moveKeyA_->Draw();
            moveArrowL_->Draw();
            moveKeyD_->Draw();
            moveArrowR_->Draw();
        }
        spaceHint_->sprite->Draw();
    }

    if (upHints_) {
        for (auto& h : *upHints_) {
            if (h.sprite) {
                h.sprite->Draw();
            }
        }
    }

    if (shiftHint_ && shiftHint_->sprite) {
        shiftHint_->sprite->Draw();
    }

    if (sprintHint_ && sprintHint_->sprite) {
        sprintHint_->sprite->Draw();
    }
}
