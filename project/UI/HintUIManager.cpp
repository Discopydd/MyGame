// HintUIManager.cpp
#include "HintUIManager.h"
#include <cmath>

void HintUIManager::Initialize(SpriteCommon* spriteCommon, Camera* camera)
{
    spriteCommon_ = spriteCommon;
    camera_       = camera;

    bobTime_      = 0.0f;
    bobAmplitude_ = 6.0f;
    bobSpeed_     = 3.0f;
}

void HintUIManager::Finalize()
{
    // 注意：这里不 delete 任何 Sprite
    // Sprite 的生命周期还是由 GameScene 管理
}

void HintUIManager::Update(float dt)
{
    if (!camera_) return;

    bobTime_ += dt;
    float offset = std::sinf(bobTime_ * bobSpeed_) * bobAmplitude_;

    // Space
    if (spaceHint_ && spaceHint_->sprite) {
        Vector3 s = WorldToScreen(spaceHint_->worldPos, camera_);
        spaceHint_->sprite->SetPosition({ s.x, s.y + offset });
        spaceHint_->sprite->Update();
    }

    // Up 一组
    if (upHints_) {
        for (auto& h : *upHints_) {
            if (!h.sprite) continue;
            Vector3 s = WorldToScreen(h.worldPos, camera_);
            h.sprite->SetPosition({ s.x, s.y + offset });
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
    // 这里默认 GameScene 已经调用过 spriteCommon_->CommonDraw()

    if (spaceHint_ && spaceHint_->sprite) {
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
