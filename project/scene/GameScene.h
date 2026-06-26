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
#include <deque>

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

class GameScene : public MyEngine::BaseScene {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;
    void UpdateInitialization() override;
    bool IsInitializationComplete() const override;

    void StartLoadingMap(const std::string& mapPath, const MyEngine::Vector3& startPos, bool isPortal);

private:
    MyEngine::WinApp*         winApp_         = nullptr;
    MyEngine::DirectXCommon*  dxCommon_       = nullptr;
    MyEngine::Input*          input_          = nullptr;
    MyEngine::SpriteCommon*   spriteCommon_   = nullptr;
    MyEngine::SrvManager*     srvManager_     = nullptr;

    // ===== GameScene 所有するオブジェクト群: unique_ptr で管理 =====
    std::unique_ptr<MyEngine::Sprite>       backgroundSprite_;
    std::unique_ptr<MyEngine::ImGuiManager> imguiManager_;
    std::unique_ptr<MyEngine::Object3dCommon> object3dCommon_;
    std::unique_ptr<MyEngine::Camera>       camera_;
    std::unique_ptr<PlayerCamera> playerCamera_;
    std::unique_ptr<Player>       player_;

    MyEngine::Vector2 rotation_{};

    // --- Map ---
    MapChipField mapChipField_;  // 地図データ本体（値セマンティクス）

    // マップブロック / 水ブロックの 3D モデル: GameScene が所有
    std::vector<std::unique_ptr<MyEngine::Object3d>> mapBlocks_;
    std::vector<std::unique_ptr<MyEngine::Object3d>> waterBlocks_;

    enum class PendingSpawnKind {
        Block,
        Block2,
        Portal,
        Item,
        Spike,
        Water,
        Enemy,
        MoveHorizontal,
        MoveVertical,
    };

    struct PendingSpawn {
        PendingSpawnKind kind = PendingSpawnKind::Block;
        MyEngine::Vector3 position{};
        uint32_t x = 0;
        uint32_t y = 0;
        uint8_t subID = 0;
        uint32_t length = 1;
    };

    void GenerateBlocks();
    void BuildPendingMapSpawns();
    void ProcessPendingMapSpawns(size_t spawnBudget);
    bool IsMapBuildComplete() const;
    void FinishMapLoading(const MyEngine::Vector3& startPos);
    void LoadMap(const std::string& mapPath, const MyEngine::Vector3& startPos);
    void HandlePlayerOnMovingPlatforms();
    void SyncLoadedSceneForReveal();
    bool CanUsePortalOnCurrentMap_() const;

    // UI / マネージャ
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

    // ================== ロード関連 ==================
    bool shouldStartLoading_ = true;   // 遅延初期化ロード
    bool isMapLoading_       = false;  // 初期ロードフラグ
    bool isPortalLoading_    = false;  // 転送門ロードフラグ

    std::string portalMapPath_;        // 転送門目標マップ
    MyEngine::Vector3     portalStartPos_;       // 転送門開始位置
    float       portalLoadingTimer_ = 0.0f;  // 転送門タイマー
    float       loadingTimer_       = 0.0f;  // 初期ロードタイマー
    std::deque<PendingSpawn> pendingMapSpawns_;
    bool        isIncrementalMapLoading_ = false;
    bool        loadPrepared_ = false;
    int         postLoadSettleFrames_ = 0;
    bool        pendingRevealAfterLoad_ = false;

    static constexpr float LOADING_DURATION = 0.5f; // 0.5秒
    static constexpr size_t kMapSpawnBudgetPerFrame = 24;
    static constexpr int kPostLoadSettleFrames = 3;

    // 転送門トリガー: 黒くなってからロードを開始するのを待つ
    bool        pendingPortalLoad_ = false;
    std::string pendingPortalMapPath_;
    MyEngine::Vector3     pendingPortalStartPos_;

    // === GameClear / タイトルへ戻るためのフラグ ===
    bool pendingGameClear_ = false;  // E でクリアをトリガーした時、黒幕が完全な黒になるのを待ってから勝利演出へ入るために使う
    bool returnToTitle_    = false;  // 勝利画面で Space を押した後、黒幕フェードアウトで Title へ戻る

    // === Victory condition ===
    bool bossDefeated_   = false;  // Boss 撃破後にクリアを1回だけトリガーする

    // ===== HP 3D バー =====
    float hpNdcZ_ = 0.08f;  // カメラに近づけ、遮蔽されるのを防ぐ

    // ===== ヒントアイコン（HintSprite 自身は「ビュー」だけであり、MyEngine::Sprite の所有権はすべて GameScene 側にある）=====
    HintSprite spaceHint_;
    HintSprite shiftHint_;
    HintSprite sprintHint_;
    std::vector<HintSprite> upHints_;

    // Hub の誘導進捗: 0=map3 へ、1=map4 へ、2=map5 へ、3=map6 へ、4=すべて完了
    int hubGuideStage_ = 0;

    // ==== 現在のマップパス（key / Hub ロジック用）====
    std::string currentMapPath_;

    // === Coin 集計 ===
    int totalCoinCollected_ = 0;

    // ==== Hub（map2）解放進捗 ====
    // 0: 第1ステージ入口だけ解放
    // 1: 第2ステージまで解放
    // 2: 第3ステージまで解放
    // 3: 最終ステージ入口まで解放
    // 4: 全ステージクリア
    int  hubProgress_      = 0;
    bool allStagesCleared_ = false;

    // 各サブマップがどの関に対応するか（0〜3）
    std::unordered_map<std::string, int> hubStageByMap_;

    // ===== パーティクル関連 =====
    // GameScene は ParticleManager を所有し、ライフサイクルは unique_ptr で管理する
    std::unique_ptr<ParticleManager> particleMgr_;

    // エミッタは ParticleManager が生成・所有し、ここでは「借用」した生ポインタを保持するだけで delete しない
    ParticleEmitter* emitter2D_        = nullptr;   // 2D（MyEngine::Sprite）粒子エミッタ
    ParticleEmitter* emitter3D_        = nullptr;   // 3D（MyEngine::Model）粒子エミッタ
    ParticleEmitter* dashStarEmitter_  = nullptr;
    ParticleEmitter* windEmitter_      = nullptr;   // 風エフェクト用のパーティクルエミッタ
    float            windSpawnTimer_   = 0.0f;
    ParticleEmitter* snowEmitter_      = nullptr;
    float            snowSpawnTimer_   = 0.0f;

    // プレイヤー位置履歴（〜用トゲまで戻す 1 秒前所在格子）
    static inline const int kPlayerIndexHistoryFrameCount_ = 30;
    MapChipField::IndexSet playerIndexHistory_[kPlayerIndexHistoryFrameCount_]{};
    int  playerIndexHistoryCursor_         = 0;
    bool playerIndexHistoryInitialized_    = false;
    MapChipField::IndexSet playerIndexOneSecAgo_{};

    MyEngine::Vector3 prevCameraPos_{};

    // 移動床
    std::vector<std::unique_ptr<MovingPlatform>> movingPlatforms_;
    float movingPlatformSpeed_ = 10.0f;

    bool crushedByPlatformThisFrame_ = false;
    bool damagedByEnemyThisFrame_    = false;

    // このフレームプレイヤーにダメージを与えた敵（被ダメージ時に軽く「分離」し、敵の中で吹き飛び / 再度埋まるのを防ぐために使う）
    Enemy* damageSourceEnemy_ = nullptr;

    // 踏みつけロック: 同じ敵に対しては、プレイヤーがその当たり判定から離れるまで再判定しない（頭上での棒立ち無限踏みを防ぐ）
    Enemy* stompLockEnemy_ = nullptr;


    // ================== Boss HP（2D） ==================
    std::unique_ptr<MyEngine::Sprite> bossHpDamageSprite_; // 赤: 遅れて減る HP バー
    std::unique_ptr<MyEngine::Sprite> bossHpSprite_;       // 緑: 即時 HP バー

    MyEngine::Vector2 bossHpBarPos_{};   // 左上座標（画面ピクセル）
    MyEngine::Vector2 bossHpBarSize_{};  // 満タン時のサイズ（画面ピクセル）

    float bossHpRatio_      = 1.0f; // 緑バーの割合
    float bossDamageRatio_  = 1.0f; // 赤バーの割合（ゆっくり緑へ追従）
    float bossDamageDropSpeed_ = 0.45f; // 毎秒の低下速度（0~1）
    bool  bossHpVisible_    = false;


    // ================== Boss トリガー演出（Boss へのカメラ寄せ + 名前表示 + プレイヤーへ戻る） ==================
    enum class BossIntroPhase {
        None,
        ToBoss,      // カメラをプレイヤーから Boss へ寄せる
        ShowName,    // 画面上端に Boss 名を表示
        BackToPlayer // カメラ戻るプレイヤー
    };

    BossIntroPhase bossIntroPhase_ = BossIntroPhase::None;
    float   bossIntroTimer_ = 0.0f;

    // トリガー瞬間: プレイヤー追従カメラの開始位置を記録
    MyEngine::Vector3 bossIntroStartCamPos_{};
    float   bossIntroStartFovY_ = 0.45f;

    // プレイヤーへ戻る段階: 戻り開始位置を記録
    MyEngine::Vector3 bossIntroBackStartCamPos_{};
    float   bossIntroBackStartFovY_ = 0.45f;

    // 対象 Boss（借用ポインタ。ライフサイクルは enemies_ 側で管理）
    BossEnemy* introBoss_ = nullptr;

    // —— 調整可能なパラメータ（より映画的にしたい場合はここをそのまま調整）——
    float bossIntroBossFovY_   = 0.28f;            // Boss へ寄せた時の FOV（小さいほど近い）
    float bossIntroBossZ_      = -25.0f;           // Boss へ寄せた時のカメラ Z（0 に近いほど近い）
    // ★ カメラ寄せ / 名前表示時: カメラ全体を少し下へずらす
    // 注意: このプロジェクトのワールド座標 Y 軸は「下が正」のようなので、「下へ」は +Y を使う。
    // もしシーンが「上が正」の場合は、この値を負数へ変更すればよい。
    MyEngine::Vector3 bossIntroBossCamOffset_ = {0.0f, 0.0f, 0.0f};

    float bossIntroToBossDur_  = 0.85f;            // カメラ寄せ時間
    float bossIntroShowDur_    = 1.10f;            // 名前表示の停止時間
    float bossIntroBackDur_    = 0.85f;            // プレイヤーへ戻る時間

    // Boss 名称（MyEngine::Sprite テクスチャ）
    std::unique_ptr<MyEngine::Sprite> bossNameSprite_;
    bool bossNameVisible_ = false;

    void StartBossIntro(BossEnemy* boss);
    void UpdateBossIntro(float dt);

    BossEnemy* FindBossEnemy();

    // カメラ目標点をマップ境界内に制限する（★ 先に目標を制約してから補間することで、境界沿いの滑りが滑らかになる）
    MyEngine::Vector3 ConstrainCameraToMap(const MyEngine::Vector3& desiredPos, float fovY, float cameraZ) const;

    // ================== Pause Menu（ESC） ==================
    bool isPaused_ = false;
    int  pauseCursor_ = 0; // 0: Continue, 1: Back to Title
    // 継続 / タイトルへ戻るボタン（通常 / 選択中）
    std::unique_ptr<MyEngine::Sprite> pauseContinueNormal_;
    std::unique_ptr<MyEngine::Sprite> pauseContinueSelected_;
    std::unique_ptr<MyEngine::Sprite> pauseBackNormal_;
    std::unique_ptr<MyEngine::Sprite> pauseBackSelected_;
    // 一時停止時の暗色オーバーレイ（FadeManager を流用せず、状態干渉を避ける）
    std::unique_ptr<MyEngine::Sprite> pauseDimSprite_;

    enum class DeferredInitPhase {
        None,
        UiSprites,
        CoreSystems,
        ModelWarmup,
        GameplayManagers,
        InitialMapPrepare,
        InitialMapBuild,
        Complete
    };

    void InitializeUiSprites_();
    void InitializeCoreSystems_();
    void InitializeGameplayManagers_();

    DeferredInitPhase deferredInitPhase_ = DeferredInitPhase::None;
    bool initComplete_ = false;
    size_t deferredModelLoadCursor_ = 0;
    static constexpr size_t kInitModelLoadsPerFrame = 3;

    // 敵
    std::vector<std::unique_ptr<Enemy>> enemies_;
};
