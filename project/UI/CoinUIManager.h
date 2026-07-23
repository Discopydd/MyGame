#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "Camera.h"
#include "WinApp.h"
#include <memory>
/// <summary>
/// CoinUIManagerに関する処理と状態を管理するクラスです。
/// </summary>
class CoinUIManager {
public:
    /// <summary>
    /// CoinUIManagerのインスタンスを生成します。
    /// </summary>
    CoinUIManager() = default;
    /// <summary>
    /// CoinUIManagerが保持するリソースを破棄します。
    /// </summary>
    ~CoinUIManager() = default;

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="spriteCommon">スプライト生成に使用する共通描画処理。</param>
    /// <param name="object3dCommon">3Dオブジェクト生成に使用する共通描画処理。</param>
    /// <param name="camera">描画および座標変換に使用するカメラ。</param>
    /// <param name="hpNdcZ">HP表示に使用するNDC空間のZ値。</param>
    void Initialize(MyEngine::SpriteCommon* spriteCommon,
                    MyEngine::Object3dCommon* object3dCommon,
                    MyEngine::Camera* camera,
                    float hpNdcZ);

    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize();

    // 現在のマップに残っているコイン数を設定する（0～999）。UI は自動で更新される
    /// <summary>
    /// Remaining Coinを設定します。
    /// </summary>
    /// <param name="remaining">設定する残り数。</param>
    void SetRemainingCoin(int remaining);
    /// <summary>
    /// Remaining Coinを取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    int  GetRemainingCoin() const { return remainingCoin_; }

    // 毎フレーム更新（ライト点滅用のタイマー）
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    void Update(float dt);

    // GameScene::Draw() 内で呼び出す
    /// <summary>
    /// 3D要素を画面へ描画します。
    /// </summary>
    void Draw3D();  // 3D coin モデル
    /// <summary>
    /// 2D要素を画面へ描画します。
    /// </summary>
    void Draw2D();  // コロン + 数字

private:
    MyEngine::SpriteCommon*   spriteCommon_   = nullptr;
    MyEngine::Object3dCommon* object3dCommon_ = nullptr;
    MyEngine::Camera*         camera_         = nullptr;
    float           hpNdcZ_         = 0.08f;

    // 3D coin モデル
    std::unique_ptr<MyEngine::Object3d> coinObj_;
    std::unique_ptr<MyEngine::Sprite>   colonSprite_;
    std::unique_ptr<MyEngine::Sprite>   digitSprites_[3] = {
        nullptr, nullptr, nullptr
    };

    int   remainingCoin_ = 0;
    int   lastCoin_      = -1;
    float lightTime_ = 0.0f;

    // 右上の数字 UI を更新する（旧 UpdateCoinCountUI_）
    /// <summary>
    /// Digitsを更新します。
    /// </summary>
    void UpdateDigits_();
};
