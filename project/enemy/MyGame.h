#pragma once

#include "scene/Framework.h"
#include "scene/GameScene.h"
#include "scene/TitleScene.h"
#include "BaseScene.h"

/// <summary>
/// ゲーム全体の起動処理を担い、最初のシーンを設定してアプリケーションを実行するクラス。
/// </summary>
class MyGame : public Framework {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;
};
