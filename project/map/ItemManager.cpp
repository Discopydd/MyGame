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
    (void)mapPath; // 这里只用来区分是否生成，真正记录在拾取时做
}

void ItemManager::Update(float dt)
{
    (void)dt;
    for (auto& v : items_) {
        if (!v.obj) continue;
        Vector3 rot = v.obj->GetRotate();
        rot.y += 0.05f;  // 旋转速度
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

        // 向上移动
        Vector3 pos = e.obj->GetTranslate();
        pos.x += e.velocity.x * dt;
        pos.y += e.velocity.y * dt;
        pos.z += e.velocity.z * dt;
        e.obj->SetTranslate(pos);

        // 快速旋转
        Vector3 rot = e.obj->GetRotate();
        rot.y += e.rotateSpeedY * dt;
        e.obj->SetRotate(rot);

        e.obj->Update();

        // 到时间就删掉
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
    // 当前格是否是道具格？
    if (field.GetMapChipTypeByIndex(playerIndex.xIndex, playerIndex.yIndex) != MapChipType::kItem) {
        return false;
    }

    uint32_t key = PackIdx(playerIndex.xIndex, playerIndex.yIndex);

    // 如果已经记过一次拾取，就直接返回
    auto& pickedSet = pickedItems_[mapPath];
    if (pickedSet.count(key)) {
        return false;
    }

    // 第一次拾取：登记
    pickedSet.insert(key);

    // 从 items_ 中找到对应格子的渲染体，删掉
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (it->x == playerIndex.xIndex && it->y == playerIndex.yIndex) {
            if (it->obj) {
                PickupEffect effect;
                effect.obj          = std::move(it->obj);
                effect.elapsed      = 0.0f;
                effect.duration     = 0.35f;                   // 播放 0.35 秒
                effect.velocity     = { 0.0f, 2.0f, 0.0f };    // 向上 2.0 单位/秒
                effect.rotateSpeedY = 25.0f;                   // 快速旋转（弧度/秒）

                pickupEffects_.push_back(std::move(effect));

            }
            items_.erase(it);
            break;
        }
    }

    // 可以在这里给 player 做一些效果（比如回血）
    if (player) {
        // TODO: player->Heal(...);
    }

    return true; // 告诉上层“这格被新拾取了”
}

void ItemManager::ClearVisuals()
{
    items_.clear();
    pickupEffects_.clear();
}
