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
#include "./map/MovingPlatform.h"
#include "../enemy/Enemy.h"

#include "../enemy/NormalEnemy.h"
#include "../enemy/BossEnemy.h"

#include <memory>

class GameScene : public BaseScene {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

    void StartLoadingMap(const std::string& mapPath, const Vector3& startPos, bool isPortal);

private:
    WinApp*         winApp_         = nullptr;
    DirectXCommon*  dxCommon_       = nullptr;
    Input*          input_          = nullptr;
    SpriteCommon*   spriteCommon_   = nullptr;
    SrvManager*     srvManager_     = nullptr;

    // ===== GameScene 自己拥有的对象们：用 unique_ptr 管理 =====
    std::unique_ptr<Sprite>       backgroundSprite_;
    std::unique_ptr<ImGuiManager> imguiManager_;
    std::unique_ptr<Object3dCommon> object3dCommon_;
    std::unique_ptr<Camera>       camera_;
    std::unique_ptr<PlayerCamera> playerCamera_;
    std::unique_ptr<Player>       player_;

    Vector2 rotation_{};

    // --- Map ---
    MapChipField mapChipField_;  // 地图数据本体（值语义）

    // 地图方块 / 水块 3D 模型：GameScene 拥有
    std::vector<std::unique_ptr<Object3d>> mapBlocks_;
    std::vector<std::unique_ptr<Object3d>> waterBlocks_;

    void GenerateBlocks();
    void LoadMap(const std::string& mapPath, const Vector3& startPos);
    void HandlePlayerOnMovingPlatforms();

    // UI / 管理器
    std::unique_ptr<DashUIManager>   dashUI_;
    std::unique_ptr<PortalManager>   portalMgr_;
    std::unique_ptr<HPBar3DManager>  hpBar_;
    std::unique_ptr<HintUIManager>   hintUI_;
    std::unique_ptr<ItemManager>     itemMgr_;
    std::unique_ptr<CoinUIManager>   coinUI_;
    std::unique_ptr<GameClearManager> gameClear_;
    std::unique_ptr<GameOverManager>  gameOver_;
    std::unique_ptr<IntroManager>     intro_;
    std::unique_ptr<FadeManager>      fade_;

    // ================== 加载相关 ==================
    bool shouldStartLoading_ = true;   // 延迟初始化加载
    bool isMapLoading_       = false;  // 初始化加载标志
    bool isPortalLoading_    = false;  // 传送门加载标志

    std::string portalMapPath_;        // 传送门目标地图
    Vector3     portalStartPos_;       // 传送门起点
    float       portalLoadingTimer_ = 0.0f;  // 传送门计时
    float       loadingTimer_       = 0.0f;  // 初始化加载计时

    static constexpr float LOADING_DURATION = 0.5f; // 0.5秒

    // 传送门触发：等待到黑后再开始加载
    bool        pendingPortalLoad_ = false;
    std::string pendingPortalMapPath_;
    Vector3     pendingPortalStartPos_;

    // === GameClear / 回标题用的标志 ===
    bool pendingGameClear_ = false;  // 按 E 触发通关时，用来等黑幕到纯黑再进入胜利演出
    bool returnToTitle_    = false;  // 在胜利画面按 Space 后，黑幕淡出回 Title

    // === Victory condition ===
    bool bossDefeated_   = false;  // Boss 被击败后只触发一次通关

    // ===== HP 3D 条 =====
    float hpNdcZ_ = 0.08f;  // 贴近相机，避免被遮挡

    // ===== 提示图标（HintSprite 自身只是“视图”，真正 Sprite 的所有权在 GameScene 里）=====
    HintSprite spaceHint_;
    HintSprite shiftHint_;
    HintSprite sprintHint_;
    std::vector<HintSprite> upHints_;

    // Hub 指引进度：0=去 map3, 1=去 map4, 2=去 map5, 3=去 map6, 4=全部完成
    int hubGuideStage_ = 0;

    // ==== 当前地图路径（用于做 key / Hub 逻辑）====
    std::string currentMapPath_;

    // === Coin 统计 ===
    int totalCoinCollected_ = 0;

    // ==== Hub（map2）解锁进度 ====
    // 0: 只解锁第1关入口
    // 1: 解锁到第2关
    // 2: 解锁到第3关
    // 3: 解锁到最终关入口
    // 4: 全部关卡通关
    int  hubProgress_      = 0;
    bool allStagesCleared_ = false;

    // 每张子地图对应哪一关（0~3）
    std::unordered_map<std::string, int> hubStageByMap_;

    // ===== 粒子系统 =====
    // GameScene 拥有 ParticleManager，用 unique_ptr 管理生命周期
    std::unique_ptr<ParticleManager> particleMgr_;

    // 发射器由 ParticleManager 创建并持有，这里只是“借用”裸指针，不 delete
    ParticleEmitter* emitter2D_        = nullptr;   // 2D（Sprite）粒子发射器
    ParticleEmitter* emitter3D_        = nullptr;   // 3D（Model）粒子发射器
    ParticleEmitter* dashStarEmitter_  = nullptr;
    ParticleEmitter* windEmitter_      = nullptr;   // 风特效的粒子发射器
    float            windSpawnTimer_   = 0.0f;
    ParticleEmitter* snowEmitter_      = nullptr;
    float            snowSpawnTimer_   = 0.0f;

    // 玩家位置历史（用于地刺回退到 1 秒前所在格子）
    static inline const int kPlayerIndexHistoryFrameCount_ = 30;
    MapChipField::IndexSet playerIndexHistory_[kPlayerIndexHistoryFrameCount_]{};
    int  playerIndexHistoryCursor_         = 0;
    bool playerIndexHistoryInitialized_    = false;
    MapChipField::IndexSet playerIndexOneSecAgo_{};

    Vector3 prevCameraPos_{};

    // 移动平台
    std::vector<std::unique_ptr<MovingPlatform>> movingPlatforms_;
    float movingPlatformSpeed_ = 10.0f;

    bool crushedByPlatformThisFrame_ = false;
    bool damagedByEnemyThisFrame_    = false;

    
    // ================== Boss HP（2D） ==================
    std::unique_ptr<Sprite> bossHpDamageSprite_; // 红色：延迟扣血条
    std::unique_ptr<Sprite> bossHpSprite_;       // 绿色：即时血条

    Vector2 bossHpBarPos_{};   // 左上角坐标（屏幕像素）
    Vector2 bossHpBarSize_{};  // 满血时的尺寸（屏幕像素）

    float bossHpRatio_      = 1.0f; // 绿色条比例
    float bossDamageRatio_  = 1.0f; // 红色条比例（缓慢追上绿色）
    float bossDamageDropSpeed_ = 0.45f; // 每秒下降速度（0~1）
    bool  bossHpVisible_    = false;


    // ================== Boss 触发演出（镜头推 Boss + 名字 + 回玩家） ==================
    enum class BossIntroPhase {
        None,
        ToBoss,      // 镜头从玩家推到 Boss（拉近）
        ShowName,    // 顶部显示 Boss 名字
        BackToPlayer // 镜头回到玩家
    };

    BossIntroPhase bossIntroPhase_ = BossIntroPhase::None;
    float   bossIntroTimer_ = 0.0f;

    // 触发瞬间：记录玩家跟随镜头的起点
    Vector3 bossIntroStartCamPos_{};
    float   bossIntroStartFovY_ = 0.45f;

    // 回玩家阶段：记录回拉起点
    Vector3 bossIntroBackStartCamPos_{};
    float   bossIntroBackStartFovY_ = 0.45f;

    // 目标 Boss（借用指针，生命周期由 enemies_ 管理）
    BossEnemy* introBoss_ = nullptr;

    // —— 可调参数（想要“更电影感”就改这里）——
    float bossIntroBossFovY_   = 0.28f;            // 推到 Boss 时的 FOV（更小=更近）
    float bossIntroBossZ_      = -25.0f;           // 推到 Boss 时相机 Z（更接近 0=更近）
    // ★ 推镜头/展示名时：整体镜头向下挪一点
    // 注意：你这个项目的世界坐标 Y 轴更像是“向下为正”，所以“向下”用 +Y。
    // 若你的场景是 Y 向上为正，把这个值改成负数即可。
    Vector3 bossIntroBossCamOffset_ = {0.0f, 0.0f, 0.0f};

    float bossIntroToBossDur_  = 0.85f;            // 推镜头时长
    float bossIntroShowDur_    = 1.10f;            // 显示名字停留时长
    float bossIntroBackDur_    = 0.85f;            // 回玩家时长

    // Boss 名字（Sprite 贴图）
    std::unique_ptr<Sprite> bossNameSprite_;
    bool bossNameVisible_ = false;

    void StartBossIntro(BossEnemy* boss);
    void UpdateBossIntro(float dt);

    BossEnemy* FindBossEnemy();

    // 把相机目标点限制在地图边界（★ 先约束目标，再插值，沿边界滑动更顺）
    Vector3 ConstrainCameraToMap(const Vector3& desiredPos, float fovY, float cameraZ) const;

    // 敌人
    std::vector<std::unique_ptr<Enemy>> enemies_;
};
