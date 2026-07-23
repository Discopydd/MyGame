#pragma once
#include "Object3dCommon.h"
#include "Object3d.h"
#include "Camera.h"
#include "WinApp.h"
#include "MyMath.h"
#include <vector>
#include <memory>
// Player の GetHpRatio を使用する
#include <player/Player.h>

/// <summary>
/// プレイヤーやボスの残りHPを3D空間および画面上のゲージとして表示するクラス。
/// </summary>
class HPBar3DManager {
public:
    /// <summary>
    /// HPBar3DManagerのインスタンスを生成します。
    /// </summary>
    HPBar3DManager() = default;
    /// <summary>
    /// HPBar3DManagerが保持するリソースを破棄します。
    /// </summary>
    ~HPBar3DManager() = default;

    // hpNdcZ: HPバーのカメラからの奥行きを制御する値（GameScene の hpNdcZ_ と同じ）
    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="objCommon">3Dオブジェクト生成に使用する共通描画処理。</param>
    /// <param name="camera">描画および座標変換に使用するカメラ。</param>
    /// <param name="player">判定または更新対象のプレイヤー。</param>
    /// <param name="hpNdcZ">HP表示に使用するNDC空間のZ値。</param>
    void Initialize(MyEngine::Object3dCommon* objCommon,
                    MyEngine::Camera* camera,
                    Player* player,
                    float hpNdcZ);

    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize();

    // 毎フレーム更新（可視段数の計算と再配置）
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    void Update(float dt);

    // GameScene::Draw の 3D 描画段階で呼び出す
    /// <summary>
    /// 3D要素を画面へ描画します。
    /// </summary>
    void Draw3D();

    // 後でプレイヤーポインタが変わった場合は再設定できる
    /// <summary>
    /// Playerを設定します。
    /// </summary>
    /// <param name="player">判定または更新対象のプレイヤー。</param>
    void SetPlayer(Player* player) { player_ = player; }

private:
    MyEngine::Object3dCommon* object3dCommon_ = nullptr;
    MyEngine::Camera*         camera_         = nullptr;
    Player*         player_         = nullptr;

    std::vector<std::unique_ptr<MyEngine::Object3d>> strips_;   // 各HPバーセグメント

    int   segments_      = 5;        // 総段数
    int   visibleCount_  = 5;        // 現在可視段数
    float insetX_        = 40.0f;     // 画面左側内側余白
    float insetY_        = 50.0f;     // 画面上側内側余白
    float segPixelW_     = 45.0f;     // 各段の幅（画面ピクセル）
    float gapPixel_      = 4.0f;      // 段間の間隔（ピクセル）
    float hpNdcZ_        = 0.08f;     // 奥行き

    // 画面座標をワールド座標に変換（GameScene.cpp 内既存の同名関数）
    /// <summary>
    /// Screen To World処理を実行します。
    /// </summary>
    /// <param name="sx">処理に使用するsxの値。</param>
    /// <param name="sy">処理に使用するsyの値。</param>
    /// <param name="ndcZ">処理に使用するndcZの値。</param>
    /// <returns>計算または取得した結果。</returns>
    MyEngine::Vector3 ScreenToWorld_(float sx, float sy, float ndcZ);
};
