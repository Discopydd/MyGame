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
#include "SoundManager.h"
#include "Camera.h"
#include <vector>
#include "BaseScene.h"
#include <map/MapChipField.h>
#include <player/Player.h>
#include <player/PlayerCamera.h>
#include <unordered_map>
#include <unordered_set>

#include "TitleScene.h"
#include "GameClearManager.h"
#include "GameOverManager.h"
#include "../UI/CoinUIManager.h"
#include "../fade/IntroManager.h"
#include "../fade/FadeManager.h"
#include "../UI/HPBar3DManager.h"
#include "../UI/HintUIManager.h"
#include "../map/ItemManager.h"
#include "../UI/DashUIManager.h" 
#include "../map/PortalManager.h" 
#include "../particle/ParticleManager.h" 
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

    DashUIManager* dashUI_ = nullptr;

     // 传送门管理器
    PortalManager* portalMgr_ = nullptr;

    // ================== 加载相关 ==================
    bool shouldStartLoading_ = true;     // 延迟初始化加载
    bool isMapLoading_ = false;          // 初始化加载标志
    bool isPortalLoading_ = false;       // 传送门加载标志

    std::string portalMapPath_;          // 传送门目标地图
    Vector3 portalStartPos_;             // 传送门起点
    float portalLoadingTimer_ = 0.0f;    // 传送门计时
    float loadingTimer_ = 0.0f;          // 初始化加载计时

    static constexpr float LOADING_DURATION = 0.5f; // 1秒

    // 传送门触发：等待到黑后再开始加载
    bool        pendingPortalLoad_ = false;
    std::string pendingPortalMapPath_;
    Vector3     pendingPortalStartPos_;

    // === GameClear / 回标题用的标志 ===
    bool pendingGameClear_ = false;   // 按 E 触发通关时，用来等黑幕到纯黑再进入胜利演出
    bool returnToTitle_ = false;   // 在胜利画面按 Space 后，黑幕淡出回 Title

    // ===== HP 3D条管理器 =====
    HPBar3DManager* hpBar_ = nullptr;
    float hpNdcZ_ = 0.08f;  // 贴近相机，避免被遮挡

    HintSprite spaceHint_;
    HintSprite shiftHint_;
    HintSprite sprintHint_;
    std::vector<HintSprite> upHints_;
    // Hub 指引进度：0=去 map3, 1=去 map4, 2=去 map5, 3=去 map6, 4=全部完成
    int hubGuideStage_ = 0;
    // 提示图标管理器
    HintUIManager* hintUI_ = nullptr;
   // ==== 道具管理器 ====
    ItemManager* itemMgr_ = nullptr;

    // ==== 当前地图路径（用于做 key / Hub 逻辑）====
    std::string currentMapPath_;

    // === Coin UI 管理器 ===
    CoinUIManager* coinUI_ = nullptr;

    // 总共拾取的 coin 数保持在 GameScene（逻辑用）
    int totalCoinCollected_ = 0;


    // ==== Hub（map2）解锁进度 ====
   // 0: 只解锁第1关入口
   // 1: 解锁到第2关
   // 2: 解锁到第3关
   // 3: 解锁到最终关入口
   // 4: 全部关卡通关
    int hubProgress_ = 0;
    bool allStagesCleared_ = false;

    // 每张子地图对应哪一关（0~3）
    std::unordered_map<std::string, int> hubStageByMap_;

    // === GameClear 管理器 ===
    GameClearManager* gameClear_ = nullptr;
    // === GameOver 管理器 ===
    GameOverManager* gameOver_ = nullptr;
    // === Intro 管理器 ===
    IntroManager* intro_ = nullptr;
    // === Fade 管理器 ===
    FadeManager* fade_ = nullptr;


    ParticleManager* particleMgr_ = nullptr;
    ParticleEmitter* emitter2D_ = nullptr;   // 2D（Sprite）粒子发射器
    ParticleEmitter* emitter3D_ = nullptr;   // 3D（Model）粒子发射器
};
