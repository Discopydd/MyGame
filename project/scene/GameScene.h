#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelManager.h"
#include "MyMath.h"
#include "ImGuiManager.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "SoundManager.h"
#include "Camera.h"
#include <vector>
#include "BaseScene.h"
#include <map/MapChipField.h>
#include <player/Player.h>
#include <player/PlayerCamera.h>
struct PortalInfo {
    MapChipField::IndexSet index;  // 传送门格子索引
    std::string targetMap;         // 目标地图路径
    Vector3 targetStartPos;        // 玩家在目标地图的起点
};
class GameScene : public BaseScene {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

    void StartLoadingMap(const std::string& mapPath, const Vector3& startPos, bool isPortal);

private:
    WinApp* winApp_ = nullptr;
    DirectXCommon* dxCommon_ = nullptr;
    Input* input_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;
    ImGuiManager* imguiManager_ = nullptr;
    Object3dCommon* object3dCommon_ = nullptr;
    Camera* camera_ = nullptr;
    PlayerCamera* playerCamera_ = nullptr;

    Vector2 rotation_{};

    //Map
    MapChipField mapChipField_;              // 地图数据管理对象
    std::vector<Object3d*> mapBlocks_;       // 存储地图方块对象
    void GenerateBlocks();
    void LoadMap(const std::string& mapPath, const Vector3& startPos);
    Player* player_ = nullptr;

    Sprite* skillSprite_ = nullptr;          // 技能图标精灵
    Sprite* grayOverlaySprite_ = nullptr;    // 灰色遮罩精灵

    std::vector<PortalInfo> portals_;
    bool wasOnPortal_ = false;
    std::string nextMapToLoad_;    // 记录下一帧要加载的地图路径
    Vector3 nextMapStartPos_;      // 记录玩家在新地图的起点

    // ================== 加载相关 ==================
    bool shouldStartLoading_ = true;     // 延迟初始化加载
    bool isMapLoading_ = false;          // 初始化加载标志
    bool isPortalLoading_ = false;       // 传送门加载标志

    std::string portalMapPath_;          // 传送门目标地图
    Vector3 portalStartPos_;             // 传送门起点
    float portalLoadingTimer_ = 0.0f;    // 传送门计时
    float loadingTimer_ = 0.0f;          // 初始化加载计时
    bool loadingStarted_ = false;        // 延迟一帧标志

    static constexpr float LOADING_DURATION = 1.0f; // 1秒

    Sprite* portalHintSprite_ = nullptr;    // 传送提示图标精灵
};
