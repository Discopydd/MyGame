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
#include <unordered_map>
#include <unordered_set>

#include "TitleScene.h"
#include "GameClearManager.h"
#include "GameOverManager.h"
#include "../UI/CoinUIManager.h"
#include "../fade/IntroManager.h"
#include "../fade/FadeManager.h"

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

    static constexpr float LOADING_DURATION = 0.5f; // 1秒

    Sprite* portalHintSprite_ = nullptr;    // 传送提示图标精灵


    // 传送门触发：等待到黑后再开始加载
    bool        pendingPortalLoad_ = false;
    std::string pendingPortalMapPath_;
    Vector3     pendingPortalStartPos_;

    // === GameClear / 回标题用的标志 ===
    bool pendingGameClear_ = false;   // 按 E 触发通关时，用来等黑幕到纯黑再进入胜利演出
    bool returnToTitle_ = false;   // 在胜利画面按 Space 后，黑幕淡出回 Title

    // ===== HP 3D条段 =====
    std::vector<Object3d*> hpStrips_;
    int   hpSegments_ = 10;     // 总段数（10格血）
    int   hpVisibleCount_ = 10;     // 本帧可见段数（随HP变化）
    float hpInsetX_ = 40.0f;  // 屏幕左侧内边距（像素）
    float hpInsetY_ = 50.0f; // 屏幕上侧内边距（像素）→数值越大越靠下
    float hpSegPixelW_ = 45.0f;  // 每段在屏幕横向占用（像素）
    float hpGapPixel_ = 4.0f;   // 段间距（像素）
    float hpNdcZ_ = 0.08f;  // 贴近相机，避免被遮挡s


    float   irisBaseHoleRadiusPx_ = 860.0f;

    struct HintSprite {
        Sprite* sprite = nullptr;
        Vector3 worldPos{};
    };

    HintSprite spaceHint_;
    HintSprite shiftHint_;
    HintSprite sprintHint_;
    std::vector<HintSprite> upHints_;
    // Hub 指引进度：0=去 map3, 1=去 map4, 2=去 map5, 3=去 map6, 4=全部完成
    int hubGuideStage_ = 0;
    // 提示图标上下浮动
    float hintBobTime_ = 0.0f;
    float hintBobAmplitude_ = 6.0f;   // 位移像素（上下±6）
    float hintBobSpeed_ = 3.0f;   // 频率（越大晃得越快）
    // ==== 道具渲染节点容器 ====
    struct ItemVisual { uint32_t x, y; Object3d* obj; };
    std::vector<ItemVisual> items_;

    // ==== 当前地图路径（用于做 key） ====
    std::string currentMapPath_;

    // ==== 跨地图持久状态：每张地图被拾取过的道具格索引 ====
    // key = 地图路径，val = 已拾取的格子集合（把 (x,y) 打包成 uint32）
    std::unordered_map<std::string, std::unordered_set<uint32_t>> pickedItems_;
  
    // 小工具：把 (x,y) 打包/拆包
    static inline uint32_t PackIdx(uint32_t x, uint32_t y) { return (y << 16) | x; }

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

};
