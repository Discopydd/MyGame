#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "Camera.h"
#include "MyMath.h"
#include "WinApp.h"
#include <memory>
class GameClearManager {
public:
    enum class State { None, SlideTitle, PlayerShow, Done };

    GameClearManager() = default;
    ~GameClearManager() = default;

    // ポインタ形式で初期化
    void Initialize(SpriteCommon* spriteCommon,
                    Object3dCommon* object3dCommon,
                    Camera* camera,
                    float hpNdcZ);

    void Finalize();

    void Start();          // GameClear 演出を開始
    void Update(float dt); // 状態機の更新（GameScene::Update 内で呼び出す）

    // Draw を 2D タイトル / 3D プレイヤーに分け、GameScene::Draw の異なる位置に挿入しやすくする
    void DrawTitle();
    void DrawPlayer();

    // 状態確認
    bool IsPlaying() const {
        return (state_ != State::None && state_ != State::Done);
    }
    State GetState() const { return state_; }

private:
    // 内部状態
    State state_ = State::None;
    float t_     = 0.0f;   // 汎用タイマー

    // === タイトル関連 ===
    std::unique_ptr<Sprite> titleSprite_;
    Vector2 titleSize_   = { 1280.0f, 720.0f };
    Vector2 titlePos_{};
    Vector2 titleStartPos_{};
    Vector2 titleEndPos_{};
    float   titleSlideTime_ = 0.65f;

    // === GameClear プレイヤーモデル ===
    std::unique_ptr<Object3d> clearPlayerObj_;
    Vector3   clearPlayerStartPos_{};
    Vector3   clearPlayerBasePos_{};
    float     clearPlayerSpinT_ = 0.0f;

    // 共用リソースへのポインタ（自前では delete しない）
    SpriteCommon*  spriteCommon_  = nullptr;
    Object3dCommon* object3dCommon_ = nullptr;
    Camera*        camera_        = nullptr;
    float          hpNdcZ_        = 0.08f;

private:
    float EaseOutCubic_(float t) { return 1.0f - powf(1.0f - t, 3.0f); }
};
