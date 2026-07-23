#pragma once
#include <vector>
#include <string>
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Camera.h"
#include <map/MapChipField.h>
#include "MyMath.h"
#include <memory>
// もともと GameScene にあった PortalInfo をここへ移動
/// <summary>
/// PortalInfoで使用する関連データをまとめて保持する構造体です。
/// </summary>
struct PortalInfo {
    MapChipField::IndexSet index;      // 転送ポータルのマス座標
    std::string targetMap;             // 遷移先マップのパス
    MyEngine::Vector3 targetStartPos;  // 遷移先マップでのプレイヤー開始位置
    MyEngine::Vector3 worldPos;        // 現在マップ上のポータル表示位置
};

// GameScene.cpp にある補助関数の宣言（実装は GameScene.cpp 側）
/// <summary>
/// World To Screen処理を実行します。
/// </summary>
/// <param name="worldPos">ワールド座標。</param>
/// <param name="camera">描画および座標変換に使用するカメラ。</param>
/// <returns>計算または取得した結果。</returns>
MyEngine::Vector3 WorldToScreen(const MyEngine::Vector3& worldPos, MyEngine::Camera* camera);

/// <summary>
/// PortalManagerに関する処理と状態を管理するクラスです。
/// </summary>
class PortalManager {
public:
    /// <summary>
    /// PortalManagerのインスタンスを生成します。
    /// </summary>
    PortalManager() = default;
    /// <summary>
    /// PortalManagerが保持するリソースを破棄します。
    /// </summary>
    ~PortalManager() = default;

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

    /// <summary>
    /// Portalsをクリアします。
    /// </summary>
    void ClearPortals();
    /// <summary>
    /// Portalを追加します。
    /// </summary>
    /// <param name="idx">対象マスのインデックス。</param>
    /// <param name="targetMap">遷移先のマップパス。</param>
    /// <param name="startPos">開始時のワールド座標。</param>
    /// <param name="worldPos">ワールド座標。</param>
    void AddPortal(const MapChipField::IndexSet& idx,
                   const std::string& targetMap,
                   const MyEngine::Vector3& startPos,
                   const MyEngine::Vector3& worldPos);

    // 現在登録されている全ポータルを返す（GameScene 側で配置に使用）
    /// <summary>
    /// Portalsを取得します。
    /// </summary>
    /// <returns>保持している値への参照。</returns>
    const std::vector<PortalInfo>& GetPortals() const { return portals_; }

    // プレイヤーがいずれかのポータル上にいる場合、そのポータルへのポインタを返す。なければ nullptr
    /// <summary>
    /// Portal Atを取得します。
    /// </summary>
    /// <param name="playerIndex">プレイヤーがいるマスのインデックス。</param>
    /// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
    const PortalInfo* GetPortalAt(const MapChipField::IndexSet& playerIndex) const;

    // 「E を押す」ヒントアイコンを更新（表示有無 + 位置）
    /// <summary>
    /// Hintを更新します。
    /// </summary>
    /// <param name="playerIndex">プレイヤーがいるマスのインデックス。</param>
    /// <param name="playerWorldPos">プレイヤーのワールド座標。</param>
    /// <param name="canControl">プレイヤーを操作可能な場合は true。</param>
    void UpdateHint(const MapChipField::IndexSet& playerIndex,
                    const MyEngine::Vector3& playerWorldPos,
                    bool canControl);

    // 描画ヒントアイコン（GameScene::Draw 内呼び出す）
    /// <summary>
    /// Hintを描画します。
    /// </summary>
    void DrawHint();

    // コイン未回収で使用不可の間、ポータル上に鍵アイコンを表示する
    /// <summary>
    /// Lock Visibleを設定します。
    /// </summary>
    /// <param name="visible">表示する場合は true。</param>
    void SetLockVisible(bool visible);
    /// <summary>
    /// Lock Iconsを描画します。
    /// </summary>
    void DrawLockIcons();

private:
    MyEngine::SpriteCommon* spriteCommon_ = nullptr;
    MyEngine::Camera*       camera_       = nullptr;

    std::unique_ptr<MyEngine::Sprite> hintSprite_;  // 「E を押す」アイコン
    std::unique_ptr<MyEngine::Sprite> lockSprite_;  // ポータルのロックアイコン
    std::vector<PortalInfo> portals_;
    bool showLockIcons_ = false;
    float lockPulseTimer_ = 0.0f;
    bool unlockBurstActive_ = false;
    float unlockBurstTimer_ = 0.0f;
};
