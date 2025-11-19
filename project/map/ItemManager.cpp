#include "ItemManager.h"

void ItemManager::Initialize(Object3dCommon* objCommon, Camera* camera)
{
    ClearVisuals();
    object3dCommon_ = objCommon;
    camera_         = camera;
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

void ItemManager::RegisterItem(const std::string& mapPath, uint32_t x, uint32_t y, Object3d* obj)
{
    ItemVisual v;
    v.x   = x;
    v.y   = y;
    v.obj = obj;
    items_.push_back(v);
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
}

void ItemManager::Draw3D()
{
    for (auto& v : items_) {
        if (v.obj) {
            v.obj->Draw();
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
                delete it->obj;
                it->obj = nullptr;
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
    for (auto& v : items_) {
        if (v.obj) {
            delete v.obj;
            v.obj = nullptr;
        }
    }
    items_.clear();
}
