// ItemManager.cpp
#include "ItemManager.h"

void ItemManager::Initialize(Object3dCommon* objCommon, Camera* camera)
{
    object3dCommon_ = objCommon;
    camera_         = camera;
    items_.clear();
    pickupEffects_.clear();
}

void ItemManager::Finalize()
{
    ClearVisuals();
    pickedItems_.clear();
}

bool ItemManager::CanSpawnItem(const std::string& mapPath, uint32_t x, uint32_t y) const
{
    uint32_t key = PackIdx(x, y);
    auto it = pickedItems_.find(mapPath);
    if (it == pickedItems_.end()) {
        return true;
    }
    return (it->second.count(key) == 0);
}

void ItemManager::RegisterItem(const std::string& mapPath, uint32_t x, uint32_t y, std::unique_ptr<Object3d> obj)
{
    ItemVisual v;
    v.x   = x;
    v.y   = y;
    v.obj = std::move(obj);
    items_.push_back(std::move(v));
    (void)mapPath; // ここでは生成済みかの判定にだけ使う、実際の記録は取得時に行う
}

void ItemManager::Update(float dt)
{
    (void)dt;
    for (auto& v : items_) {
        if (!v.obj) continue;
        Vector3 rot = v.obj->GetRotate();
        rot.y += 0.05f;  // 回転速度
        v.obj->SetRotate(rot);
        v.obj->Update();
    }
    for (auto it = pickupEffects_.begin(); it != pickupEffects_.end(); ) {
        PickupEffect& e = *it;
        if (!e.obj) {
            it = pickupEffects_.erase(it);
            continue;
        }

        e.elapsed += dt;

        // 上方向へ移動
        Vector3 pos = e.obj->GetTranslate();
        pos.x += e.velocity.x * dt;
        pos.y += e.velocity.y * dt;
        pos.z += e.velocity.z * dt;
        e.obj->SetTranslate(pos);

        // 高速回転
        Vector3 rot = e.obj->GetRotate();
        rot.y += e.rotateSpeedY * dt;
        e.obj->SetRotate(rot);

        e.obj->Update();

        // 時間になったら削除
        if (e.elapsed >= e.duration) {
            e.obj.reset();
            it = pickupEffects_.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ItemManager::Draw3D()
{
    for (auto& v : items_) {
        if (v.obj) {
            v.obj->Draw();
        }
    }
     for (auto& e : pickupEffects_) {
        if (e.obj) {
            e.obj->Draw();
        }
    }
}

bool ItemManager::OnPlayerStepOnTile(const std::string& mapPath,
                                     const MapChipField::IndexSet& playerIndex,
                                     MapChipField& field,
                                     Player* player)
{
    // 現在のセルがアイテムマスかどうか？
    if (field.GetMapChipTypeByIndex(playerIndex.xIndex, playerIndex.yIndex) != MapChipType::kItem) {
        return false;
    }

    uint32_t key = PackIdx(playerIndex.xIndex, playerIndex.yIndex);

    // すでに取得済みならそのまま返す
    auto& pickedSet = pickedItems_[mapPath];
    if (pickedSet.count(key)) {
        return false;
    }

    // 初回取得: 記録する
    pickedSet.insert(key);

    // items_ から対応セルの描画オブジェクトを見つけて削除
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (it->x == playerIndex.xIndex && it->y == playerIndex.yIndex) {
            if (it->obj) {
                PickupEffect effect;
                effect.obj          = std::move(it->obj);
                effect.elapsed      = 0.0f;
                effect.duration     = 0.35f;                   // 0.35 秒再生
                effect.velocity     = { 0.0f, 2.0f, 0.0f };    // 上方向へ 2.0 単位/秒
                effect.rotateSpeedY = 25.0f;                   // 高速回転（弧度/秒）

                pickupEffects_.push_back(std::move(effect));

            }
            items_.erase(it);
            break;
        }
    }

    // ここで player に効果を付与できる（例: 回復）
    if (player) {
        // TODO: player->Heal(...);
    }

    return true; // 上位へ通知「このマスが新たに取得された」
}

void ItemManager::ClearVisuals()
{
    items_.clear();
    pickupEffects_.clear();
}
