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
    HPBar3DManager() = default;
    ~HPBar3DManager() = default;

    // hpNdcZ: HPバーのカメラからの奥行きを制御する値（GameScene の hpNdcZ_ と同じ）
    void Initialize(MyEngine::Object3dCommon* objCommon,
                    MyEngine::Camera* camera,
                    Player* player,
                    float hpNdcZ);

    void Finalize();

    // 毎フレーム更新（可視段数の計算と再配置）
    void Update(float dt);

    // GameScene::Draw の 3D 描画段階で呼び出す
    void Draw3D();

    // 後でプレイヤーポインタが変わった場合は再設定できる
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
    MyEngine::Vector3 ScreenToWorld_(float sx, float sy, float ndcZ);
};
