#include "PortalManager.h"

void PortalManager::Initialize(SpriteCommon* spriteCommon, Camera* camera)
{

    if (hintSprite_) {
        delete hintSprite_;
        hintSprite_ = nullptr;
    }
    portals_.clear();
    spriteCommon_ = spriteCommon;
    camera_       = camera;

    hintSprite_ = new Sprite();
    hintSprite_->Initialize(spriteCommon_, "Resources/letterE.png");
    hintSprite_->SetPosition({ 0.0f, 0.0f });
    hintSprite_->SetSize({ 32.0f, 32.0f });
    hintSprite_->SetVisible(false); 

}

void PortalManager::Finalize()
{
    if (hintSprite_) {
        delete hintSprite_;
        hintSprite_ = nullptr;
    }
    portals_.clear();
}

void PortalManager::ClearPortals()
{
    portals_.clear();
}

void PortalManager::AddPortal(const MapChipField::IndexSet& idx,
                              const std::string& targetMap,
                              const Vector3& startPos)
{
    PortalInfo p;
    p.index         = idx;
    p.targetMap     = targetMap;
    p.targetStartPos= startPos;
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
        // 提示位置：玩家头上稍微偏一下，和你原来代码一致
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

void PortalManager::DrawHint()
{
    if (hintSprite_ && hintSprite_->IsVisible()) {
        hintSprite_->Draw();
    }
}
