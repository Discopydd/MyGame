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

    // overlay の元テクスチャサイズを保存する（Initialize 内で textureSize_ は「元画像サイズ」になる）
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
            // アイコンの画面サイズ／位置（位置合わせ用）
            const Vector2 iconSize = icon_ ? icon_->GetSize() : Vector2{ 32.0f, 32.0f };
            const Vector2 iconPos  = icon_ ? icon_->GetPosition() : Vector2{ 40.0f, 80.0f };

            const float fullSpriteH    = iconSize.y;
            const float visibleSpriteH = fullSpriteH * ratio;

            // テクスチャの切り抜きは必ず「元テクスチャの高さ」で計算する（例: 52 * ratio）。そうしないと切り抜き位置がずれる
            const float texW        = overlayFullTexSize_.x;
            const float texH        = overlayFullTexSize_.y;
            const float visibleTexH = texH * ratio;

            // 下から表示を開始（下揃え）
            overlay_->SetTextureLeftTop({ 0.0f, texH - visibleTexH });
            overlay_->SetTextureSize({ texW, visibleTexH });

            // 画面上では幅をそのままにし、高さだけ ratio に応じて縮め、位置を下げて下揃えを維持する
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
