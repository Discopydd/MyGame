#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Camera.h"
#include "MyMath.h"
#include <vector>
#include <memory>

// GameScene 内にすでにある構造体。ここでも同じ定義を使う
/// <summary>
/// HintSpriteで使用する関連データをまとめて保持する構造体です。
/// </summary>
struct HintSprite {
    std::unique_ptr<MyEngine::Sprite> sprite;
    MyEngine::Vector3 worldPos{};
};

// GameScene 内で実装されている WorldToScreen。ここでは宣言のみ行う
/// <summary>
/// World To Screen処理を実行します。
/// </summary>
/// <param name="worldPos">ワールド座標。</param>
/// <param name="camera">描画および座標変換に使用するカメラ。</param>
/// <returns>計算または取得した結果。</returns>
MyEngine::Vector3 WorldToScreen(const MyEngine::Vector3& worldPos, MyEngine::Camera* camera);

/// <summary>
/// HintUIManagerに関する処理と状態を管理するクラスです。
/// </summary>
class HintUIManager {
public:
    /// <summary>
    /// HintUIManagerのインスタンスを生成します。
    /// </summary>
    HintUIManager() = default;
    /// <summary>
    /// HintUIManagerが保持するリソースを破棄します。
    /// </summary>
    ~HintUIManager() = default;

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="spriteCommon">スプライト生成に使用する共通描画処理。</param>
    /// <param name="camera">描画および座標変換に使用するカメラ。</param>
    void Initialize(MyEngine::SpriteCommon* spriteCommon, MyEngine::Camera* camera);
    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize();

    // このマネージャに GameScene 内の HintSprite / コンテナを渡す
    /// <summary>
    /// Space Hintを設定します。
    /// </summary>
    /// <param name="spaceHint">処理対象のオブジェクトへのポインタ。</param>
    void SetSpaceHint(HintSprite* spaceHint)   { spaceHint_ = spaceHint; }
    /// <summary>
    /// Shift Hintを設定します。
    /// </summary>
    /// <param name="shiftHint">処理対象のオブジェクトへのポインタ。</param>
    void SetShiftHint(HintSprite* shiftHint)   { shiftHint_ = shiftHint; }
    /// <summary>
    /// Sprint Hintを設定します。
    /// </summary>
    /// <param name="sprintHint">処理対象のオブジェクトへのポインタ。</param>
    void SetSprintHint(HintSprite* sprintHint) { sprintHint_ = sprintHint; }
    /// <summary>
    /// Up Hintsを設定します。
    /// </summary>
    /// <param name="upHints">処理対象のオブジェクトへのポインタ。</param>
    void SetUpHints(std::vector<HintSprite>* upHints) { upHints_ = upHints; }

    // 毎フレーム更新：上下の揺れを計算し、ワールド座標から画面座標へ変換する
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    void Update(float dt);

    // GameScene::Draw() の「中間レイヤー MyEngine::Sprite」内で呼び出す
    /// <summary>
    /// 現在の状態を画面へ描画します。
    /// </summary>
    void Draw();

private:
    MyEngine::SpriteCommon* spriteCommon_ = nullptr;
    MyEngine::Camera*       camera_       = nullptr;

    HintSprite* spaceHint_  = nullptr;
    HintSprite* shiftHint_  = nullptr;
    HintSprite* sprintHint_ = nullptr;
    std::vector<HintSprite>* upHints_ = nullptr;

    // 上下に揺らすためのパラメータ（元の GameScene にあった 3 つ）
    float bobTime_      = 0.0f;
    float bobAmplitude_ = 6.0f;   // 移動ピクセル（上下±6）
    float bobSpeed_     = 3.0f;   // 周波数（大きいほど速く揺れる）

    // Move hint icons (key_A / arrow_left / key_D / arrow_right)
    std::unique_ptr<MyEngine::Sprite> moveKeyA_;
    std::unique_ptr<MyEngine::Sprite> moveArrowL_;
    std::unique_ptr<MyEngine::Sprite> moveKeyD_;
    std::unique_ptr<MyEngine::Sprite> moveArrowR_;
};
