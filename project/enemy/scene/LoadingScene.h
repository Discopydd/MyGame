#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "SoundManager.h"
#include "Camera.h"
#include <vector>
#include "BaseScene.h"
#include <thread>
#include <atomic>
#include <memory>  // ★ std::unique_ptr を使用

/// <summary>
/// マップ読み込み中の画面表示と非同期初期化の進行を管理するシーンクラス。
/// </summary>
class LoadingScene : public MyEngine::BaseScene {
public:
    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    void Initialize() override;
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    void Update() override;
    /// <summary>
    /// 現在の状態を画面へ描画します。
    /// </summary>
    void Draw() override;
    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize() override;

    // 設定読み込み進捗（0.0 から 1.0）
    /// <summary>
    /// Progressを設定します。
    /// </summary>
    /// <param name="progress">処理に使用するprogressの値。</param>
    void SetProgress(float progress) { progress_ = progress; }

private:
    // 非所有: 外部シングルトンなので生ポインタのままでよい
    MyEngine::WinApp*        winApp_    = nullptr;
    MyEngine::DirectXCommon* dxCommon_  = nullptr;
    MyEngine::Input*         input_     = nullptr;
    MyEngine::SrvManager*    srvManager_ = nullptr;
    MyEngine::SpriteCommon* spriteCommon_ = nullptr;

    // プログレスバー関連
    float progress_ = 0.0f;
    std::unique_ptr<MyEngine::Sprite> progressBar_;
    std::unique_ptr<MyEngine::Sprite> progressBackground_;

    std::unique_ptr<MyEngine::Sprite> blackSprite_;
    std::unique_ptr<MyEngine::Sprite> portalRingSprite_;
    float portalRingRotation_ = 0.0f;

    // ===== Spinner（白い点が回転する表示） =====
    std::vector<std::unique_ptr<MyEngine::Sprite>> spinnerDots_;
    int   spinnerCount_     = 12;      // 点の数
    float spinnerRadius_    = 20.0f;   // 半径（ピクセル）
    float spinnerSize_      = 7.0f;    // 各点の正方形サイズ（ピクセル）
    float spinnerSpeed_     = 10.0f;   // 角速度（ラジアン/秒）
    float spinnerHeadAngle_ = 0.0f;    // ヘッドの現在角度
    float spinnerTrailLen_  = 0.9f;    // テール長（0〜π）。値が大きいほど尾が長くなる
    float spinnerMinAlpha_  = 0.18f;   // 尾端の最小透明度
    std::string spinnerTexPath_ = "Resources/white_sphere.png";

    /// <summary>
    /// Spinnerを生成します。
    /// </summary>
    void CreateSpinner_();
};
