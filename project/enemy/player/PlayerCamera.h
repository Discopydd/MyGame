#pragma once
#include "Camera.h"
#include "Vector3.h"
#include "Player.h"
#include "map/MapChipField.h"

/// <summary>
/// プレイヤーを追従しながら表示範囲を制限するゲーム用カメラクラス。
/// </summary>
class PlayerCamera : public MyEngine::Camera {
public:
    /// <summary>
    /// PlayerCameraのインスタンスを生成します。
    /// </summary>
    PlayerCamera();
    /// <summary>
    /// PlayerCameraが保持するリソースを破棄します。
    /// </summary>
    ~PlayerCamera() = default;

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="camera">描画および座標変換に使用するカメラ。</param>
    /// <param name="player">判定または更新対象のプレイヤー。</param>
    /// <param name="map">地形情報と衝突判定に使用するマップデータ。</param>
    void Initialize(MyEngine::Camera* camera, const Player* player, const MapChipField* map);
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    void Update();
    /// <summary>
    /// Snap To Target処理を実行します。
    /// </summary>
    void SnapToTarget();

    /// <summary>
    /// Offsetを設定します。
    /// </summary>
    /// <param name="offset">処理に使用する参照値。</param>
    void SetOffset(const MyEngine::Vector3& offset) { offset_ = offset; }
    /// <summary>
    /// Follow Speedを設定します。
    /// </summary>
    /// <param name="speed">移動速度。</param>
    void SetFollowSpeed(float speed) { followSpeed_ = speed; }
    /// <summary>
    /// Constrain To Mapを設定します。
    /// </summary>
    /// <param name="constrain">範囲制限を有効にする場合は true。</param>
    void SetConstrainToMap(bool constrain) { constrainToMap_ = constrain; }
    /// <summary>
    /// Map Boundsを設定します。
    /// </summary>
    /// <param name="min">最小座標。</param>
    /// <param name="max">最大座標。</param>
    void SetMapBounds(const MyEngine::Vector3& min, const MyEngine::Vector3& max) {
        mapMin_ = min;
        mapMax_ = max;
        useCustomBounds_ = true;
    }
    
    /// <summary>
    /// Map Boundsを初期状態へ戻します。
    /// </summary>
    void ResetMapBounds() {
        useCustomBounds_ = false;
    }
private:
    MyEngine::Camera* camera_ = nullptr;
    const Player* player_ = nullptr;
    const MapChipField* map_ = nullptr;

    MyEngine::Vector3 offset_ = {0, 0.0f, 0.0f}; // デフォルトのカメラオフセット (X,Y,Z)
    float followSpeed_ = 0.1f;            // 追従のなめらかさ (0-1)
    bool constrainToMap_ = true;          // マップ境界内に制限するかどうか

    MyEngine::Vector3 mapMin_ = {0, 0, 0};         // カスタムマップ最小境界
    MyEngine::Vector3 mapMax_ = {100, 100, 0};     // カスタムマップ最大境界
    bool useCustomBounds_ = false;       // カスタム境界を使うかどうか

    /// <summary>
    /// Target Positionを計算します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
    MyEngine::Vector3 CalculateTargetPosition() const;
    /// <summary>
    /// Constrain Position処理を実行します。
    /// </summary>
    /// <param name="position">対象のワールド座標。</param>
    /// <returns>計算または取得した結果。</returns>
    MyEngine::Vector3 ConstrainPosition(const MyEngine::Vector3& position) const;
};
