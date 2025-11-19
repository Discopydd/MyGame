#pragma once
#include <vector>
#include <string>
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Camera.h"
#include <map/MapChipField.h>
#include "MyMath.h"

// 原来在 GameScene 里的 PortalInfo 挪到这里
struct PortalInfo {
    MapChipField::IndexSet index;  // 传送门格子索引
    std::string targetMap;         // 目标地图路径
    Vector3 targetStartPos;        // 玩家在目标地图的起点
};

// 声明一下 GameScene.cpp 里的工具函数（实现还在 GameScene.cpp）
Vector3 WorldToScreen(const Vector3& worldPos, Camera* camera);

class PortalManager {
public:
    PortalManager() = default;
    ~PortalManager() = default;

    void Initialize(SpriteCommon* spriteCommon, Camera* camera);
    void Finalize();

    void ClearPortals();
    void AddPortal(const MapChipField::IndexSet& idx,
                   const std::string& targetMap,
                   const Vector3& startPos);

    // 返回当前所有门（给 GameScene 用来配置）
    const std::vector<PortalInfo>& GetPortals() const { return portals_; }

    // 玩家是否站在某个门上，若是返回该门指针，否则nullptr
    const PortalInfo* GetPortalAt(const MapChipField::IndexSet& playerIndex) const;

    // 更新“按E提示”图标（是否显示 + 位置）
    void UpdateHint(const MapChipField::IndexSet& playerIndex,
                    const Vector3& playerWorldPos,
                    bool canControl);

    // 绘制提示图标（GameScene::Draw 里调用）
    void DrawHint();

private:
    SpriteCommon* spriteCommon_ = nullptr;
    Camera*       camera_       = nullptr;

    Sprite* hintSprite_         = nullptr;  // “按E”图标
    std::vector<PortalInfo> portals_;
};
