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

    // ===== 画面淡入淡出 =====
    enum class FadePhase { None, FadingOut, LoadingHold, FadingIn };
    FadePhase fadePhase_ = FadePhase::None;

    Sprite* fadeSprite_ = nullptr;   // 全屏黑幕
    float   fadeAlpha_ = 0.0f;      // 0透明 -> 1全黑
    float   fadeSpeed_ = 0.16f;     // 淡速（可调）

    // 传送门触发：等待到黑后再开始加载
    bool        pendingPortalLoad_ = false;
    std::string pendingPortalMapPath_;
    Vector3     pendingPortalStartPos_;

    // === GameClear / 回标题用的标志 ===
    bool pendingGameClear_ = false;   // 按 E 触发通关时，用来等黑幕到纯黑再进入胜利演出
    bool returnToTitle_ = false;   // 在胜利画面按 Space 后，黑幕淡出回 Title

    // ====== Intro (开场演出) ======
    enum class IntroState { None, BarsIn, OrbitZoom, TitleShow, BarsOut, Done };
    IntroState introState_ = IntroState::None;
    float      introT_ = 0.0f;          // 当前阶段计时
    bool       introSkippable_ = true;  // 允许按键跳过
    bool       introStarted_ = false;   // 防止重复启动

    // 电影黑边&UI
    Sprite* letterboxTop_ = nullptr;
    Sprite* letterboxBottom_ = nullptr;
    Sprite* vignette_ = nullptr;        // 暗角
    Sprite* introTitle_ = nullptr;      // 开场大字
    Sprite* skipHint_ = nullptr;        // "Press Any Key to Skip"

    // 相机演出参数
    Vector3 camStartPos_{ 0, 12, -85 };   // 初始远景
    Vector3 camTargetPos_{ 0,  8, -38 };  // 结束近景
    Vector3 camPivot_{ 0, 0, 0 };         // 围绕的中心(稍后以玩家位置为基准设定)
    float   camOrbitDeg_ = 0.0f;          // 环绕角度

    // 屏幕震动
    float shakeTime_ = 0.0f;
    float shakeAmp_ = 0.0f;
    void  ApplyScreenShake_(Vector3& camPos);

    // Easing
    float EaseOutCubic_(float t) { return 1.0f - powf(1.0f - t, 3.0f); }
    float EaseInOutSine_(float t) { return 0.5f * (1.0f - cosf(3.1415926f * t)); }

    // Intro 驱动
    void StartIntro_();
    void UpdateIntro_(float dt);
    void DrawIntro_();

    // 是否需要播放开场演出
    bool playIntroOnThisMap_ = false;

    // ===== HP 3D条段 =====
    std::vector<Object3d*> hpStrips_;
    int   hpSegments_ = 10;     // 总段数（10格血）
    int   hpVisibleCount_ = 10;     // 本帧可见段数（随HP变化）
    float hpInsetX_ = 40.0f;  // 屏幕左侧内边距（像素）
    float hpInsetY_ = 50.0f; // 屏幕上侧内边距（像素）→数值越大越靠下
    float hpSegPixelW_ = 45.0f;  // 每段在屏幕横向占用（像素）
    float hpGapPixel_ = 4.0f;   // 段间距（像素）
    float hpNdcZ_ = 0.08f;  // 贴近相机，避免被遮挡s

    bool overlayPushed_ = false;  // 是否已叠加 LoadingScene
    bool reachedBlack_ = false;  // 是否已达到纯黑（刚到1.0的那一帧）
    int  blackHoldFrames_ = 0;    // 纯黑保留帧数

    enum class GameOverState { None, Dying, FadeOut, BlackHold, ShowTitle, Done };
    GameOverState gameOverState_ = GameOverState::None;
    float  gameOverT_ = 0.0f;

    // Game Over 标题
    Sprite* gameOverSprite_ = nullptr;
    Vector2 gameOverSize_ = { 500.0f, 300.0f }; // 你这张PNG大约500x300，请按实际资源调整
    Vector2 gameOverPos_;       // 动画中的位置（屏幕像素）
    Vector2 gameOverStartPos_;  // y在屏幕外上方
    Vector2 gameOverEndPos_;    // 屏幕中央
    float   gameOverSlideTime_ = 0.65f;
    // 启动/更新/绘制
    void StartGameOver_();
    void UpdateGameOver_(float dt);
    void DrawGameOver_();

    // ===== Game Clear =====
    enum class GameClearState { None, SlideTitle, PlayerShow, Done };
    GameClearState gameClearState_ = GameClearState::None;
    float  gameClearT_ = 0.0f;

    // Game Clear 标题
    Sprite* gameClearSprite_ = nullptr;
    Vector2 gameClearSize_ = { 1280.0f, 720.0f };   // 先和 GameOver 一样，有需要再改
    Vector2 gameClearPos_;
    Vector2 gameClearStartPos_;
    Vector2 gameClearEndPos_;
    float   gameClearSlideTime_ = 0.65f;

    // GameClear 用的玩家模型（独立于实际玩家，用来在画面中翻跟头）
    Vector3   clearPlayerStartPos_{};
    Object3d* clearPlayerObj_ = nullptr;
    Vector3   clearPlayerBasePos_{};
    float     clearPlayerSpinT_ = 0.0f;

    void StartGameClear_();
    void UpdateGameClear_(float dt);
    void DrawGameClear_();

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
    // Coin UI 灯光闪烁计时
    float coinUiLightTime_ = 0.0f;
    // ==== 道具渲染节点容器 ====
    struct ItemVisual { uint32_t x, y; Object3d* obj; };
    std::vector<ItemVisual> items_;

    // ==== 当前地图路径（用于做 key） ====
    std::string currentMapPath_;

    // ==== 跨地图持久状态：每张地图被拾取过的道具格索引 ====
    // key = 地图路径，val = 已拾取的格子集合（把 (x,y) 打包成 uint32）
    std::unordered_map<std::string, std::unordered_set<uint32_t>> pickedItems_;
    // ==== Coin 计数 UI（右上角）====
   // 使用 coin 模型 + colon.png + 0.png~9.png
    Object3d* coinUiObj_ = nullptr;          // 右上角显示的 coin 模型
    Sprite* coinColonSprite_ = nullptr;    // 冒号 ":"
    Sprite* coinDigitSprites_[3] = { nullptr, nullptr, nullptr }; // 最多 3 位数字（0~999）

    int totalCoinCollected_ = 0;             // ★ 跨地图「总共拾取的 coin 数」
    int coinCount_ = 0;             // 当前 UI 显示的数值（= totalCoinCollected_）
    int lastCoinCount_ = -1;            // 上一帧的数值（保留给需要时使用）

    // 刷新 coin 剩余数量 UI（重设位置与贴图）
    void UpdateCoinCountUI_();
    // 小工具：把 (x,y) 打包/拆包
    static inline uint32_t PackIdx(uint32_t x, uint32_t y) { return (y << 16) | x; }

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
};
