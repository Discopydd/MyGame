#include "DashUIManager.h"

void DashUIManager::Initialize(SpriteCommon* spriteCommon, Player* player)
{
    spriteCommon_ = spriteCommon;
    player_       = player;

    const char* textureFilePath[] = { "Resources/skill_icon.png", "Resources/gray.png" };

    icon_ = std::make_unique<Sprite>();
    icon_->Initialize(spriteCommon_, textureFilePath[0]);
    icon_->SetPosition({ 30.0f, 80.0f });
    icon_->SetSize({ 64.0f, 64.0f });

    overlay_ = std::make_unique<Sprite>();
    overlay_->Initialize(spriteCommon_, textureFilePath[1]);

    // 保存 overlay 原贴图尺寸（Initialize 内会把 textureSize_ 设为“贴图原尺寸”）
    overlayFullTexSize_ = overlay_->GetTextureSize();

    overlay_->SetPosition({ 30.0f, 80.0f });
    overlay_->SetSize({ 64.0f, 64.0f });
}

void DashUIManager::Finalize()
{
    icon_.reset();
    overlay_.reset();
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
            // 图标的屏幕尺寸/位置（用于对齐）
            const Vector2 iconSize = icon_ ? icon_->GetSize() : Vector2{ 32.0f, 32.0f };
            const Vector2 iconPos  = icon_ ? icon_->GetPosition() : Vector2{ 40.0f, 80.0f };

            const float fullSpriteH    = iconSize.y;
            const float visibleSpriteH = fullSpriteH * ratio;

            // 贴图裁剪必须按“原贴图高度”计算（例如 52 * ratio），否则会裁错
            const float texW        = overlayFullTexSize_.x;
            const float texH        = overlayFullTexSize_.y;
            const float visibleTexH = texH * ratio;

            // 从底部开始显示（底部对齐）
            overlay_->SetTextureLeftTop({ 0.0f, texH - visibleTexH });
            overlay_->SetTextureSize({ texW, visibleTexH });

            // 屏幕上仍然是 32 宽，按 ratio 缩高度，并把位置往下挪保持底部对齐
            overlay_->SetSize({ iconSize.x, visibleSpriteH });
            overlay_->SetPosition({ iconPos.x, iconPos.y + (fullSpriteH - visibleSpriteH) });

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
