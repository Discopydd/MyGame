#pragma once

#include "scene/Framework.h"
#include "scene/GameScene.h"
#include "scene/TitleScene.h"
#include "BaseScene.h"

class MyGame : public Framework {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    BaseScene* scene_ = nullptr;
};
