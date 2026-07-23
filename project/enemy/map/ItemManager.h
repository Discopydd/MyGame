#pragma once
#include "Object3dCommon.h"
#include "Object3d.h"
#include "Camera.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <map/MapChipField.h>
#include <player/Player.h>
#include <utility>
/// <summary>
/// マップ上の収集アイテムの生成、取得判定、演出、描画を管理するクラス。
/// </summary>
class ItemManager {
public:
    /// <summary>
    /// マップ上に配置されたアイテムの座標と表示用3Dオブジェクトを保持する構造体。
    /// </summary>
    struct ItemVisual {
        uint32_t x = 0, y = 0;
        std::unique_ptr<MyEngine::Object3d> obj;
    };
    /// <summary>
    /// アイテム取得時に表示する演出オブジェクトと経過時間を保持する構造体。
    /// </summary>
    struct PickupEffect {
        std::unique_ptr<MyEngine::Object3d> obj;
        float     elapsed = 0.0f;                 // 再生経過時間（秒）
        float     duration = 0.35f;                // 総時間（秒）
        MyEngine::Vector3   velocity = { 0.0f, 0.0f, 0.0f }; // 上昇速度（ワールド単位/秒）
        float     rotateSpeedY = 0.0f;              // Y 軸まわりの回転速度（ラジアン/秒）
    };
    /// <summary>
    /// ItemManagerのインスタンスを生成します。
    /// </summary>
    ItemManager() = default;
    /// <summary>
    /// ItemManagerが保持するリソースを破棄します。
    /// </summary>
    ~ItemManager() = default;

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="objCommon">3Dオブジェクト生成に使用する共通描画処理。</param>
    /// <param name="camera">描画および座標変換に使用するカメラ。</param>
    void Initialize(MyEngine::Object3dCommon* objCommon, MyEngine::Camera* camera);
    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize();

    // 生成ブロック時用: このマップでそのセルが取得済みかどうか？
    /// <summary>
    /// Spawn Item可能かを判定します。
    /// </summary>
    /// <param name="mapPath">読み込みまたは識別に使用するマップファイルのパス。</param>
    /// <param name="x">X方向のインデックスまたは値。</param>
    /// <param name="y">Y方向のインデックスまたは値。</param>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool CanSpawnItem(const std::string& mapPath, uint32_t x, uint32_t y) const;

    // MyEngine::Object3d 生成後にマネージャへ登録
    /// <summary>
    /// Itemを登録します。
    /// </summary>
    /// <param name="mapPath">読み込みまたは識別に使用するマップファイルのパス。</param>
    /// <param name="x">X方向のインデックスまたは値。</param>
    /// <param name="y">Y方向のインデックスまたは値。</param>
    /// <param name="obj">処理に使用するobjの値。</param>
    void RegisterItem(const std::string& mapPath, uint32_t x, uint32_t y, std::unique_ptr<MyEngine::Object3d> obj);

    // 毎フレーム更新: 回転して Update
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    void Update(float dt);

    // 3D 描画
    /// <summary>
    /// 3D要素を画面へ描画します。
    /// </summary>
    void Draw3D();

    // プレイヤーがセル上に立った時に呼ぶ；アイテムを取得したら true を返す
    /// <summary>
    /// On Player Step On Tile処理を実行します。
    /// </summary>
    /// <param name="mapPath">読み込みまたは識別に使用するマップファイルのパス。</param>
    /// <param name="playerIndex">プレイヤーがいるマスのインデックス。</param>
    /// <param name="field">更新対象のマップデータ。</param>
    /// <param name="player">判定または更新対象のプレイヤー。</param>
    /// <returns>判定結果。</returns>
    bool OnPlayerStepOnTile(const std::string& mapPath,
                            const MapChipField::IndexSet& playerIndex,
                            MapChipField& field,
                            Player* player);

    // マップ切替時は現在のマップの可視オブジェクトだけクリア（取得済み記録は保持）
    /// <summary>
    /// Visualsをクリアします。
    /// </summary>
    void ClearVisuals();

    // 現在読み込まれているマップで、まだ取得されていないコイン数を返す
    /// <summary>
    /// Remaining Item Countを取得します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    int GetRemainingItemCount() const;

private:
    MyEngine::Object3dCommon* object3dCommon_ = nullptr;
    MyEngine::Camera*         camera_         = nullptr;

    std::vector<ItemVisual> items_;

    // key = マップパス、value = 取得済みマスの集合（(x, y) を uint32 にパック）
    std::unordered_map<std::string, std::unordered_set<uint32_t>> pickedItems_;

    /// <summary>
    /// Pack Idx処理を実行します。
    /// </summary>
    /// <param name="x">X方向のインデックスまたは値。</param>
    /// <param name="y">Y方向のインデックスまたは値。</param>
    /// <returns>計算または取得した数値。</returns>
    static inline uint32_t PackIdx(uint32_t x, uint32_t y) { return (y << 16) | x; }

    std::vector<PickupEffect> pickupEffects_;
};
