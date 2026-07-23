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

    /// <summary>
    /// IntroManagerのインスタンスを生成します。
    /// </summary>
    IntroManager() = default;
    /// <summary>
    /// IntroManagerが保持するリソースを破棄します。
    /// </summary>
    ~IntroManager() = default;

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="spriteCommon">スプライト生成に使用する共通描画処理。</param>
    /// <param name="input">入力状態を取得する入力管理オブジェクト。</param>
    void Initialize(MyEngine::SpriteCommon* spriteCommon, MyEngine::Input* input);
    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize();

    // Intro を開始し、playerPos を周回中心として保持する（現状では保持のみで、後からカメラ演出にも使える）
    /// <summary>
    /// Start処理を実行します。
    /// </summary>
    /// <param name="playerPos">処理に使用する参照値。</param>
    void Start(const MyEngine::Vector3& playerPos);

    // 毎フレーム更新（GameScene::Update から呼び出す）
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    void Update(float dt);

    // イントロ UI を描画（GameScene::Draw から呼び出す）
    /// <summary>
    /// 現在の状態を画面へ描画します。
    /// </summary>
    void Draw();

    // 状態確認
    /// <summary>
    /// Playingかを判定します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool IsPlaying() const {
        return (state_ != State::None && state_ != State::Done);
    }
    /// <summary>
    /// Startedを保持しているかを判定します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool HasStarted() const { return started_; }
    /// <summary>
    /// Stateを取得します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
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
    /// <summary>
    /// Ease Out Cubic処理を実行します。
    /// </summary>
    /// <param name="t">補間係数。</param>
    /// <returns>計算または取得した数値。</returns>
    float EaseOutCubic_(float t) { return 1.0f - powf(1.0f - t, 3.0f); }
    /// <summary>
    /// Ease In Out Sine処理を実行します。
    /// </summary>
    /// <param name="t">補間係数。</param>
    /// <returns>計算または取得した数値。</returns>
    float EaseInOutSine_(float t) { return 0.5f * (1.0f - cosf(3.1415926f * t)); }
};
