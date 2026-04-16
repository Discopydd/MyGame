#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "Camera.h"
#include "MyMath.h"
#include "WinApp.h"
#include <memory>
#include <vector>

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

    // 勝利画面の 2D 要素を描画
    void DrawTitle();
    void DrawPlayer();

    // 状態確認
    bool IsPlaying() const {
        return (state_ != State::None && state_ != State::Done);
    }
    State GetState() const { return state_; }

private:
    struct PortalMote {
        std::unique_ptr<Sprite> sprite;
        float angle = 0.0f;
        float radius = 0.0f;
        float angularSpeed = 0.0f;
        float radialSpeed = 0.0f;
        float driftY = 0.0f;
        float life = 0.0f;
        float maxLife = 0.0f;
        float baseSize = 16.0f;
        Vector4 color = { 1,1,1,1 };
    };

private:
    void SpawnMote_();
    void UpdateMotes_(float dt);

    float Clamp01_(float t) {
        if (t < 0.0f) { return 0.0f; }
        if (t > 1.0f) { return 1.0f; }
        return t;
    }

    float EaseOutBack_(float t) {
        float c1 = 1.70158f;
        float c3 = c1 + 1.0f;
        float p  = t - 1.0f;
        return 1.0f + p * p * (c3 * p + c1);
    }

    float EaseOutCubic_(float t) { return 1.0f - powf(1.0f - t, 3.0f); }

private:
    // 内部状態
    State state_ = State::None;
    float t_     = 0.0f;   // 汎用タイマー

    // === タイトル / 背景関連 ===
    std::unique_ptr<Sprite> backdropSprite_;
    std::unique_ptr<Sprite> portalRingSprite_;
    std::unique_ptr<Sprite> titleSprite_;
    std::unique_ptr<Sprite> panelSprite_;
    std::unique_ptr<Sprite> promptSprite_;

    std::vector<PortalMote> motes_;
    float moteSpawnTimer_ = 0.0f;

    Vector2 titleSize_   = { 1280.0f, 720.0f };
    Vector2 titlePos_{};
    Vector2 titleStartPos_{};
    Vector2 titleEndPos_{};
    float   titleSlideTime_ = 0.65f;

    Vector2 ringCenter_{};
    Vector2 ringBaseSize_ = { 560.0f, 560.0f };
    float   ringRotation_ = 0.0f;
    float   ringPulseT_   = 0.0f;
    float   uiFloatT_     = 0.0f;
    Vector2 panelBasePos_{};
    Vector2 promptBasePos_{};

    // 共用リソースへのポインタ（自前では delete しない）
    SpriteCommon*   spriteCommon_   = nullptr;
    Object3dCommon* object3dCommon_ = nullptr;
    Camera*         camera_         = nullptr;
    float           hpNdcZ_         = 0.08f;
};
