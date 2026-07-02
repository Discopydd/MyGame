#include "PortalManager.h"

#include <cmath>

using namespace MyEngine;
void PortalManager::Initialize(SpriteCommon* spriteCommon, Camera* camera)
{
    spriteCommon_ = spriteCommon;
    camera_       = camera;

    hintSprite_ = std::make_unique<Sprite>();
    hintSprite_->Initialize(spriteCommon_, "Resources/letterE.png");
    hintSprite_->SetPosition({ 0.0f, 0.0f });
    hintSprite_->SetSize({ 32.0f, 32.0f });
    hintSprite_->SetVisible(false);

    lockSprite_ = std::make_unique<Sprite>();
    lockSprite_->Initialize(spriteCommon_, "Resources/portal_lock.png");
    lockSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    lockSprite_->SetPosition({ 0.0f, 0.0f });
    lockSprite_->SetSize({ 30.0f, 30.0f });
    lockSprite_->SetVisible(false);

    showLockIcons_ = false;
    lockPulseTimer_ = 0.0f;
    unlockBurstActive_ = false;
    unlockBurstTimer_ = 0.0f;
    portals_.clear();
}

void PortalManager::Finalize()
{
    hintSprite_.reset();
    lockSprite_.reset();
    portals_.clear();
    showLockIcons_ = false;
    lockPulseTimer_ = 0.0f;
    unlockBurstActive_ = false;
    unlockBurstTimer_ = 0.0f;
}

void PortalManager::ClearPortals()
{
    portals_.clear();
    showLockIcons_ = false;
    unlockBurstActive_ = false;
    unlockBurstTimer_ = 0.0f;
}

void PortalManager::AddPortal(const MapChipField::IndexSet& idx,
                              const std::string& targetMap,
                              const Vector3& startPos,
                              const Vector3& worldPos)
{
    PortalInfo p;
    p.index          = idx;
    p.targetMap      = targetMap;
    p.targetStartPos = startPos;
    p.worldPos       = worldPos;
    portals_.push_back(p);
}

const PortalInfo* PortalManager::GetPortalAt(const MapChipField::IndexSet& playerIndex) const
{
    for (const auto& p : portals_) {
        if (p.index.xIndex == playerIndex.xIndex &&
            p.index.yIndex == playerIndex.yIndex) {
            return &p;
        }
    }
    return nullptr;
}

void PortalManager::UpdateHint(const MapChipField::IndexSet& playerIndex,
                               const Vector3& playerWorldPos,
                               bool canControl)
{
    if (!hintSprite_ || !camera_) return;

    const PortalInfo* p = GetPortalAt(playerIndex);
    if (p && canControl) {
        // ヒント表示位置: プレイヤーの少し上。元のコードと同じ
        Vector3 pos = playerWorldPos;
        pos.x -= 0.25f;
        pos.y += 2.0f;
        Vector3 screenPos = WorldToScreen(pos, camera_);
        hintSprite_->SetPosition({ screenPos.x, screenPos.y });
        hintSprite_->SetVisible(true);
    } else {
        hintSprite_->SetVisible(false);
    }

    hintSprite_->Update();
}

void PortalManager::SetLockVisible(bool visible)
{
    if (showLockIcons_ && !visible) {
        unlockBurstActive_ = true;
        unlockBurstTimer_ = 0.0f;
    }

    showLockIcons_ = visible;
    if (!visible && lockSprite_) {
        lockSprite_->SetVisible(false);
        lockSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
        lockSprite_->Update();
    }
}

void PortalManager::DrawHint()
{
    if (hintSprite_ && hintSprite_->IsVisible()) {
        hintSprite_->Draw();
    }
}

void PortalManager::DrawLockIcons()
{
    if (!lockSprite_ || !camera_) {
        return;
    }

    constexpr float kBaseLockSize = 44.0f;
    constexpr float kLockWorldYOffset = 0.2f;
    constexpr float kUnlockBurstDuration = 0.22f;

    if (showLockIcons_) {
        lockPulseTimer_ += 0.05f;
        const float pulse = 1.0f + 0.08f * std::sin(lockPulseTimer_);

        for (const auto& p : portals_) {
            Vector3 iconWorldPos = p.worldPos;
            iconWorldPos.y += kLockWorldYOffset;
            Vector3 screenPos = WorldToScreen(iconWorldPos, camera_);
            if (screenPos.z < 0.0f || screenPos.z > 1.0f) {
                continue;
            }

            lockSprite_->SetPosition({ screenPos.x, screenPos.y });
            lockSprite_->SetSize({ kBaseLockSize * pulse, kBaseLockSize * pulse });
            lockSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
            lockSprite_->SetVisible(true);
            lockSprite_->Update();
            lockSprite_->Draw();
        }
    }

    if (unlockBurstActive_) {
        unlockBurstTimer_ += 1.0f / 60.0f;
        float t = unlockBurstTimer_ / kUnlockBurstDuration;
        if (t >= 1.0f) {
            unlockBurstActive_ = false;
            unlockBurstTimer_ = 0.0f;
        } else {
            const float burstScale = 1.0f + 0.75f * t;
            const float alpha = 1.0f - t;
            const float rotation = t * 0.35f;
            for (const auto& p : portals_) {
                Vector3 iconWorldPos = p.worldPos;
                iconWorldPos.y += kLockWorldYOffset;
                Vector3 screenPos = WorldToScreen(iconWorldPos, camera_);
                if (screenPos.z < 0.0f || screenPos.z > 1.0f) {
                    continue;
                }

                lockSprite_->SetPosition({ screenPos.x, screenPos.y });
                lockSprite_->SetSize({ kBaseLockSize * burstScale, kBaseLockSize * burstScale });
                lockSprite_->SetRotation(rotation);
                lockSprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha });
                lockSprite_->SetVisible(true);
                lockSprite_->Update();
                lockSprite_->Draw();
            }
        }
    }

    lockSprite_->SetRotation(0.0f);
    lockSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    lockSprite_->SetVisible(false);
}
