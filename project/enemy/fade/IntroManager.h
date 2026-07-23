#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Input.h"
#include "WinApp.h"
#include "MyMath.h"
#include <memory>
/// <summary>
/// ゲーム開始時の黒帯、カメラ演出、タイトル表示などのイントロ演出を管理するクラス。
/// </summary>
class IntroManager {
public:
    enum class State { None, BarsIn, OrbitZoom, TitleShow, BarsOut, Done };

    IntroManager() = default;
    ~IntroManager() = default;

    void Initialize(MyEngine::SpriteCommon* spriteCommon, MyEngine::Input* input);
    void Finalize();

    // Intro を開始し、playerPos を周回中心として保持する（現状では保持のみで、後からカメラ演出にも使える）
    void Start(const MyEngine::Vector3& playerPos);

    // 毎フレーム更新（GameScene::Update から呼び出す）
    void Update(float dt);

    // イントロ UI を描画（GameScene::Draw から呼び出す）
    void Draw();

    // 状態確認
    bool IsPlaying() const {
        return (state_ != State::None && state_ != State::Done);
    }
    bool HasStarted() const { return started_; }
    State GetState() const { return state_; }

private:
    // 状態
    State state_   = State::None;
    float t_       = 0.0f;
    bool  skippable_ = true;   // キー入力でスキップ可能か
    bool  started_   = false;  // すでに一度開始したか

    // 各種 MyEngine::Sprite
    std::unique_ptr<MyEngine::Sprite> letterboxTop_;
    std::unique_ptr<MyEngine::Sprite> letterboxBottom_;
    std::unique_ptr<MyEngine::Sprite> vignette_;
    std::unique_ptr<MyEngine::Sprite> introTitle_;
    std::unique_ptr<MyEngine::Sprite> skipHint_;

    // カメラ関連（今は保持のみで、ここでは直接カメラを動かさない）
    MyEngine::Vector3 camStartPos_{ 0, 12, -85 };
    MyEngine::Vector3 camTargetPos_{ 0,  8, -38 };
    MyEngine::Vector3 camPivot_{ 0, 0, 0 };
    float   camOrbitDeg_ = 0.0f;

    // 画面揺れ（後で使いたくなったら、ここから camera にオフセットをかけられる）
    float shakeTime_ = 0.0f;
    float shakeAmp_  = 0.0f;

    MyEngine::SpriteCommon* spriteCommon_ = nullptr;
    MyEngine::Input*        input_        = nullptr;

    // Easing（GameScene の実装をそのまま使用）
    float EaseOutCubic_(float t) { return 1.0f - powf(1.0f - t, 3.0f); }
    float EaseInOutSine_(float t) { return 0.5f * (1.0f - cosf(3.1415926f * t)); }
};
