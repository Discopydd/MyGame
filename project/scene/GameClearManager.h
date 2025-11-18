#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "Camera.h"
#include "MyMath.h"
#include "WinApp.h"

class GameClearManager {
public:
    enum class State { None, SlideTitle, PlayerShow, Done };

    GameClearManager() = default;
    ~GameClearManager() = default;

    // 使用指针形式初始化
    void Initialize(SpriteCommon* spriteCommon,
                    Object3dCommon* object3dCommon,
                    Camera* camera,
                    float hpNdcZ);

    void Finalize();

    void Start();          // 开始 GameClear 演出
    void Update(float dt); // 状态机更新（GameScene::Update 里调用）

    // Draw 分成 2D 标题 / 3D 玩家，方便插在 GameScene::Draw 的不同位置
    void DrawTitle();
    void DrawPlayer();

    // 状态查询
    bool IsPlaying() const {
        return (state_ != State::None && state_ != State::Done);
    }
    State GetState() const { return state_; }

private:
    // 内部状态
    State state_ = State::None;
    float t_     = 0.0f;   // 通用计时

    // === 标题相关 ===
    Sprite* titleSprite_ = nullptr;
    Vector2 titleSize_   = { 1280.0f, 720.0f };
    Vector2 titlePos_{};
    Vector2 titleStartPos_{};
    Vector2 titleEndPos_{};
    float   titleSlideTime_ = 0.65f;

    // === GameClear 玩家模型 ===
    Object3d* clearPlayerObj_ = nullptr;
    Vector3   clearPlayerStartPos_{};
    Vector3   clearPlayerBasePos_{};
    float     clearPlayerSpinT_ = 0.0f;

    // 共用资源指针（不自己 delete）
    SpriteCommon*  spriteCommon_  = nullptr;
    Object3dCommon* object3dCommon_ = nullptr;
    Camera*        camera_        = nullptr;
    float          hpNdcZ_        = 0.08f;

private:
    float EaseOutCubic_(float t) { return 1.0f - powf(1.0f - t, 3.0f); }
};
