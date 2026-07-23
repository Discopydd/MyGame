#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "WinApp.h"
#include <memory>
// 元の列挙名をそのまま使い、再利用しやすくする
enum class FadePhase { None, FadingOut, LoadingHold, FadingIn };

/// <summary>
/// 画面のフェードアウト、読み込み待機、フェードインによる場面転換を管理するクラス。
/// </summary>
class FadeManager {
public:
    /// <summary>
    /// FadeManagerのインスタンスを生成します。
    /// </summary>
    FadeManager() = default;
    /// <summary>
    /// FadeManagerが保持するリソースを破棄します。
    /// </summary>
    ~FadeManager() = default;

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="spriteCommon">スプライト生成に使用する共通描画処理。</param>
    void Initialize(MyEngine::SpriteCommon* spriteCommon);
    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize();

    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    void Update(float dt);
    /// <summary>
    /// 現在の状態を画面へ描画します。
    /// </summary>
    void Draw();

    // Phase / Alpha へのアクセス
    /// <summary>
    /// Phaseを取得します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
    FadePhase GetPhase() const { return phase_; }
    /// <summary>
    /// Phaseを設定します。
    /// </summary>
    /// <param name="p">処理に使用するpの値。</param>
    void      SetPhase(FadePhase p) { phase_ = p; }

    /// <summary>
    /// Alphaを取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    float GetAlpha() const { return alpha_; }
    /// <summary>
    /// Alphaを設定します。
    /// </summary>
    /// <param name="a">計算に使用する1つ目の値。</param>
    void  SetAlpha(float a) {
        alpha_ = a;
        if (alpha_ < 0.0f) alpha_ = 0.0f;
        if (alpha_ > 1.0f) alpha_ = 1.0f;
        SyncVisual_();
    }

    /// <summary>
    /// Speedを取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    float GetSpeed() const { return speed_; }
    /// <summary>
    /// Speedを設定します。
    /// </summary>
    /// <param name="s">演算に使用するスカラー値。</param>
    void  SetSpeed(float s) { speed_ = s; }

    /// <summary>
    /// Spriteを取得します。
    /// </summary>
    /// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
    MyEngine::Sprite* GetSprite() { return sprite_.get(); }

    // 簡易操作
    /// <summary>
    /// Fade Outを開始します。
    /// </summary>
    void StartFadeOut();   // 0 から 1 へフェード
    /// <summary>
    /// Fade Inを開始します。
    /// </summary>
    void StartFadeIn();    // 1 から 0 へフェード
    /// <summary>
    /// Blackを設定します。
    /// </summary>
    void SetBlack();       // 即座に全黒
    /// <summary>
    /// Clear処理を実行します。
    /// </summary>
    void Clear();          // 即座に透明

    // 現在のロジックで使っている「全黒到達 + 停留」もそのまま残す
    /// <summary>
    /// Reached Black処理を実行します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool  ReachedBlack() const { return reachedBlack_; }
    /// <summary>
    /// Reached Blackを設定します。
    /// </summary>
    /// <param name="v">設定または計算に使用する値。</param>
    void  SetReachedBlack(bool v) { reachedBlack_ = v; }

    /// <summary>
    /// Black Hold Framesを取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    int   GetBlackHoldFrames() const { return blackHoldFrames_; }
    /// <summary>
    /// Black Hold Framesを設定します。
    /// </summary>
    /// <param name="f">処理に使用するfの値。</param>
    void  SetBlackHoldFrames(int f) { blackHoldFrames_ = f; }

    /// <summary>
    /// Overlay Pushed処理を実行します。
    /// </summary>
    /// <returns>判定結果。</returns>
    bool  OverlayPushed() const { return overlayPushed_; }
    /// <summary>
    /// Overlay Pushedを設定します。
    /// </summary>
    /// <param name="v">設定または計算に使用する値。</param>
    void  SetOverlayPushed(bool v) { overlayPushed_ = v; }

private:
    /// <summary>
    /// Sync Visual処理を実行します。
    /// </summary>
    void SyncVisual_();

    MyEngine::SpriteCommon* spriteCommon_ = nullptr;
    std::unique_ptr<MyEngine::Sprite> sprite_;
    std::unique_ptr<MyEngine::Sprite> portalRingSprite_;

    float ringRotation_  = 0.0f;
    float ringPulseTime_ = 0.0f;

    FadePhase phase_   = FadePhase::None;
    float     alpha_   = 0.0f;
    float     speed_   = 0.16f;

    bool  reachedBlack_    = false;
    int   blackHoldFrames_ = 0;
    bool  overlayPushed_   = false;
};
