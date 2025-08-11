#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelManager.h"
#include "MyMath.h"
#include "ImGuiManager.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "SoundManager.h"
#include "Camera.h"
#include <vector>
#include "BaseScene.h"
class GameScene : public BaseScene {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    WinApp* winApp_ = nullptr;
    DirectXCommon* dxCommon_ = nullptr;
    Input* input_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;
    ImGuiManager* imguiManager_ = nullptr;
    Object3dCommon* object3dCommon_ = nullptr;
    Object3d* box_ = nullptr;
    Camera* camera_ = nullptr;
    std::vector<Sprite*> sprites_;
    ParticleEmitter* particleEmitter_ = nullptr;

    Vector2 rotation_{};
};
