#pragma once
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Camera.h"
#include "map/MapChipField.h"
#include <vector>

/// <summary>
/// MovingPlatformに関する処理と状態を管理するクラスです。
/// </summary>
class MovingPlatform {
public:
    enum class Axis {
        Horizontal,
        Vertical
    };

    /// <summary>
    /// MovingPlatformのインスタンスを生成します。
    /// </summary>
    MovingPlatform() = default;
    /// <summary>
    /// MovingPlatformが保持するリソースを破棄します。
    /// </summary>
    ~MovingPlatform() = default;

    // speed の正負で初期方向を決める（正: 右/上、負: 左/下）
    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="common">3Dオブジェクト生成に使用する共通描画処理。</param>
    /// <param name="camera">描画および座標変換に使用するカメラ。</param>
    /// <param name="startPos">開始時のワールド座標。</param>
    /// <param name="axis">移動または回転の軸。</param>
    /// <param name="speed">移動速度。</param>
    /// <param name="lengthInTiles">処理に使用するlengthInTilesの値。</param>
    void Initialize(MyEngine::Object3dCommon* common, MyEngine::Camera* camera,
                    const MyEngine::Vector3& startPos, Axis axis, float speed,int lengthInTiles);

    // allPlatforms 足場同士の衝突検出用
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    /// <param name="field">更新対象のマップデータ。</param>
    /// <param name="allPlatforms">処理対象のオブジェクトへのポインタ。</param>
    void Update(float dt,
                const MapChipField& field,
                const std::vector<MovingPlatform*>& allPlatforms);

    /// <summary>
    /// 現在の状態を画面へ描画します。
    /// </summary>
    void Draw();

    /// <summary>
    /// 現在のワールド座標を取得します。
    /// </summary>
    /// <returns>保持している値への参照。</returns>
    const MyEngine::Vector3& GetPosition() const { return position_; }
    /// <summary>
    /// Prev Positionを取得します。
    /// </summary>
    /// <returns>保持している値への参照。</returns>
    const MyEngine::Vector3& GetPrevPosition() const { return prevPosition_; }

    // 現在の足場の AABB（MapChipField::Rect と一致）
    /// <summary>
    /// Rectを取得します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
    MapChipField::Rect GetRect() const;

private:
    // 1 本の足場に含まれるブロック数
    int lengthInTiles_ = 1;

    // この足場を構成する小ブロック
    std::vector<std::unique_ptr<MyEngine::Object3d>> tiles_;
    std::vector<MyEngine::Vector3>                   tileOffsets_;

    Axis axis_ = Axis::Horizontal;
    float speed_ = 1.0f;   // 絶対速度（単位: ワールド単位/秒）
    MyEngine::Vector3 dir_ = { 1, 0, 0 }; // 単位方向（左右 / 上下）

    MyEngine::Vector3 position_{};
    MyEngine::Vector3 prevPosition_{};

    float halfW_ = 0.0f;      // 足場全体の半幅
    float halfH_ = 0.0f;      // 足場全体の半高

    /// <summary>
    /// Block Collisionを処理します。
    /// </summary>
    /// <param name="field">更新対象のマップデータ。</param>
    void HandleBlockCollision(const MapChipField& field);
    /// <summary>
    /// Platform Collisionを処理します。
    /// </summary>
    /// <param name="allPlatforms">処理対象のオブジェクトへのポインタ。</param>
    void HandlePlatformCollision(const std::vector<MovingPlatform*>& allPlatforms);

};
