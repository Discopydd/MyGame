#pragma once
#include "Object3dCommon.h"
#include "Object3d.h"
#include "Camera.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <map/MapChipField.h>
#include <player/Player.h>

class ItemManager {
public:
    struct ItemVisual {
        uint32_t x = 0, y = 0;
        Object3d* obj = nullptr;
    };

    ItemManager() = default;
    ~ItemManager() = default;

    void Initialize(Object3dCommon* objCommon, Camera* camera);
    void Finalize();

    // 生成方块时用：当前格子在该地图是否已经被拾取过？
    bool CanSpawnItem(const std::string& mapPath, uint32_t x, uint32_t y) const;

    // 生成好 Object3d 后注册给管理器
    void RegisterItem(const std::string& mapPath, uint32_t x, uint32_t y, Object3d* obj);

    // 每帧更新：旋转 & Update
    void Update(float dt);

    // 3D 绘制
    void Draw3D();

    // 玩家站在某格子上时调用；如果拾取了道具返回 true
    bool OnPlayerStepOnTile(const std::string& mapPath,
                            const MapChipField::IndexSet& playerIndex,
                            MapChipField& field,
                            Player* player);

    // 换地图时，仅清掉当前地图的可见物体（保留已拾取记录）
    void ClearVisuals();

private:
    Object3dCommon* object3dCommon_ = nullptr;
    Camera*         camera_         = nullptr;

    std::vector<ItemVisual> items_;

    // key = 地图路径，val = 已拾取的格子集合（把 (x,y) 打包成 uint32）
    std::unordered_map<std::string, std::unordered_set<uint32_t>> pickedItems_;

    static inline uint32_t PackIdx(uint32_t x, uint32_t y) { return (y << 16) | x; }
};
