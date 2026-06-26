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
class ItemManager {
public:
    struct ItemVisual {
        uint32_t x = 0, y = 0;
        std::unique_ptr<MyEngine::Object3d> obj;
    };
    struct PickupEffect {
        std::unique_ptr<MyEngine::Object3d> obj;
        float     elapsed = 0.0f;                 // 再生経過時間（秒）
        float     duration = 0.35f;                // 総時間（秒）
        MyEngine::Vector3   velocity = { 0.0f, 0.0f, 0.0f }; // 上昇速度（ワールド単位/秒）
        float     rotateSpeedY = 0.0f;              // Y 軸まわりの回転速度（ラジアン/秒）
    };
    ItemManager() = default;
    ~ItemManager() = default;

    void Initialize(MyEngine::Object3dCommon* objCommon, MyEngine::Camera* camera);
    void Finalize();

    // 生成ブロック時用: このマップでそのセルが取得済みかどうか？
    bool CanSpawnItem(const std::string& mapPath, uint32_t x, uint32_t y) const;

    // MyEngine::Object3d 生成後にマネージャへ登録
    void RegisterItem(const std::string& mapPath, uint32_t x, uint32_t y, std::unique_ptr<MyEngine::Object3d> obj);

    // 毎フレーム更新: 回転して Update
    void Update(float dt);

    // 3D 描画
    void Draw3D();

    // プレイヤーがセル上に立った時に呼ぶ；アイテムを取得したら true を返す
    bool OnPlayerStepOnTile(const std::string& mapPath,
                            const MapChipField::IndexSet& playerIndex,
                            MapChipField& field,
                            Player* player);

    // マップ切替時は現在のマップの可視オブジェクトだけクリア（取得済み記録は保持）
    void ClearVisuals();

    // 現在読み込まれているマップで、まだ取得されていないコイン数を返す
    int GetRemainingItemCount() const;

private:
    MyEngine::Object3dCommon* object3dCommon_ = nullptr;
    MyEngine::Camera*         camera_         = nullptr;

    std::vector<ItemVisual> items_;

    // key = マップパス、value = 取得済みマスの集合（(x, y) を uint32 にパック）
    std::unordered_map<std::string, std::unordered_set<uint32_t>> pickedItems_;

    static inline uint32_t PackIdx(uint32_t x, uint32_t y) { return (y << 16) | x; }

    std::vector<PickupEffect> pickupEffects_;
};
