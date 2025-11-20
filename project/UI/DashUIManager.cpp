#include "DashUIManager.h"

void DashUIManager::Initialize(SpriteCommon* spriteCommon, Player* player)
{
    spriteCommon_ = spriteCommon;
    player_       = player;

    const char* textureFilePath[] = { "Resources/skill_icon.png", "Resources/gray.png" };

    icon_ = new Sprite();
    icon_->Initialize(spriteCommon_, textureFilePath[0]);
    icon_->SetPosition({ 40.0f, 80.0f });
    icon_->SetSize({ 32.0f, 32.0f });

    overlay_ = new Sprite();
    overlay_->Initialize(spriteCommon_, textureFilePath[1]);
    overlay_->SetPosition({ 40.0f, 80.0f });
    overlay_->SetSize({ 32.0f, 32.0f });
}

void DashUIManager::Finalize()
{
    if (icon_) {
        delete icon_;
        icon_ = nullptr;
    }
    if (overlay_) {
        delete overlay_;
        overlay_ = nullptr;
    }
}

void DashUIManager::Update(float dt)
{
    (void)dt;
    if (!player_) {
        if (icon_)    icon_->Update();
        if (overlay_) overlay_->Update();
        return;
    }

    float ratio = 0.0f;
    if (!player_->CanDash()) {
        ratio = player_->GetDashCooldown() / player_->GetDashCooldownDuration();
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
    }

    if (overlay_) {
        if (ratio > 0.0f) {
            float fullH     = 32.0f;
            float visibleH  = fullH * ratio;
            overlay_->SetTextureLeftTop({ 0.0f, 0.0f });
            overlay_->SetTextureSize({ 32.0f, visibleH });
            overlay_->SetSize({ 32.0f, visibleH });
            overlay_->SetVisible(true);
        } else {
            overlay_->SetVisible(false);
        }
        overlay_->Update();
    }

    if (icon_) {
        icon_->Update();
    }
}

void DashUIManager::Draw()
{
    if (icon_) {
        icon_->Draw();
    }
    if (overlay_ && overlay_->IsVisible()) {
        overlay_->Draw();
    }
}
