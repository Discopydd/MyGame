#include "GameScene.h"
#include <numbers>
#include <algorithm>
#include <cmath>
#include <scene/LoadingScene.h>
#include "SceneManager.h"
#include <cstdlib>
namespace {
    // 0.0f = ダメージ高さなし、1.0f = セル全体をトゲとして扱う
    constexpr float kSpikeHeightRatio = 0.5f;

    // Boss 演出中: プレイヤー位置を固定し、重力／慣性／衝突による移動を防ぐ
    bool    gBossIntroFreezePlayer = false;
    Vector3 gBossIntroFrozenPlayerPos{};
}

static float RandRangeFloat(float a, float b)
{
    float t = static_cast<float>(rand()) / RAND_MAX;
    return a + (b - a) * t;
}

static float SmoothStep01(float t)
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

static float LerpFloat(float a, float b, float t)
{
    return a + (b - a) * t;
}

static Vector3 LerpVec3(const Vector3& a, const Vector3& b, float t)
{
    return {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    };
}
// 3Dワールド座標を画面座標に変換
Vector3 WorldToScreen(const Vector3& worldPos, Camera* camera)
{
    // まず同次座標を構築
    float x = worldPos.x;
    float y = worldPos.y;
    float z = worldPos.z;
    float w = 1.0f;

    // VP 行列
    const Matrix4x4& vp = camera->GetViewprojectionMatrix();

    // クリップ空間へ変換
    float clipX = x * vp.m[0][0] + y * vp.m[1][0] + z * vp.m[2][0] + w * vp.m[3][0];
    float clipY = x * vp.m[0][1] + y * vp.m[1][1] + z * vp.m[2][1] + w * vp.m[3][1];
    float clipZ = x * vp.m[0][2] + y * vp.m[1][2] + z * vp.m[2][2] + w * vp.m[3][2];
    float clipW = x * vp.m[0][3] + y * vp.m[1][3] + z * vp.m[2][3] + w * vp.m[3][3];

    // 透視除法
    if (clipW != 0.0f)
    {
        clipX /= clipW;
        clipY /= clipW;
        clipZ /= clipW;
    }

    // NDC (-1~1) -> 画面座標
    float screenX = (clipX * 0.5f + 0.5f) * float(WinApp::kClientWidth);
    float screenY = (1.0f - (clipY * 0.5f + 0.5f)) * float(WinApp::kClientHeight);
    float ndcZ    = clipZ * 0.5f + 0.5f;
    return { screenX, screenY, ndcZ };
}
// 画面ピクセル座標(x,y)をワールド座標へ変換。ndcZ∈[0,1]: 0=ニアクリップ面、1=ファークリップ面
Vector3 ScreenToWorld(float screenX, float screenY, float ndcZ, Camera* camera)
{
    const Matrix4x4& vp = camera->GetViewprojectionMatrix();
    Matrix4x4 invVP = Math::Inverse(vp); // プロジェクト内の逆行列計算関数が必要

    float ndcX = (screenX / float(WinApp::kClientWidth)) * 2.0f - 1.0f;
    float ndcY = -(screenY / float(WinApp::kClientHeight)) * 2.0f + 1.0f; // Y反転に注意

    Vector3 world = Math::TransformCoordLocal(Vector3{ ndcX, ndcY, ndcZ }, invVP);
    return world;
}


namespace {
    const char* kDeferredInitModelPaths[] = {
        "cube/cube.obj",
        "player/player.obj",
        "door/Door.obj",
        "strip/strip.obj",
        "coin/coin.obj",
        "coin_ui/coin_ui.obj",
        "snow/snow.obj",
        "jump/jump.obj",
        "star/star.obj",
        "hurd/hurd.obj",
        "cube2/cube2.obj",
        "water/water.obj",
        "enemy0/enemy0.obj",
        "enemy1/enemy1.obj",
        "enemy2/enemy2.obj",
        "enemy3/enemy3.obj",
        "enemyBullet/enemyBullet.obj",
    };
}

bool GameScene::IsInitializationComplete() const
{
    return initComplete_;
}

void GameScene::InitializeUiSprites_()
{
    // === 背景 Sprite ===
    const std::string kSkyTexPath = "Resources/sky_bg.png";

    backgroundSprite_ = std::make_unique<Sprite>();
    backgroundSprite_->Initialize(spriteCommon_, kSkyTexPath);
    backgroundSprite_->SetPosition({ 0.0f, 0.0f });
    backgroundSprite_->SetSize({
        static_cast<float>(WinApp::kClientWidth),
        static_cast<float>(WinApp::kClientHeight)
    });
    backgroundSprite_->Update();

    // ================== Pause Menu（ESC） ==================
    {
        const std::string kContinueNormalTex  = "Resources/button_continue_game_q_2x.png";
        const std::string kBackNormalTex      = "Resources/button_back_to_title_q_2x.png";
        const std::string kContinueSelectTex  = "Resources/selected_continue_game_2x.png";
        const std::string kBackSelectTex      = "Resources/selected_back_to_title_2x.png";

        const Vector2 kBtnSize = { 416.0f / 2.0f, 168.0f / 2.0f };
        const float   kGapY = 18.0f;
        const float   totalH = kBtnSize.y * 2.0f + kGapY;
        const float   startY = (WinApp::kClientHeight - totalH) * 0.5f;
        const float   posX   = (WinApp::kClientWidth - kBtnSize.x) * 0.5f;
        const Vector2 continuePos = { posX, startY };
        const Vector2 backPos     = { posX, startY + kBtnSize.y + kGapY };

        pauseContinueNormal_ = std::make_unique<Sprite>();
        pauseContinueNormal_->Initialize(spriteCommon_, kContinueNormalTex);
        pauseContinueNormal_->SetPosition(continuePos);
        pauseContinueNormal_->SetSize(kBtnSize);

        pauseContinueSelected_ = std::make_unique<Sprite>();
        pauseContinueSelected_->Initialize(spriteCommon_, kContinueSelectTex);
        pauseContinueSelected_->SetPosition(continuePos);
        pauseContinueSelected_->SetSize(kBtnSize);

        pauseBackNormal_ = std::make_unique<Sprite>();
        pauseBackNormal_->Initialize(spriteCommon_, kBackNormalTex);
        pauseBackNormal_->SetPosition(backPos);
        pauseBackNormal_->SetSize(kBtnSize);

        pauseBackSelected_ = std::make_unique<Sprite>();
        pauseBackSelected_->Initialize(spriteCommon_, kBackSelectTex);
        pauseBackSelected_->SetPosition(backPos);
        pauseBackSelected_->SetSize(kBtnSize);

        isPaused_ = false;
        pauseCursor_ = 0;
    }

    {
        const std::string kPauseDimTex = "Resources/black.png";
        pauseDimSprite_ = std::make_unique<Sprite>();
        pauseDimSprite_->Initialize(spriteCommon_, kPauseDimTex);
        pauseDimSprite_->SetPosition({ 0.0f, 0.0f });
        pauseDimSprite_->SetSize({
            static_cast<float>(WinApp::kClientWidth),
            static_cast<float>(WinApp::kClientHeight)
        });
        pauseDimSprite_->SetColor({ 0.0f, 0.0f, 0.0f, 0.6f });
        pauseDimSprite_->SetVisible(true);
        pauseDimSprite_->Update();
    }

    const std::string kBossDamageBarTex = "Resources/Damagebar.png";
    const std::string kBossHpBarTex     = "Resources/HPbar.png";

    bossHpDamageSprite_ = std::make_unique<Sprite>();
    bossHpDamageSprite_->Initialize(spriteCommon_, kBossDamageBarTex);

    bossHpSprite_ = std::make_unique<Sprite>();
    bossHpSprite_->Initialize(spriteCommon_, kBossHpBarTex);

    bossHpBarSize_ = { 420.0f, 24.0f };
    bossHpBarPos_  = { (WinApp::kClientWidth - bossHpBarSize_.x) * 0.5f, 24.0f };

    bossHpDamageSprite_->SetPosition(bossHpBarPos_);
    bossHpDamageSprite_->SetSize(bossHpBarSize_);
    bossHpDamageSprite_->SetVisible(false);

    bossHpSprite_->SetPosition(bossHpBarPos_);
    bossHpSprite_->SetSize(bossHpBarSize_);
    bossHpSprite_->SetVisible(false);

    bossHpRatio_ = 1.0f;
    bossDamageRatio_ = 1.0f;
    bossHpVisible_ = false;

    const std::string kBossNameTex = "Resources/Boss_name.png";
    bossNameSprite_ = std::make_unique<Sprite>();
    bossNameSprite_->Initialize(spriteCommon_, kBossNameTex);
    const Vector2 bossNameSize = { 420.0f, 64.0f };
    const Vector2 bossNamePos  = { (WinApp::kClientWidth - bossNameSize.x) * 0.5f, 16.0f };
    bossNameSprite_->SetPosition(bossNamePos);
    bossNameSprite_->SetSize(bossNameSize);
    bossNameSprite_->SetVisible(false);
    bossNameVisible_ = false;
}

void GameScene::InitializeCoreSystems_()
{
    imguiManager_ = std::make_unique<ImGuiManager>();
    imguiManager_->Initialize(winApp_, dxCommon_, srvManager_);

    object3dCommon_ = std::make_unique<Object3dCommon>();
    object3dCommon_->Initialize(dxCommon_);

    ModelManager::GetInstants()->Initialize(dxCommon_);

    SoundManager* soundMgr = SoundManager::GetInstance();
    soundMgr->Initialize();
    soundMgr->LoadWav("fanfare", "resources/fanfare.wav");

    camera_ = std::make_unique<Camera>();
    camera_->SetRotate({ 0, 0, 0 });
    object3dCommon_->SetDefaultCamera(camera_.get());
}

void GameScene::InitializeGameplayManagers_()
{
    player_ = std::make_unique<Player>();
    player_->Initialize(object3dCommon_.get(), camera_.get());

    hpBar_ = std::make_unique<HPBar3DManager>();
    hpBar_->Initialize(object3dCommon_.get(), camera_.get(), player_.get(), hpNdcZ_);

    playerCamera_ = std::make_unique<PlayerCamera>();
    playerCamera_->Initialize(camera_.get(), player_.get(), &mapChipField_);
    playerCamera_->SetOffset({ 0, 0.0f, -40.0f });
    playerCamera_->SetFollowSpeed(0.1f);
    playerCamera_->SetConstrainToMap(true);

    prevCameraPos_ = camera_->GetTransform().translate;

    dashUI_ = std::make_unique<DashUIManager>();
    dashUI_->Initialize(spriteCommon_, player_.get());

    coinUI_ = std::make_unique<CoinUIManager>();
    coinUI_->Initialize(spriteCommon_, object3dCommon_.get(), camera_.get(), hpNdcZ_);
    coinUI_->SetTotalCoin(totalCoinCollected_);

    hintUI_ = std::make_unique<HintUIManager>();
    hintUI_->Initialize(spriteCommon_, camera_.get());
    hintUI_->SetSpaceHint(&spaceHint_);
    hintUI_->SetShiftHint(&shiftHint_);
    hintUI_->SetSprintHint(&sprintHint_);
    hintUI_->SetUpHints(&upHints_);

    itemMgr_ = std::make_unique<ItemManager>();
    itemMgr_->Initialize(object3dCommon_.get(), camera_.get());

    portalMgr_ = std::make_unique<PortalManager>();
    portalMgr_->Initialize(spriteCommon_, camera_.get());

    isMapLoading_ = false;
    loadingTimer_ = 0.0f;

    fade_ = std::make_unique<FadeManager>();
    fade_->Initialize(spriteCommon_);
    fade_->SetAlpha(1.0f);

    intro_ = std::make_unique<IntroManager>();
    intro_->Initialize(spriteCommon_, input_);

    gameOver_ = std::make_unique<GameOverManager>();
    gameOver_->Initialize(spriteCommon_);

    gameClear_ = std::make_unique<GameClearManager>();
    gameClear_->Initialize(spriteCommon_, object3dCommon_.get(), camera_.get(), hpNdcZ_);

    particleMgr_ = std::make_unique<ParticleManager>();
    particleMgr_->Initialize(object3dCommon_.get(), spriteCommon_);

    emitter2D_       = particleMgr_->CreateEmitter();
    emitter3D_       = particleMgr_->CreateEmitter();
    windEmitter_     = particleMgr_->CreateEmitter();
    snowEmitter_     = particleMgr_->CreateEmitter();
    dashStarEmitter_ = particleMgr_->CreateEmitter();

    if (windEmitter_) {
        windEmitter_->SetWindMode(true);
        windEmitter_->SetUseOriginalSpriteSize(true);
        windEmitter_->SetMaxParticles(40);
    }

    if (snowEmitter_) {
        snowEmitter_->SetSnowMode(true);
        snowEmitter_->SetMaxParticles(200);
        snowEmitter_->SetFollowCamera(true);
    }

    if (dashStarEmitter_) {
        dashStarEmitter_->SetMaxParticles(150);
        dashStarEmitter_->SetSnowMode(false);
        dashStarEmitter_->SetWindMode(false);
        dashStarEmitter_->SetFollowCamera(false);
    }

    hubStageByMap_.clear();
    hubStageByMap_["Resources/map/map3.csv"] = 0;
    hubStageByMap_["Resources/map/map4.csv"] = 1;
    hubStageByMap_["Resources/map/map5.csv"] = 2;
    hubStageByMap_["Resources/map/map6.csv"] = 3;
    hubProgress_      = 0;
    allStagesCleared_ = false;

    bossDefeated_ = false;
    playerIndexHistoryCursor_      = 0;
    playerIndexHistoryInitialized_ = false;
    playerIndexOneSecAgo_          = MapChipField::IndexSet{};
}

void GameScene::UpdateInitialization()
{
    switch (deferredInitPhase_) {
    case DeferredInitPhase::UiSprites:
        InitializeUiSprites_();
        deferredInitPhase_ = DeferredInitPhase::CoreSystems;
        break;

    case DeferredInitPhase::CoreSystems:
        InitializeCoreSystems_();
        deferredInitPhase_ = DeferredInitPhase::ModelWarmup;
        break;

    case DeferredInitPhase::ModelWarmup:
    {
        size_t loadedThisFrame = 0;
        const size_t kModelCount = sizeof(kDeferredInitModelPaths) / sizeof(kDeferredInitModelPaths[0]);
        while (deferredModelLoadCursor_ < kModelCount && loadedThisFrame < kInitModelLoadsPerFrame) {
            ModelManager::GetInstants()->LoadModel(kDeferredInitModelPaths[deferredModelLoadCursor_]);
            ++deferredModelLoadCursor_;
            ++loadedThisFrame;
        }
        if (deferredModelLoadCursor_ >= kModelCount) {
            deferredInitPhase_ = DeferredInitPhase::GameplayManagers;
        }
        break;
    }

    case DeferredInitPhase::GameplayManagers:
        InitializeGameplayManagers_();
        deferredInitPhase_ = DeferredInitPhase::InitialMapPrepare;
        break;

    case DeferredInitPhase::InitialMapPrepare:
        shouldStartLoading_ = false;
        LoadMap("Resources/map/map6.csv", { 2, 1, 0 });
        isIncrementalMapLoading_ = true;
        deferredInitPhase_ = DeferredInitPhase::InitialMapBuild;
        break;

    case DeferredInitPhase::InitialMapBuild:
        ProcessPendingMapSpawns(kMapSpawnBudgetPerFrame);
        if (IsMapBuildComplete()) {
            FinishMapLoading(Vector3{ 3, 3, 0 });
            if (player_) {
                player_->ResetForMapTransition(true);
            }
            initComplete_ = true;
            deferredInitPhase_ = DeferredInitPhase::Complete;
        }
        break;

    case DeferredInitPhase::Complete:
    case DeferredInitPhase::None:
    default:
        break;
    }
}

void GameScene::SyncLoadedSceneForReveal()
{
    if (player_) {
        player_->SetVelocity({ 0.0f, 0.0f, 0.0f });
    }

    if (playerCamera_) {
        playerCamera_->SnapToTarget();
    } else if (camera_) {
        camera_->Update();
    }

    for (auto& block : mapBlocks_) {
        if (block) {
            block->Update();
        }
    }
    for (auto& water : waterBlocks_) {
        if (water) {
            water->Update();
        }
    }

    if (itemMgr_) {
        itemMgr_->Update(0.0f);
    }
    if (hpBar_) {
        hpBar_->Update(0.0f);
    }
    if (hintUI_) {
        hintUI_->Update(0.0f);
    }
    if (dashUI_) {
        dashUI_->Update(0.0f);
    }
    if (coinUI_) {
        coinUI_->Update(0.0f);
    }
}

void GameScene::GenerateBlocks() {
    BuildPendingMapSpawns();
    while (!pendingMapSpawns_.empty()) {
        ProcessPendingMapSpawns(pendingMapSpawns_.size());
    }
}

void GameScene::BuildPendingMapSpawns()
{
    pendingMapSpawns_.clear();

    for (uint32_t y = 0; y < mapChipField_.numBlockVertical_; ++y) {
        for (uint32_t x = 0; x < mapChipField_.numBlockHorizontal_; ++x) {
            MapChipType type = mapChipField_.GetMapChipTypeByIndex(x, y);
            Vector3 position = mapChipField_.GetMapChipPositionByIndex(x, y);

            PendingSpawn spawn{};
            spawn.position = position;
            spawn.x = x;
            spawn.y = y;
            spawn.subID = mapChipField_.GetMapChipSubIDByIndex(x, y);

            switch (type) {
            case MapChipType::kBlock:
                spawn.kind = PendingSpawnKind::Block;
                pendingMapSpawns_.push_back(spawn);
                break;
            case MapChipType::kBlock2:
                spawn.kind = PendingSpawnKind::Block2;
                pendingMapSpawns_.push_back(spawn);
                break;
            case MapChipType::kPortal:
                spawn.kind = PendingSpawnKind::Portal;
                pendingMapSpawns_.push_back(spawn);
                break;
            case MapChipType::kItem:
                if (itemMgr_ && itemMgr_->CanSpawnItem(currentMapPath_, x, y)) {
                    spawn.kind = PendingSpawnKind::Item;
                    pendingMapSpawns_.push_back(spawn);
                }
                break;
            case MapChipType::kSpike:
                spawn.kind = PendingSpawnKind::Spike;
                pendingMapSpawns_.push_back(spawn);
                break;
            case MapChipType::kWater:
                spawn.kind = PendingSpawnKind::Water;
                pendingMapSpawns_.push_back(spawn);
                break;
            case MapChipType::kEnemy:
                spawn.kind = PendingSpawnKind::Enemy;
                pendingMapSpawns_.push_back(spawn);
                break;
            default:
                break;
            }
        }
    }

    for (uint32_t y = 0; y < mapChipField_.numBlockVertical_; ++y) {
        uint32_t x = 0;
        while (x < mapChipField_.numBlockHorizontal_) {
            if (mapChipField_.GetMapChipTypeByIndex(x, y) != MapChipType::kMoveHorizontal) {
                ++x;
                continue;
            }

            uint32_t startX = x;
            uint32_t endX = x;
            while (endX + 1 < mapChipField_.numBlockHorizontal_ &&
                mapChipField_.GetMapChipTypeByIndex(endX + 1, y) == MapChipType::kMoveHorizontal) {
                ++endX;
            }

            PendingSpawn spawn{};
            spawn.kind = PendingSpawnKind::MoveHorizontal;
            spawn.x = startX;
            spawn.y = y;
            spawn.length = endX - startX + 1;
            Vector3 leftPos = mapChipField_.GetMapChipPositionByIndex(startX, y);
            Vector3 rightPos = mapChipField_.GetMapChipPositionByIndex(endX, y);
            spawn.position = {
                (leftPos.x + rightPos.x) * 0.5f,
                leftPos.y,
                leftPos.z
            };
            pendingMapSpawns_.push_back(spawn);
            x = endX + 1;
        }
    }

    for (uint32_t y = 0; y < mapChipField_.numBlockVertical_; ++y) {
        uint32_t x = 0;
        while (x < mapChipField_.numBlockHorizontal_) {
            if (mapChipField_.GetMapChipTypeByIndex(x, y) != MapChipType::kMoveVertical) {
                ++x;
                continue;
            }

            uint32_t startX = x;
            uint32_t endX = x;
            while (endX + 1 < mapChipField_.numBlockHorizontal_ &&
                mapChipField_.GetMapChipTypeByIndex(endX + 1, y) == MapChipType::kMoveVertical) {
                ++endX;
            }

            PendingSpawn spawn{};
            spawn.kind = PendingSpawnKind::MoveVertical;
            spawn.x = startX;
            spawn.y = y;
            spawn.length = endX - startX + 1;
            Vector3 leftPos = mapChipField_.GetMapChipPositionByIndex(startX, y);
            Vector3 rightPos = mapChipField_.GetMapChipPositionByIndex(endX, y);
            spawn.position = {
                (leftPos.x + rightPos.x) * 0.5f,
                leftPos.y,
                leftPos.z
            };
            pendingMapSpawns_.push_back(spawn);
            x = endX + 1;
        }
    }
}

void GameScene::ProcessPendingMapSpawns(size_t spawnBudget)
{
    while (spawnBudget > 0 && !pendingMapSpawns_.empty()) {
        PendingSpawn spawn = pendingMapSpawns_.front();
        pendingMapSpawns_.pop_front();
        --spawnBudget;

        switch (spawn.kind) {
        case PendingSpawnKind::Block:
        {
            auto block = std::make_unique<Object3d>();
            block->Initialize(object3dCommon_.get());
            block->SetModel("cube/cube.obj");
            block->SetCamera(camera_.get());
            block->SetTranslate(spawn.position);
            block->Update();
            mapBlocks_.push_back(std::move(block));
            break;
        }
        case PendingSpawnKind::Block2:
        {
            auto block2 = std::make_unique<Object3d>();
            block2->Initialize(object3dCommon_.get());
            block2->SetModel("cube2/cube2.obj");
            block2->SetCamera(camera_.get());
            block2->SetTranslate(spawn.position);
            block2->Update();
            mapBlocks_.push_back(std::move(block2));
            break;
        }
        case PendingSpawnKind::Portal:
        {
            auto portal = std::make_unique<Object3d>();
            portal->Initialize(object3dCommon_.get());
            portal->SetModel("door/Door.obj");
            portal->SetCamera(camera_.get());
            portal->SetTranslate(spawn.position);
            portal->Update();
            mapBlocks_.push_back(std::move(portal));
            break;
        }
        case PendingSpawnKind::Item:
        {
            if (!itemMgr_) { break; }
            auto item = std::make_unique<Object3d>();
            item->Initialize(object3dCommon_.get());
            item->SetModel("coin/coin.obj");
            item->SetCamera(camera_.get());
            Vector3 itemPos = spawn.position;
            itemPos.y += 0.4f;
            item->SetTranslate(itemPos);
            item->SetEnableLighting(true);
            item->SetDirectionalLightIntensity(2.0f);
            item->SetPointLightIntensity(2.0f);
            item->Update();
            itemMgr_->RegisterItem(currentMapPath_, spawn.x, spawn.y, std::move(item));
            break;
        }
        case PendingSpawnKind::Spike:
        {
            auto spike = std::make_unique<Object3d>();
            spike->Initialize(object3dCommon_.get());
            spike->SetModel("strip/strip.obj");
            spike->SetCamera(camera_.get());
            Vector3 spikePos = spawn.position;
            spikePos.y -= 0.1f;
            spike->SetTranslate(spikePos);
            spike->SetLightingMode(2);
            spike->Update();
            mapBlocks_.push_back(std::move(spike));
            break;
        }
        case PendingSpawnKind::Water:
        {
            auto water = std::make_unique<Object3d>();
            water->Initialize(object3dCommon_.get());
            water->SetModel("water/water.obj");
            water->SetCamera(camera_.get());
            water->SetTranslate(spawn.position);
            Vector4 color = water->GetColor();
            color.w = 0.5f;
            water->SetColor(color);
            water->Update();
            waterBlocks_.push_back(std::move(water));
            break;
        }
        case PendingSpawnKind::Enemy:
        {
            EnemyType eType = EnemyType::Type0;
            if (spawn.subID == 1) {
                eType = EnemyType::Type1;
            }
            else if (spawn.subID == 2) {
                eType = EnemyType::Boss;
            }
            else if (spawn.subID == 3) {
                eType = EnemyType::Type2;
            }

            std::unique_ptr<Enemy> enemy;
            if (eType == EnemyType::Boss) {
                enemy = std::make_unique<BossEnemy>();
            }
            else {
                enemy = std::make_unique<NormalEnemy>();
            }
            enemy->Initialize(object3dCommon_.get(), camera_.get(), spawn.position, eType);
            enemies_.push_back(std::move(enemy));
            break;
        }
        case PendingSpawnKind::MoveHorizontal:
        case PendingSpawnKind::MoveVertical:
        {
            auto platform = std::make_unique<MovingPlatform>();
            platform->Initialize(
                object3dCommon_.get(),
                camera_.get(),
                spawn.position,
                spawn.kind == PendingSpawnKind::MoveHorizontal ? MovingPlatform::Axis::Horizontal : MovingPlatform::Axis::Vertical,
                movingPlatformSpeed_,
                static_cast<int>(spawn.length)
            );
            movingPlatforms_.push_back(std::move(platform));
            break;
        }
        default:
            break;
        }
    }
}

bool GameScene::IsMapBuildComplete() const
{
    return loadPrepared_ && pendingMapSpawns_.empty();
}

void GameScene::FinishMapLoading(const Vector3& startPos)
{
    isIncrementalMapLoading_ = false;
    loadPrepared_ = false;
    postLoadSettleFrames_ = 0;
    pendingRevealAfterLoad_ = false;

    if (player_) {
        player_->SetPosition(startPos);
        player_->ResetForMapTransition(true);
    }

    if (fade_) {
        fade_->SetAlpha(1.0f);
        fade_->SetPhase(FadePhase::LoadingHold);
    }

    SyncLoadedSceneForReveal();
    postLoadSettleFrames_ = kPostLoadSettleFrames;
    pendingRevealAfterLoad_ = true;

    if (input_) {
        input_->ResetAllKeys();
    }
}

void GameScene::Initialize() {
    winApp_       = WinApp::GetInstance();
    dxCommon_     = DirectXCommon::GetInstance();
    input_        = Input::GetInstance();
    srvManager_   = SrvManager::GetInstance();
    spriteCommon_ = SpriteCommon::GetInstance();

    TextureManager::GetInstance()->Initialize(dxCommon_, srvManager_);

    pendingMapSpawns_.clear();
    isIncrementalMapLoading_ = false;
    loadPrepared_ = false;
    postLoadSettleFrames_ = 0;
    pendingRevealAfterLoad_ = false;
    shouldStartLoading_ = false;
    isMapLoading_ = false;
    isPortalLoading_ = false;
    loadingTimer_ = 0.0f;
    portalLoadingTimer_ = 0.0f;
    bossNameVisible_ = false;
    returnToTitle_ = false;
    pendingGameClear_ = false;
    pendingPortalLoad_ = false;
    deferredModelLoadCursor_ = 0;
    initComplete_ = false;
    deferredInitPhase_ = DeferredInitPhase::UiSprites;

    if (sceneManager_ && !sceneManager_->GetOverlayScene()) {
        sceneManager_->SetOverlayScene(std::make_unique<LoadingScene>());
    }
}

void GameScene::Update() {
    const float deltaTime = 1.0f / 60.0f;
    input_->Update();
    backgroundSprite_->Update();

    if (bossNameSprite_) {
        bossNameSprite_->SetVisible(bossNameVisible_);
        bossNameSprite_->Update();
    }
    // —— プレイヤー操作を許可するかどうか（フェードアウト / ロード / フェードイン中、および開始演出中はすべて禁止）——
    const bool isFading = (fade_ && fade_->GetPhase() != FadePhase::None);
    const bool inIntro = (intro_ && intro_->IsPlaying());
    const bool inGameOver = (gameOver_ && gameOver_->IsPlaying());
    const bool inGameClear = (gameClear_ && gameClear_->IsPlaying());
    bool inBossIntro = (bossIntroPhase_ != BossIntroPhase::None);
    bool canControl = !(isFading || inIntro || inGameOver || inGameClear || inBossIntro);

    // ================== Pause Menu（ESC） ==================
    // ・ESCで一時停止/再開
    // ・W/S or ↑/↓で選択（上下端はループ）
    // ・SPACE/ENTERで決定
    bool wasPaused = isPaused_;
    if (canControl) {
        if (!isPaused_ && input_ && input_->TriggerKey(DIK_ESCAPE)) {
            isPaused_ = true;
            pauseCursor_ = 0; // デフォルトは Continue
        }
    }

    if (isPaused_) {
        if (!wasPaused) {
            // ちょうど一時停止に入ったこの1フレームでは ESC による解除を処理しない。同フレームでのオン / オフ切替を防ぐ
            return;
        }
        // Pause中はゲーム進行を止め、メニュー入力だけを処理する
        if (input_) {
            if (input_->TriggerKey(DIK_ESCAPE)) {
                isPaused_ = false;
                return;
            }

            constexpr int kPauseItemCount = 2;
            if (input_->TriggerKey(DIK_W) || input_->TriggerKey(DIK_UP)) {
                pauseCursor_ = (pauseCursor_ + kPauseItemCount - 1) % kPauseItemCount;
            }
            if (input_->TriggerKey(DIK_S) || input_->TriggerKey(DIK_DOWN)) {
                pauseCursor_ = (pauseCursor_ + 1) % kPauseItemCount;
            }

            if (input_->TriggerKey(DIK_SPACE) || input_->TriggerKey(DIK_RETURN)) {
                if (pauseCursor_ == 0) {
                    // Continue
                    isPaused_ = false;
                    input_->ResetAllKeys();
                    return;
                }
                else {
                    // Back to Title（Fadeで戻る）
                    isPaused_ = false;

                    if (!returnToTitle_ && fade_) {
                        returnToTitle_ = true;

                        // 黒幕パラメータをリセットし、完全な黒までフェードアウトを開始
                        fade_->SetAlpha(0.0f);
                        fade_->SetReachedBlack(false);
                        fade_->SetBlackHoldFrames(0);
                        fade_->SetOverlayPushed(false);

                        fade_->SetPhase(FadePhase::FadingOut);
                        if (Sprite* s = fade_->GetSprite()) {
                            s->SetVisible(true);
                        }
                    }
                    input_->ResetAllKeys();
                    return;
                }
            }
        }

        // PauseメニューのSpriteは動かないが、今後の拡張のためUpdateしておく
        if (pauseContinueNormal_) { pauseContinueNormal_->Update(); }
        if (pauseContinueSelected_) { pauseContinueSelected_->Update(); }
        if (pauseBackNormal_) { pauseBackNormal_->Update(); }
        if (pauseBackSelected_) { pauseBackSelected_->Update(); }
        return;
    }

    // ===== Intro 駆動（ロード／フェードアウト等の早期 return 前に実行。ただし Loading は上書きしない） =====
    if (fade_ && fade_->GetPhase() == FadePhase::None && intro_) {
        intro_->Update(deltaTime);
        // Intro 側で内部 Sprite の状態は更新されるため、ここでは再度手動 Update しない
    }

    // ===== 画面フェードイン／フェードアウトのステートマシン（優先実行） =====
    if (fade_ && fade_->GetPhase() == FadePhase::FadingOut) {
        // 1) alpha を毎フレーム増加
        float a = fade_->GetAlpha();
        a += fade_->GetSpeed();
        if (a > 1.0f) a = 1.0f;
        fade_->SetAlpha(a);

        // 2) 完全に黒くなった後の処理
        if (a >= 1.0f) {
            // 2-1) 完全な黒のまま数フレーム維持
            if (!fade_->ReachedBlack()) {
                fade_->SetReachedBlack(true);
                // 初めて完全な黒になったフレームでは先に return し、この1フレームは黒だけを表示する
                return;
            }
            else if (fade_->GetBlackHoldFrames() > 0) {
                fade_->SetBlackHoldFrames(fade_->GetBlackHoldFrames() - 1);
                return;
            }

            // 2-2) GameClear / GameOver からタイトルへ戻る
            if (returnToTitle_) {
                returnToTitle_ = false;
                if (sceneManager_) {
                    sceneManager_->ClearOverlayScene();
                    sceneManager_->SetNextScene(std::make_unique<TitleScene>());
                }
                return;
            }

            // 2-3) Loading のオーバーレイシーンを積む（1回だけ）
            if (!fade_->OverlayPushed()) {
                if (!pendingGameClear_ && !returnToTitle_) {
                    if (sceneManager_) {
                        sceneManager_->SetOverlayScene(std::make_unique<LoadingScene>());
                    }
                }
                fade_->SetOverlayPushed(true);
            }

            // 2-4) 転送門: この時点で実際のロードを開始
            if (pendingPortalLoad_) {
                pendingPortalLoad_ = false;
                StartLoadingMap(pendingPortalMapPath_, pendingPortalStartPos_, true);
                fade_->SetPhase(FadePhase::LoadingHold);
                return;
            }

            // 2-5) クリア: 全黒状態で GameClear 演出を開始
            if (pendingGameClear_) {
                pendingGameClear_ = false;
                if (sceneManager_) {
                    sceneManager_->ClearOverlayScene();
                }
                if (gameClear_ && !gameClear_->IsPlaying()) {
                    gameClear_->Start();
                }
                // 黒幕は完全な黒のまま維持し、背景は GameClear 側で描く
                fade_->SetAlpha(1.0f);
                fade_->SetPhase(FadePhase::None);
                return;
            }

            // 2-6) 通常時: 完全な黒からフェードインへ切り替える
            fade_->SetPhase(FadePhase::FadingIn);
            return;
        }

        // まだ 0 → 1 の途中
        return;
    }

    // このフレーム後半で FadingIn を処理する（以下参照）
    if (shouldStartLoading_) {
        shouldStartLoading_ = false;
        StartLoadingMap("Resources/map/map6.csv", { 2,1,0 }, false);
        return; // このフレーム先に表示 LoadingScene
    }
    // 2️⃣ 初期ロードのタイマー
    if (isMapLoading_) {
        loadingTimer_ += deltaTime;
        if (loadingTimer_ >= LOADING_DURATION) {
            isMapLoading_ = false;
            LoadMap("Resources/map/map6.csv", { 2,1,0 });
            isIncrementalMapLoading_ = true;
        }
        else {
            if (fade_) fade_->Update(deltaTime);
            return;
        }
    }

    // 3️⃣ 転送門ロードのタイマー
    if (isPortalLoading_) {
        portalLoadingTimer_ += deltaTime;
        if (portalLoadingTimer_ >= LOADING_DURATION) {
            isPortalLoading_ = false;
            LoadMap(portalMapPath_, portalStartPos_);
            isIncrementalMapLoading_ = true;
        }
        else {
            if (fade_) {
                fade_->Update(deltaTime);
                return;
            }
        }
    }

    if (isIncrementalMapLoading_) {
        ProcessPendingMapSpawns(kMapSpawnBudgetPerFrame);
        if (IsMapBuildComplete()) {
            FinishMapLoading(player_ ? player_->GetPosition() : Vector3{ 0, 0, 0 });
        }
        return;
    }

    if (pendingRevealAfterLoad_) {
        SyncLoadedSceneForReveal();

        if (postLoadSettleFrames_ > 0) {
            --postLoadSettleFrames_;
            return;
        }

        pendingRevealAfterLoad_ = false;
        if (sceneManager_) {
            sceneManager_->ClearOverlayScene();
        }
        if (fade_) {
            fade_->SetAlpha(1.0f);
            fade_->SetPhase(FadePhase::FadingIn);
        }
        return;
    }

    imguiManager_->Begin();

    // ===== Boss トリガー演出: 演出中はプレイヤー追従カメラを動かさない =====
    if (bossIntroPhase_ != BossIntroPhase::None) {
        UpdateBossIntro(deltaTime);
    } else {
        playerCamera_->Update();
    }

    std::vector<MovingPlatform*> platformPtrs;
    platformPtrs.reserve(movingPlatforms_.size());
    for (auto& mp : movingPlatforms_) {
        platformPtrs.push_back(mp.get());
    }
    if (!inBossIntro) {

    // プラットフォーム自身の Update（platformPtrs を渡して足場同士の衝突を処理）
    for (auto* p : platformPtrs) {
        if (p) {
            p->Update(deltaTime, mapChipField_, platformPtrs);
        }
    }
    }
    // 敵 Update
    // Boss 演出中は「世界を静止させる」ため、すべての敵を更新しない。
    // ただし Boss 本体だけは Object3d の Transform / 点滅タイマー更新を継続する。そうしないと不可視フレームで止まる可能性がある。
    if (!inBossIntro) {
        for (auto& e : enemies_) {
            if (e) {
                e->Update(deltaTime, mapChipField_, *player_);
            }
        }
    } else {
        if (introBoss_) {
            introBoss_->Update(deltaTime, mapChipField_, *player_);
        } else {
            if (BossEnemy* b = FindBossEnemy()) {
                b->Update(deltaTime, mapChipField_, *player_);
            }
        }
    }
    // ===== Boss 演出中: プレイヤーを完全静止させる（重力 / 慣性の影響を受けない）=====
    if (inBossIntro) {
        if (!gBossIntroFreezePlayer) {
            gBossIntroFreezePlayer = true;
            gBossIntroFrozenPlayerPos = player_->GetPosition();
        }

        // Player 自身の更新を1フレーム進める（タイマー／アニメ等）。ただし直後に位置／速度を強制的に戻す
        player_->Update(nullptr, mapChipField_);

        player_->SetPosition(gBossIntroFrozenPlayerPos);
        player_->SetVelocity({ 0.0f, 0.0f, 0.0f });
    }
    else {
        gBossIntroFreezePlayer = false;
        player_->Update(canControl ? input_ : nullptr, mapChipField_);
    }

    // ================== Boss トリガー演出: トリガー地点を判定してカメラ演出を開始 ==================
    if (bossIntroPhase_ == BossIntroPhase::None && canControl) {
        BossEnemy* boss = FindBossEnemy();
        if (boss && !boss->IsBattleTriggered() && boss->IsBattleTriggerReady(*player_, mapChipField_)) {
            StartBossIntro(boss);
            // ★ このフレーム開始時点で演出突入とみなす: 以降のロジック（ダメージ / 足場 / 衝突など）はすべてスキップ
            inBossIntro = true;
            canControl = false;
            // 即座にカメラを1フレーム進め、トリガーフレームからそのままカメラ演出が始まるようにする
            UpdateBossIntro(deltaTime);
        }
    }

    // ========= 段階1: 2本の移動床でプレイヤーを挟み込む =========
    crushedByPlatformThisFrame_ = false;
    damagedByEnemyThisFrame_ = false;
    damageSourceEnemy_ = nullptr;
    if (!inBossIntro) {
        if (player_ && !player_->IsDead() && !player_->IsInvincible()) {

        Vector3 pPos = player_->GetPosition();
        float halfW = player_->GetWidth() * 0.5f;
        float halfH = player_->GetHeight() * 0.5f;

        float left = pPos.x - halfW;
        float right = pPos.x + halfW;
        float bottom = pPos.y - halfH;
        float top = pPos.y + halfH;

        int overlapPlatformCount = 0;
        for (auto* plat : platformPtrs) {
            if (!plat) continue;
            MapChipField::Rect r = plat->GetRect();
            bool overlapX = !(right <= r.left || left >= r.right);
            bool overlapY = !(top <= r.bottom || bottom >= r.top);
            if (overlapX && overlapY) {
                ++overlapPlatformCount;
                if (overlapPlatformCount >= 2) {
                    // 同時に2本の足場と重なっている場合は、挟まれたと直接判定する
                    crushedByPlatformThisFrame_ = true;
                    break;
                }
            }
        }
    }
    }
    // ========= プレイヤーと敵の衝突 =========
    if (!inBossIntro && player_ && !player_->IsDead()) {

        Vector3 pPos = player_->GetPosition();
        float   pHalfW = player_->GetWidth() * 0.5f;
        float   pHalfH = player_->GetHeight() * 0.5f;
        Vector3 pVel = player_->GetVelocity();

        float pLeft = pPos.x - pHalfW;
        float pRight = pPos.x + pHalfW;
        float pBottom = pPos.y - pHalfH;
        float pTop = pPos.y + pHalfH;

        // stomp lock: ロック中の敵がすでにリストにいなければ解除する
        if (stompLockEnemy_) {
            bool found = false;
            for (auto& e : enemies_) {
                if (e.get() == stompLockEnemy_) { found = true; break; }
            }
            if (!found) { stompLockEnemy_ = nullptr; }
        }

        for (auto& enemyPtr : enemies_) {
            Enemy* enemy = enemyPtr.get();
            if (!enemy) { continue; }

            // Boss の遠距離弾幕判定（Boss 本体と重なる必要はない）
            if (enemy->CheckBossProjectileHit(*player_)) {
                if (!player_->IsInvincible()) {
                    damagedByEnemyThisFrame_ = true;
                    if (!damageSourceEnemy_) { damageSourceEnemy_ = enemy; }
                }
            }
            Vector3 ePos = enemy->GetPosition();
            float   eHalfW = enemy->GetWidth() * 0.5f;
            float   eHalfH = enemy->GetHeight() * 0.5f;

            // 死亡演出などで当たり判定を消している敵はスキップ
            if (enemy->GetWidth() <= 0.01f || enemy->GetHeight() <= 0.01f) {
                continue;
            }

            float eLeft = ePos.x - eHalfW;
            float eRight = ePos.x + eHalfW;
            float eBottom = ePos.y - eHalfH;
            float eTop = ePos.y + eHalfH;

            bool overlapX = !(pRight <= eLeft || pLeft >= eRight);
            bool overlapY = !(pTop <= eBottom || pBottom >= eTop);

            // 踏みつけロック: 同じ敵に対しては、プレイヤーがその当たり判定から離れるまで再判定しない
            if (enemy == stompLockEnemy_) {
                if (overlapX && overlapY) {
                    continue;
                } else {
                    stompLockEnemy_ = nullptr;
                }
            }
            if (!overlapX || !overlapY) {
                continue;
            }

            // ====== 「上から踏んだ」敵かどうかを判定 ======
            const float stompTolerance = 0.15f;

            float stompMinCenterY = ePos.y; // 通常の敵: 中心以上からなら踏みつけ扱い
            if (enemy->GetType() == EnemyType::Boss) {
                // Boss は少し高めなので、プレイヤー中心がやや低くても踏みつけ扱いにする（操作感向上のため）
                stompMinCenterY = ePos.y - eHalfH * 0.20f;
            }

           const float kStompFallSpeed = -0.05f;

           bool isStomp =
               (pVel.y < kStompFallSpeed) &&
               (pPos.y >= stompMinCenterY) &&
               (pBottom <= eTop + stompTolerance);
            if (isStomp) {
                stompLockEnemy_ = enemy;

                const bool isBoss = (enemy->GetType() == EnemyType::Boss);

                auto BounceAndLift = [&](float bounceY, float kickX) {
                    Vector3 newVel = pVel;
                    newVel.y = bounceY;
                    player_->SetVelocity(newVel);

                    Vector3 newPos = pPos;
                    newPos.y = eTop + pHalfH + 0.01f;
                    player_->SetPosition(newPos);

                    if (kickX != 0.0f) {
                        // まずプレイヤーの現在の水平速度方向へ弾く。ほぼ静止している場合は Boss との相対位置で方向を決める
                        float dir = (std::fabs(pVel.x) > 0.02f) ? (pVel.x > 0.0f ? 1.0f : -1.0f)
                                                               : (pPos.x >= ePos.x ? 1.0f : -1.0f);

                        float vx = pVel.x * 0.35f + dir * kickX;
                        vx = std::clamp(vx, -0.45f, 0.45f);

                        // ノックバックを短時間だけ安定して効かせる（プレイヤーの入力に依存しない）。さらに ease-out 減衰で自然に見せる
                        player_->StartStompKick(vx, 0.18f);

                        Vector3 v = player_->GetVelocity();
                        v.x = vx;
                        player_->SetVelocity(v);
                    }
                };

                // ✅ 被ダメージ無敵中でも、踏みつけによる撃破 / ダメージは許可する（無敵は「プレイヤーがダメージを受ける側」にのみ影響し、「敵を攻撃する側」には影響しない）

                // ★ Boss: Boss の踏みつけ無敵中、またはプレイヤーの踏みつけクールダウン中
                // 跳ね返りは可能だが、横方向へ強制的に蹴り出して「頭上で棒立ちして無限踏み / 無限安全」になるのを防ぐ
                if (isBoss && (!enemy->CanTakeStompDamage() || player_->IsStompCooldown())) {
                    BounceAndLift(0.55f, 0.40f);
                    player_->StartStompInvincible(0.08f);
                    break;
                }

                // ☆ 正常踏みつけ: 敵ダメージを与える/硬直/死亡判定
                enemy->OnStomp();

                BounceAndLift(0.70f, isBoss ? 0.32f : 0.0f);

                // 踏みつけ後少し無敵時間を与える（点滅なし）: 主な目的は「踏んだ直後の軽い誤ダメージ」を防ぐこと
                player_->StartStompInvincible(0.12f);

                // 連続踏みが簡単すぎないよう、クールダウン中は Boss への再度の踏みつけダメージを発生させない
                player_->StartStompCooldown(isBoss ? 0.45f : 0.25f);

                break;
            }
            else {
                // ☆ 上から踏んだのでなければ、敵にぶつかったものとしてプレイヤーにダメージを与える
                if (!player_->IsInvincible()) {
                    damagedByEnemyThisFrame_ = true;
                    if (!damageSourceEnemy_) { damageSourceEnemy_ = enemy; }
                }
            }
        }
    }
        // ========= Boss 撃破判定: クリア条件を「Boss を倒す」に変更 =========
    if (!bossDefeated_ && !pendingGameClear_ && gameClear_ && !gameClear_->IsPlaying()) {
        bool bossJustDied = false;
        for (auto& e : enemies_) {
            if (!e) { continue; }
            if (e->GetType() == EnemyType::Boss && e->IsDead()) {
                bossJustDied = true;
                break;
            }
        }

        if (bossJustDied) {
            bossDefeated_ = true;

            // クリア演出をトリガー: 先に完全な黒までフェードアウトし、その黒の上で GameClear を開始する
            if (fade_) {
                pendingGameClear_ = true;

                fade_->SetAlpha(0.0f);
                fade_->SetReachedBlack(false);
                fade_->SetBlackHoldFrames(0);
                fade_->SetOverlayPushed(false);

                fade_->SetPhase(FadePhase::FadingOut);
                if (Sprite* s = fade_->GetSprite()) {
                    s->SetVisible(true);
                }
            } else {
                // 万一 FadeManager が無ければ、そのまま直接勝利演出を開始
                gameClear_->Start();
            }
        }
    }

// ========= 死亡した敵を削除（Boss は x 回踏まれると死亡フラグが立つ） =========
    enemies_.erase(
        std::remove_if(enemies_.begin(), enemies_.end(),
            [](const std::unique_ptr<Enemy>& e) { return (!e) || e->IsDead(); }),
        enemies_.end());


    // ================== Boss HP（2D）の更新 ==================
    if (bossHpDamageSprite_ && bossHpSprite_) {
        Enemy* boss = nullptr;
        for (auto& e : enemies_) {
            if (e && e->GetType() == EnemyType::Boss) {
                boss = e.get();
                break;
            }
        }

        if (boss && !boss->IsDead()) {
            // 「Boss に接近した時（または Boss 戦がトリガーされた時）」のみ HP バーを表示する。ロジックは BossEnemy 側に任せる
            bool shouldShow = true;
            if (auto* b = dynamic_cast<BossEnemy*>(boss)) {
                shouldShow = b->ShouldShowBossHp(*player_, mapChipField_);
            }

            if (shouldShow) {
                bossHpVisible_ = true;

                float target = boss->GetHpRatio();
                target = std::clamp(target, 0.0f, 1.0f);
                bossHpRatio_ = target;

                // 赤の遅延バー: 緑バーへゆっくり追従して減少する（回復した場合は即座に追いつく）
                if (bossDamageRatio_ < target) {
                    bossDamageRatio_ = target;
                } else {
                    bossDamageRatio_ -= bossDamageDropSpeed_ * deltaTime;
                    if (bossDamageRatio_ < target) {
                        bossDamageRatio_ = target;
                    }
                }

                Vector2 dmgSize = bossHpBarSize_;
                dmgSize.x = bossHpBarSize_.x * bossDamageRatio_;
                bossHpDamageSprite_->SetPosition(bossHpBarPos_);
                bossHpDamageSprite_->SetSize(dmgSize);
                bossHpDamageSprite_->SetVisible(true);

                Vector2 hpSize = bossHpBarSize_;
                hpSize.x = bossHpBarSize_.x * bossHpRatio_;
                bossHpSprite_->SetPosition(bossHpBarPos_);
                bossHpSprite_->SetSize(hpSize);
                bossHpSprite_->SetVisible(true);
            } else {
                // Boss がまだ生きていても、未接近 / 未侵入の状態では HP バーを非表示にする（比率はリセットせず、点滅時に満タンへ跳ねるのを防ぐ）
                bossHpVisible_ = false;
                bossHpDamageSprite_->SetVisible(false);
                bossHpSprite_->SetVisible(false);
            }
        }
        else {
            // Boss が存在しない、または死亡している場合は非表示にしてリセット
            bossHpVisible_ = false;
            bossHpRatio_ = 1.0f;
            bossDamageRatio_ = 1.0f;
            bossHpDamageSprite_->SetVisible(false);
            bossHpSprite_->SetVisible(false);
        }

        bossHpDamageSprite_->Update();
        bossHpSprite_->Update();
    }

    // 先に足場上での着地 / 分離 / 搭乗を処理する（Boss 演出中はすべて固定）
    if (!inBossIntro && !crushedByPlatformThisFrame_) {
        HandlePlayerOnMovingPlatforms();
    }

        if (!inBossIntro) {
    // ========= 段階2: 足場に押されてブロック / トゲ / 門へめり込む、またはマップ外へ押し出される =========
    if (player_ && !player_->IsDead() && !player_->IsInvincible()) {
        Vector3 pPos = player_->GetPosition();
        float halfW = player_->GetWidth() * 0.5f;
        float halfH = player_->GetHeight() * 0.5f;

        float left = pPos.x - halfW;
        float right = pPos.x + halfW;
        float bottom = pPos.y - halfH;
        float top = pPos.y + halfH;

        // 2a) この時点でプレイヤー AABB がすでに Block / Spike / Portal に食い込んでいれば、そのまま足場に押し込まれたとみなす
        auto minIdx = mapChipField_.GetMapChipIndexByPosition({ left,  bottom, 0.0f });
        auto maxIdx = mapChipField_.GetMapChipIndexByPosition({ right, top,    0.0f });

        for (uint32_t y = minIdx.yIndex; y <= maxIdx.yIndex && !crushedByPlatformThisFrame_; ++y) {
            for (uint32_t x = minIdx.xIndex; x <= maxIdx.xIndex; ++x) {
                MapChipType t = mapChipField_.GetMapChipTypeByIndex(x, y);
                if (t != MapChipType::kBlock &&
                    t != MapChipType::kSpike &&
                    t != MapChipType::kBlock2) {
                    continue;
                }

                MapChipField::Rect r = mapChipField_.GetRectByIndex(x, y);

                // Block / Block2: マス全体
                if (t == MapChipType::kBlock || t == MapChipType::kBlock2) {
                    bool overlapX = !(right <= r.left || left >= r.right);
                    bool overlapY = !(top <= r.bottom || bottom >= r.top);
                    if (overlapX && overlapY) {
                        crushedByPlatformThisFrame_ = true;
                        break;
                    }
                }
                // Spike: 下端 kSpikeHeightRatio 分の高さだけを使う
                else if (t == MapChipType::kSpike) {
                    float tileHeight = r.top - r.bottom;
                    MapChipField::Rect spikeHitRect = r;
                    spikeHitRect.top = spikeHitRect.bottom + tileHeight * kSpikeHeightRatio;

                    bool overlapX = !(right <= spikeHitRect.left || left >= spikeHitRect.right);
                    bool overlapY = !(top <= spikeHitRect.bottom || bottom >= spikeHitRect.top);

                    if (overlapX && overlapY) {
                        crushedByPlatformThisFrame_ = true;
                        break;
                    }
                }
            }
        }


        // 2b) 足場に押されてマップ外（上下境界の外）へ出た場合も圧死扱い
        if (!crushedByPlatformThisFrame_) {
            Vector3 mapMin = mapChipField_.GetMapMinPosition();
            Vector3 mapMax = mapChipField_.GetMapMaxPosition();

            // ここでは少し余裕を持たせ、浮動小数点の揺れを避ける
            if (top > mapMax.y + 0.01f || bottom < mapMin.y - 0.01f) {
                crushedByPlatformThisFrame_ = true;
            }
        }
    }

    }

    if (!inBossIntro && particleMgr_ && emitter3D_ && player_->ConsumeDoubleJumpEvent()) {
        Vector3 playerPos = player_->GetPosition();

        // 少し持ち上げて、粒子が体の近くから弾けるようにする
        Vector3 spawnPos = playerPos + Vector3{ 0.0f, 0.8f, 0.0f };
        float horizontalBias = player_->IsFacingRight() ? -1.0f : 1.0f;
        emitter3D_->Emit(
            14,                        // 粒子数。好みに応じて 12〜24 へ調整可
            ParticleType::Model3D,     // 3D 粒子
            "jump/jump.obj",           // 仮に cube モデルを使用
            spawnPos,
            4.0f, 8.0f,                // 速度範囲: 四方へ弾ける感覚
            0.25f, 0.45f, horizontalBias                 // ライフサイクル: やや短めで、キビキビ見せる
        );
    }

    camera_->Update();
    // === ダッシュ時の星の尾エフェクト ===
    if (!inBossIntro && dashStarEmitter_ && player_->IsDashing()) {
        Vector3 pos = player_->GetPosition();
        float h = player_->GetHeight();

        // 足元付近
        pos.y -= h * 0.4f;

        // ダッシュ方向に応じて生成位置を「背後」へ少しずらす
        Vector3 dashDir = player_->GetDashDirection(); // x>0 で右ダッシュ、x<0 で左ダッシュ
        pos.x -= dashDir.x * 0.8f;

        dashStarEmitter_->Emit(
            1,                      // 毎フレーム数個の星
            ParticleType::Model3D,
            "star/star.obj",
            pos,
            2.0f, 4.0f,             // 速度
            0.08f, 0.16f,            // 寿命
            0.0f, true
        );
    }
    {
        Vector3 camPos = camera_->GetTransform().translate;
        Vector3 camDelta = camPos - prevCameraPos_;
        prevCameraPos_ = camPos;

        if (snowEmitter_) {
            snowEmitter_->ApplyCameraMove(camDelta);
        }
    }
    // まだ GameOver に入っていなければ HP を確認
    if (player_->GetHP() <= 0.0f) {
        if (gameOver_ && !gameOver_->IsPlaying()) {
            gameOver_->Start();
        }
    }
    if (coinUI_) {
        coinUI_->Update(deltaTime);
    }
    if (hintUI_) {
        hintUI_->Update(deltaTime);
    }
    // GameOver ステートマシンを進行（マネージャ側へ委譲）
    if (gameOver_) {
        gameOver_->Update(deltaTime);
    }
    // GameClear ステートマシン推進
    if (gameClear_) {
        gameClear_->Update(deltaTime);
    }
    // ==== 風パーティクル効果（画面全体 & map のみ）====
    if (windEmitter_ && currentMapPath_ == "Resources/map/map5.csv") {

        windSpawnTimer_ -= deltaTime;
        if (windSpawnTimer_ <= 0.0f) {
            // 発生頻度をやや高めにし、画面全体に風の線が出るようにする
            windSpawnTimer_ = 0.1f;   // 約 33 本 / 秒。好みに応じて調整可

            const float margin = 50.0f;

            // 発生領域: 画面右上の一帯
            // x は画面右側 60% 〜 画面外少しまで
            float spawnX = RandRangeFloat(
                WinApp::kClientWidth * 0.6f,
                WinApp::kClientWidth + margin
            );
            // y は画面上端 〜 上半分まで
            float spawnY = RandRangeFloat(
                -margin,
                WinApp::kClientHeight * 0.4f
            );

            Vector3 spawnPos = { spawnX, spawnY, 0.0f };

            // ここでの min/maxSpeed は「1秒あたりの移動ピクセル量」
            // 700〜1200 程度にすると、1〜1.5 秒ほどで右上から左下へ流れる
            windEmitter_->Emit(
                1,
                ParticleType::Sprite2D,
                "Resources/wind.dds",
                spawnPos,
                700.0f, 1200.0f,   // 速度 (px/s)
                1.0f, 1.5f         // ライフサイクル (秒)
            );
        }
    }
    if (snowEmitter_ && currentMapPath_ == "Resources/map/map5.csv") {

        // 想定する平均発生間隔（秒）
        const float snowInterval = 0.1f;   // 約 25 回 / 秒。自分で調整可

        snowSpawnTimer_ -= deltaTime;

        // while を使うのはフレームレート変動に対応するため
        // 現在の deltaTime は 1/60 固定なので問題ない
        while (snowSpawnTimer_ <= 0.0f) {
            snowSpawnTimer_ += snowInterval;

            const float margin = 75.0f;

            float screenX = RandRangeFloat(
                -margin,
                WinApp::kClientWidth + margin
            );
            float screenY = -margin;

            float ndcZ = RandRangeFloat(0.45f, 0.65f);

            Vector3 worldPos = ScreenToWorld(screenX, screenY, ndcZ, camera_.get());

            // ✅ 1 回につき 1 枚（または 2 枚）だけ発生させ、積み重ねて「連続する雪」に見せる
            snowEmitter_->Emit(
                1,                          // 少なめに変更: 1〜2 枚
                ParticleType::Model3D,
                "snow/snow.obj",
                worldPos,
                0.10f, 0.15f,
                5.0f, 9.0f
            );
        }
    }
    if (particleMgr_) {
        particleMgr_->Update(deltaTime);
    }
    // === GameClear 演出中に Space を押す → タイトルへ戻る ===
    if (gameClear_ && gameClear_->IsPlaying() && input_ && input_->TriggerKey(DIK_SPACE)) {

        if (sceneManager_) {
            sceneManager_->ClearOverlayScene();
            sceneManager_->SetNextScene(std::make_unique<TitleScene>());
        }
    }

    // === GameOver 演出中に Space を押す → タイトルへ戻る ===
    if (gameOver_ && gameOver_->IsPlaying() && input_ && input_->TriggerKey(DIK_SPACE)) {

        if (!returnToTitle_ && fade_) {
            returnToTitle_ = true;

            // 黒幕パラメータをリセットし、完全な黒までフェードアウトを開始
            fade_->SetAlpha(0.0f);
            fade_->SetReachedBlack(false);
            fade_->SetBlackHoldFrames(0);
            fade_->SetOverlayPushed(false);

            fade_->SetPhase(FadePhase::FadingOut);
            if (Sprite* s = fade_->GetSprite()) {
                s->SetVisible(true);
            }
            gameOver_->SetDrawEnabled(false);
        }
    }


    if (hpBar_) {
        hpBar_->Update(deltaTime);
    }
    for (auto& block : mapBlocks_) {
        if (block) {
            block->Update();
        }
    }
    for (auto& water : waterBlocks_) {
        if (water) {
            water->Update();
        }
    }

    if (itemMgr_) {
        itemMgr_->Update(deltaTime);
    }

    MapChipField::IndexSet playerIndex =
        mapChipField_.GetMapChipIndexByPosition(player_->GetPosition());
    // ===== プレイヤーがいたマスの履歴を記録し、「1 秒前の位置へ戻す」ために使う =====
    if (!playerIndexHistoryInitialized_) {
        // 初回は現在のセルでバッファ全体を埋め、ゴミデータを読まないようにする
        for (int i = 0; i < kPlayerIndexHistoryFrameCount_; ++i) {
            playerIndexHistory_[i] = playerIndex;
        }
        playerIndexHistoryInitialized_ = true;
        playerIndexHistoryCursor_ = 0;
        playerIndexOneSecAgo_ = playerIndex;
    }

    // 1 秒前のマス = 現在書き込もうとしている位置に残っている古い値
    playerIndexOneSecAgo_ = playerIndexHistory_[playerIndexHistoryCursor_];

    // このフレームのマスを履歴へ書き込み、カーソルを進める
    playerIndexHistory_[playerIndexHistoryCursor_] = playerIndex;
    playerIndexHistoryCursor_++;
    if (playerIndexHistoryCursor_ >= kPlayerIndexHistoryFrameCount_) {
        playerIndexHistoryCursor_ = 0;
    }

    // ===== トゲを踏んだ場合: 1 ダメージを与え、1 秒前にいたマスへ戻す =====
    {
        MapChipType tileType =
            mapChipField_.GetMapChipTypeByIndex(playerIndex.xIndex, playerIndex.yIndex);

        bool onSpike = false;

        // トゲマスに対してだけ「少し低い判定領域」を使う
        if (tileType == MapChipType::kSpike) {

            // 1) トゲがあるマスの矩形
            MapChipField::Rect spikeRect =
                mapChipField_.GetRectByIndex(playerIndex.xIndex, playerIndex.yIndex);

            // 2) プレイヤー現在の AABB
            Vector3 pPos = player_->GetPosition();
            float halfW = player_->GetWidth() * 0.5f;
            float halfH = player_->GetHeight() * 0.5f;

            float left = pPos.x - halfW;
            float right = pPos.x + halfW;
            float bottom = pPos.y - halfH;
            float top = pPos.y + halfH;

            // 3) マスの高さの 60% だけを「有効なトゲ高さ」として使う
            float tileHeight = spikeRect.top - spikeRect.bottom;

            MapChipField::Rect hitRect = spikeRect;
            hitRect.top = hitRect.bottom + tileHeight * kSpikeHeightRatio;

            // 4) プレイヤー AABB と「短くしたトゲ矩形」で重なり判定を行う
            bool overlapX = !(right <= hitRect.left || left >= hitRect.right);
            bool overlapY = !(top <= hitRect.bottom || bottom >= hitRect.top);

            onSpike = overlapX && overlapY;
        }

        // 足場に挟まれた死亡判定は従来どおり crushedByPlatformThisFrame_（マス全体判定）を使う
        if (!inBossIntro && (onSpike || crushedByPlatformThisFrame_ || damagedByEnemyThisFrame_) &&
            !player_->IsDead() && !player_->IsInvincible()) {

            // ✅ 敵がプレイヤーに当たった場合: 「跳ね上げ / 1 秒前へ戻す」は行わず、ダメージ + 無敵のみ与え、軽く分離して敵の中に埋まるのを防ぐ
            const bool enemyHitOnly = (damagedByEnemyThisFrame_ && !onSpike && !crushedByPlatformThisFrame_);
            if (enemyHitOnly) {
                player_->TakeDamage(static_cast<float>(player_->GetMaxHp() * 0.2f));
                player_->StartInvincible(1.0f);

                // 「上へ飛ばされる」挙動を打ち消す（TakeDamage 内で上向き速度が与えられていても、ここで強制的に抑える）
                {
                    Vector3 v = player_->GetVelocity();
                    if (v.y > 0.0f) { v.y = 0.0f; }
                    player_->SetVelocity(v);
                }

                // プレイヤーを敵から少し押し出し、無敵終了直後に再び即ダメージを受けるのを防ぐ
                if (damageSourceEnemy_) {
                    Vector3 p = player_->GetPosition();
                    Vector3 e = damageSourceEnemy_->GetPosition();
                    const float pHalfW = player_->GetWidth() * 0.5f;
                    const float eHalfW = damageSourceEnemy_->GetWidth() * 0.5f;
                    const float eps = 0.03f;
                    if (p.x < e.x) {
                        p.x = e.x - eHalfW - pHalfW - eps;
                    }
                    else {
                        p.x = e.x + eHalfW + pHalfW + eps;
                    }
                    player_->SetPosition(p);
                }
            }
            else {
                // トゲ / 圧死: 元のロジックを維持し、1 秒前にいたマスへ戻してダメージを与える
                MapChipField::IndexSet safeIndex = playerIndexOneSecAgo_;

                // もし 1 秒前もトゲの上なら、さらにさかのぼって「トゲでない」マスを探す
                MapChipType safeType =
                    mapChipField_.GetMapChipTypeByIndex(safeIndex.xIndex, safeIndex.yIndex);

                if (safeType == MapChipType::kSpike || safeType == MapChipType::kEnemy) {
                    MapChipField::IndexSet firstNonSpike{};
                    bool found = false;

                    for (int i = 0; i < kPlayerIndexHistoryFrameCount_; ++i) {
                        const auto& candidate = playerIndexHistory_[i];
                        MapChipType type =
                            mapChipField_.GetMapChipTypeByIndex(candidate.xIndex, candidate.yIndex);
                        if (type != MapChipType::kSpike && type != MapChipType::kEnemy) {
                            firstNonSpike = candidate;
                            found = true;
                            break;
                        }
                    }

                    if (found) {
                        safeIndex = firstNonSpike;
                    }
                }

                auto rectPrev = mapChipField_.GetRectByIndex(
                    safeIndex.xIndex, safeIndex.yIndex);

                // プレイヤーをこのマスの「上」に立たせる
                Vector3 targetPos{};
                targetPos.x = (rectPrev.left + rectPrev.right) * 0.5f;

                float halfH = player_->GetHeight() * 0.5f;
                targetPos.y = rectPrev.top + halfH;
                targetPos.z = player_->GetPosition().z;

                player_->SetPosition(targetPos);
                player_->ResetForMapTransition(true);

                player_->TakeDamage(static_cast<float>(player_->GetMaxHp() * 0.2f));
                player_->StartInvincible(1.0f);
            }
        }
    }
    // 転送門ヒントアイコンを更新（表示するかどうか + 位置）
    const PortalInfo* currentPortal = nullptr;
    if (portalMgr_) {
        portalMgr_->UpdateHint(playerIndex, player_->GetPosition(), canControl);
        currentPortal = portalMgr_->GetPortalAt(playerIndex);
    }

    if (itemMgr_) {
        bool picked = itemMgr_->OnPlayerStepOnTile(currentMapPath_, playerIndex, mapChipField_, player_.get());
        if (picked) {
            ++totalCoinCollected_;
            if (coinUI_) {
                coinUI_->SetTotalCoin(totalCoinCollected_);
            }
        }
    }
    if (currentPortal) {
        // プレイヤーがいずれかの門マスの上に立っている
        if (canControl && input_->TriggerKey(DIK_E)) {
                        // ==== 子ステージから Hub(map2) に戻る場合は解放進捗を更新する（解放専用であり、以後クリア条件には使わない）====
            auto itStage = hubStageByMap_.find(currentMapPath_);
            if (currentPortal->targetMap == "Resources/map/map2.csv" && itStage != hubStageByMap_.end()) {
                int stageIndex = itStage->second;   // これは第何関か (0〜3)
                if (hubProgress_ < stageIndex + 1) {
                    hubProgress_ = stageIndex + 1;
                    if (hubProgress_ >= 4) {
                        allStagesCleared_ = true;
                    }
                }
            }

            // ==== 通常の転送処理 ====
            pendingPortalMapPath_ = currentPortal->targetMap;
            pendingPortalStartPos_ = currentPortal->targetStartPos;
            pendingPortalLoad_ = true;

            if (fade_) {
                fade_->SetAlpha(0.0f);
                fade_->SetReachedBlack(false);
                fade_->SetBlackHoldFrames(0);
                fade_->SetOverlayPushed(false);

                fade_->SetPhase(FadePhase::FadingOut);
                if (Sprite* s = fade_->GetSprite()) {
                    s->SetVisible(true);
                }
            }
        }
    }
    if (dashUI_) {
        dashUI_->Update(deltaTime);
    }
    if (input_->TriggerKey(DIK_P)) {
        SoundManager::GetInstance()->Play("fanfare", false, 1.0f);
    }
    // ===== FadingIn: 完全な黒からフェードイン =====
    if (fade_ && fade_->GetPhase() == FadePhase::FadingIn) {
        float a = fade_->GetAlpha();
        a -= fade_->GetSpeed();
        if (a < 0.0f) {
            a = 0.0f;
            fade_->SetPhase(FadePhase::None); // 淡入完成

            // フェードイン完了 → 開始演出を起動（1 回のみ）
            if (intro_ && !intro_->HasStarted() &&
                !(gameOver_ && gameOver_->IsPlaying()) &&
                !(gameClear_ && gameClear_->IsPlaying())) {

                Vector3 pivot = player_ ? player_->GetPosition() : Vector3{ 0,0,0 };
                if (player_) {
                    player_->ResetForMapTransition(true);
                }
                if (input_) {
                    input_->ResetAllKeys();
                }
                intro_->Start(pivot);
            }
        }
        fade_->SetAlpha(a);
    }

#ifdef USE_IMGUI
    ImGui::Begin("Scene Controller");

    // ======= Camera =======
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        Vector3 camPos = camera_->GetTransform().translate;
        Vector3 camRot = camera_->GetTransform().rotate;
        float camPosArr[3] = { camPos.x, camPos.y, camPos.z };
        float camRotArr[3] = { camRot.x, camRot.y, camRot.z };

        if (ImGui::DragFloat3("Camera Position", camPosArr, 0.1f)) {
            camera_->SetTranslate({ camPosArr[0], camPosArr[1], camPosArr[2] });
        }
        if (ImGui::DragFloat3("Camera Rotation", camRotArr, 0.1f)) {
            camera_->SetRotate({ camRotArr[0], camRotArr[1], camRotArr[2] });
        }
    }
    // ======= Player Info =======
    if (ImGui::CollapsingHeader("Player Info", ImGuiTreeNodeFlags_DefaultOpen)) {
        // 位置情報
        Vector3 playerPos = player_->GetPosition();
        float playerPosArr[3] = { playerPos.x, playerPos.y, playerPos.z };
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", playerPos.x, playerPos.y, playerPos.z);
        MapChipField::IndexSet playerIndex = mapChipField_.GetMapChipIndexByPosition(player_->GetPosition());
        ImGui::Text("MapChip Index: (%d, %d)", playerIndex.xIndex, playerIndex.yIndex);

        // 現在のセル種別情報を追加
        MapChipType currentType = mapChipField_.GetMapChipTypeByIndex(playerIndex.xIndex, playerIndex.yIndex);
        const char* typeName = "Unknown";
        switch (currentType) {
        case MapChipType::kBlank:  typeName = "Blank";  break;
        case MapChipType::kBlock:  typeName = "Block";  break;
        case MapChipType::kPortal: typeName = "Portal"; break;
        case MapChipType::kItem:   typeName = "Item";   break;
        case MapChipType::kSpike:  typeName = "Spike";  break;
        case MapChipType::kWater:  typeName = "Water";  break;
        case MapChipType::kMoveHorizontal: typeName = "MoveHorizontal"; break;
        case MapChipType::kMoveVertical:   typeName = "MoveVertical";   break;
        case MapChipType::kEnemy:  typeName = "Enemy";  break;
        }
        ImGui::Text("Current MapChip Type: %s", typeName);
    }
    ImGui::End();
#endif
    imguiManager_->End();
}

void GameScene::Draw() {
    dxCommon_->Begin();

    // GameClear 演出中かどうか
    bool inGameClear = (gameClear_ && gameClear_->IsPlaying());

    // ================== 0) 背景の空（2D Sprite） ==================
    srvManager_->PreDraw();
    spriteCommon_->CommonDraw();
    if (backgroundSprite_) {
        backgroundSprite_->Draw();
    }
    dxCommon_->ClearDepthBuffer();

    // ================== 1) 3D シーン（地図、HP 3D バーなど） ==================
    srvManager_->PreDraw();
    object3dCommon_->CommonDraw();

    // マップブロック
    for (auto& block : mapBlocks_) {
        if (block) {
            block->Draw();
        }
    }

    // 移動床
    for (auto& p : movingPlatforms_) {
        if (p) {
            p->Draw();
        }
    }

    // 敵
    for (auto& e : enemies_) {
        if (e) {
            e->Draw();
        }
    }

    // アイテム
    if (itemMgr_) {
        itemMgr_->Draw3D();
    }

    // HP 3D バー
    if (hpBar_) {
        hpBar_->Draw3D();
    }

    // ================== 1.5) GameClear 用の全画面黒背景 ==================
    if (inGameClear && fade_) {
        Sprite* s = fade_->GetSprite();
        spriteCommon_->CommonDraw();
        if (s) {
            s->SetVisible(true);
            s->Draw();
        }
    }

    // ================== 2) 中間レイヤー: 交互ヒント Sprite ==================
    spriteCommon_->CommonDraw();
    if (!inGameClear) {
        if (hintUI_) {
            hintUI_->Draw();
        }

        // ダッシュスキル UI
        if (dashUI_) {
            dashUI_->Draw();
        }

        // Coin 数字 UI（コイン + 数字）
        if (coinUI_) {
            coinUI_->Draw2D();
        }

        // 転送門提示
        if (portalMgr_) {
            portalMgr_->DrawHint();
        }
    }

    // ================== 3) 前景 3D: プレイヤー（ヒントの手前に描画） ==================
    dxCommon_->ClearDepthBuffer();

    srvManager_->PreDraw();
    object3dCommon_->CommonDraw();

    // 勝利画面中は通常プレイヤー / 3D コイン / 3D 粒子 / 水ブロックを描画しない
    if (!(gameClear_ && gameClear_->IsPlaying())) {
        if (player_) {
            player_->Draw();
        }

        // 右上の 3D コイン
        if (coinUI_) {
            coinUI_->Draw3D();
        }

        // 3D 粒子
        if (particleMgr_) {
            particleMgr_->Draw3D();
        }

        // 水ブロック（プレイヤーの前面に配置）
        for (auto& water : waterBlocks_) {
            if (water) {
                water->Draw();
            }
        }
    }

    // ================== 4) 最前景 UI Sprite ==================
    spriteCommon_->CommonDraw();

    // ===== Boss Name（2D）=====
    if (!inGameClear && bossNameVisible_) {
        if (bossNameSprite_) { bossNameSprite_->Draw(); }
    }

    // ===== Boss HP（2D）=====
    if (!inGameClear && bossHpVisible_) {
        if (bossHpDamageSprite_) { bossHpDamageSprite_->Draw(); }
        if (bossHpSprite_)       { bossHpSprite_->Draw(); }
    }

    // Intro / 黒縁 / 暗角 / タイトル / Skip 表示
    if (intro_) {
        intro_->Draw();
    }

    // 黒幕のフェードイン / フェードアウト（GameClear 時はこの黒幕を使わず、勝利画面を隠さないようにする）
    if (fade_ && !inGameClear) {
        fade_->Draw();
    }

    // GameOver
    if (gameOver_) {
        gameOver_->Draw();
    }

    // GameClear タイトル
    if (gameClear_) {
        gameClear_->DrawTitle();
    }

    // 2D 粒子
    if (particleMgr_) {
        particleMgr_->Draw2D();
    }

    // ================== Pause Menu（ESC） ==================
    if (isPaused_ && !inGameClear) {
        spriteCommon_->CommonDraw();

        // 1) 背景を暗くする（Fade 用の黒幕 Sprite を一時的に借りて描画する）
        if (pauseDimSprite_) {
            pauseDimSprite_->SetColor({ 0.0f, 0.0f, 0.0f, 0.6f });
            pauseDimSprite_->Update();
            pauseDimSprite_->Draw();
        }



        // 2) ボタン（選択中のみ金枠）
        if (pauseCursor_ == 0) {
            if (pauseContinueSelected_) { pauseContinueSelected_->Draw(); }
            if (pauseBackNormal_) { pauseBackNormal_->Draw(); }
        }
        else {
            if (pauseContinueNormal_) { pauseContinueNormal_->Draw(); }
            if (pauseBackSelected_) { pauseBackSelected_->Draw(); }
        }
    }

    // ImGui（debug UI）
    if (imguiManager_) {
        imguiManager_->Draw();
    }

    dxCommon_->End();
}


void GameScene::Finalize() {
    // ==== ImGui ====
    if (imguiManager_) {
        imguiManager_->Finalize();
        imguiManager_.reset();
    }

    // ==== 各種マネージャ / UI ====
    if (dashUI_) {
        dashUI_->Finalize();
        dashUI_.reset();
    }

    if (portalMgr_) {
        portalMgr_->Finalize();
        portalMgr_.reset();
    }

    if (fade_) {
        fade_->Finalize();
        fade_.reset();
    }

    if (intro_) {
        intro_->Finalize();
        intro_.reset();
    }

    if (hpBar_) {
        hpBar_->Finalize();
        hpBar_.reset();
    }

    if (gameOver_) {
        gameOver_->Finalize();
        gameOver_.reset();
    }

    if (itemMgr_) {
        itemMgr_->Finalize();
        itemMgr_.reset();
    }

    if (coinUI_) {
        coinUI_->Finalize();
        coinUI_.reset();
    }

    if (gameClear_) {
        gameClear_->Finalize();
        gameClear_.reset();
    }

    if (hintUI_) {
        hintUI_->Finalize();
        hintUI_.reset();
    }

    // ==== パーティクル系 ====
    if (particleMgr_) {
        particleMgr_->Finalize();
        particleMgr_.reset();
    }
    // エミッタはここでは借用の生ポインタのみを保持し、delete は ParticleManager 側で管理済み
    emitter2D_ = nullptr;
    emitter3D_ = nullptr;
    dashStarEmitter_ = nullptr;
    windEmitter_ = nullptr;
    snowEmitter_ = nullptr;

    // ==== 背景 Sprite ====
    backgroundSprite_.reset();

    // ==== Pause Menu Sprites ====
    pauseContinueNormal_.reset();
    pauseContinueSelected_.reset();
    pauseBackNormal_.reset();
    pauseBackSelected_.reset();
    pauseDimSprite_.reset();
    isPaused_ = false;
    pauseCursor_ = 0;

    // ==== Boss HP（2D）Sprite ====
    bossHpDamageSprite_.reset();
    bossHpSprite_.reset();
    bossNameSprite_.reset();
    bossNameVisible_ = false;

    // ==== 3D オブジェクトコンテナ（ブロック / 水面 / 敵 / 足場）====
    // 中身は unique_ptr のため、clear() 時に要素は自動 delete される
    mapBlocks_.clear();
    waterBlocks_.clear();
    enemies_.clear();
    movingPlatforms_.clear();

    // ==== プレイヤー / カメラ / 3D 共通 ====
    playerCamera_.reset();
    player_.reset();
    camera_.reset();
    object3dCommon_.reset();

    spaceHint_.sprite.reset();
    shiftHint_.sprite.reset();
    sprintHint_.sprite.reset();
    upHints_.clear();

    deferredInitPhase_ = DeferredInitPhase::None;
    initComplete_ = false;
    deferredModelLoadCursor_ = 0;
    postLoadSettleFrames_ = 0;
    pendingRevealAfterLoad_ = false;
}


void GameScene::StartLoadingMap(const std::string& mapPath, const Vector3& startPos, bool isPortal) {
    if (sceneManager_ && !sceneManager_->GetOverlayScene()) {
        sceneManager_->SetOverlayScene(std::make_unique<LoadingScene>());
    }
    pendingMapSpawns_.clear();
    isIncrementalMapLoading_ = false;
    loadPrepared_ = false;
    postLoadSettleFrames_ = 0;
    pendingRevealAfterLoad_ = false;
    if (isPortal) {
        // 転送門ロード
        isPortalLoading_ = true;
        portalMapPath_ = mapPath;
        portalStartPos_ = startPos;
        portalLoadingTimer_ = 0.0f;
    }
    else {
        // 初期ロード
        isMapLoading_ = true;
        loadingTimer_ = 0.0f;
    }
}

void GameScene::LoadMap(const std::string& mapPath, const Vector3& startPos)
{
    // 今回のマップパスを記録
    currentMapPath_ = mapPath;

    // 古い item 描画オブジェクトをクリア
    if (itemMgr_) {
        itemMgr_->ClearVisuals();
    }

    // ==== 古い 3D オブジェクトをクリア（unique_ptr コンテナなのでそのまま clear でよい）====
    mapBlocks_.clear();
    waterBlocks_.clear();
    enemies_.clear();
    movingPlatforms_.clear();

    // ==== 古い Hint Sprite をクリア（unique_ptr を reset）====
    spaceHint_.sprite.reset();
    shiftHint_.sprite.reset();
    sprintHint_.sprite.reset();
    upHints_.clear();

    // マップを再ロード
    mapChipField_.LoadMapChipCsv(mapPath);
    BuildPendingMapSpawns();
    loadPrepared_ = true;

    // ==== 右上 Coin UI を更新: 「合計で取得した coin 数」を表示 ====
    if (coinUI_) {
        coinUI_->SetTotalCoin(totalCoinCollected_);
    }

    // === map1 のみ Space / Shift / Sprint / Up のヒントを生成 ===
    if (mapPath == "Resources/map/map.csv") {

        // (5,2) → space.png
        spaceHint_.sprite = std::make_unique<Sprite>();
        spaceHint_.sprite->Initialize(spriteCommon_, "Resources/space2.png");
        spaceHint_.sprite->SetSize({ 48.0f, 32.0f });
        spaceHint_.worldPos = mapChipField_.GetMapChipPositionByIndex(5, 2);
        spaceHint_.worldPos.y += 0.4f;

        // (19,6) → shift.png
        shiftHint_.sprite = std::make_unique<Sprite>();
        shiftHint_.sprite->Initialize(spriteCommon_, "Resources/shift.png");
        shiftHint_.sprite->SetSize({ 48.0f, 32.0f });
        shiftHint_.worldPos = mapChipField_.GetMapChipPositionByIndex(19, 6);
        shiftHint_.worldPos.x -= 0.2f;
        shiftHint_.worldPos.y += 0.5f;

        // (20,6) → sprint.png
        sprintHint_.sprite = std::make_unique<Sprite>();
        sprintHint_.sprite->Initialize(spriteCommon_, "Resources/sprint.png");
        sprintHint_.sprite->SetSize({ 48.0f, 48.0f });
        sprintHint_.worldPos = mapChipField_.GetMapChipPositionByIndex(20, 6);
        sprintHint_.worldPos.x -= 0.3f;
        sprintHint_.worldPos.y += 0.5f;

        // Up ヒント一式
        auto makeUpHint = [&](int x, int y) {
            HintSprite h;
            h.sprite = std::make_unique<Sprite>();
            h.sprite->Initialize(spriteCommon_, "Resources/up.dds");
            h.sprite->SetSize({ 32.0f, 32.0f });
            h.worldPos = mapChipField_.GetMapChipPositionByIndex(x, y);
            // vector 内は unique_ptr なので、必ず move する
            upHints_.push_back(std::move(h));
            };

        // (6,2), (11,4), (12,4)
        makeUpHint(6, 2);
        makeUpHint(11, 4);
        makeUpHint(12, 4);
    }
    // === Hub マップ（map2）: 次の関への門を指す方向矢印だけを表示する ===
    else if (mapPath == "Resources/map/map2.csv") {
        // チュートリアル用の Space / Shift ヒントは Hub では表示せず、位置だけクリアする
        spaceHint_.worldPos = { 0,0,0 };
        shiftHint_.worldPos = { 0,0,0 };
        sprintHint_.worldPos = { 0,0,0 };

        int nextX = -1;
        int nextY = -1;

        // 門インデックス（map2 内）:
        //   map3: (11,5)
        //   map4: (14,5)
        //   map5: (23,1)
        //   map6: (12,14)
        if (hubProgress_ <= 0) {
            nextX = 11; nextY = 5;
        }
        else if (hubProgress_ == 1) {
            nextX = 14; nextY = 5;
        }
        else if (hubProgress_ == 2) {
            nextX = 23; nextY = 1;
        }
        else if (hubProgress_ == 3) {
            nextX = 12; nextY = 14;
        }
        else {
            // hubProgress_ >= 4 なら全ステージクリア済みなので、以後は方向を表示しない
        }

        if (nextX >= 0) {
            HintSprite h;
            h.sprite = std::make_unique<Sprite>();
            h.sprite->Initialize(spriteCommon_, "Resources/up.png");
            h.sprite->SetSize({ 32.0f, 32.0f });
            h.sprite->SetRotation(std::numbers::pi_v<float>);
            h.worldPos = mapChipField_.GetMapChipPositionByIndex(nextX, nextY);
            h.worldPos.x += 0.4f;
            h.worldPos.y += 2.0f;   // 少し持ち上げて、門の上に浮かせる
            upHints_.push_back(std::move(h));
        }
    }
    else {
        // map1 / map2 以外では、チュートリアルヒントを確実に描画しない
        spaceHint_.worldPos = { 0,0,0 };
        shiftHint_.worldPos = { 0,0,0 };
        sprintHint_.worldPos = { 0,0,0 };
    }

    // ==== プレイヤー開始位置を設定 ====
    if (player_) {
        player_->SetPosition(startPos);
    }

    MapChipField::IndexSet startIndex = mapChipField_.GetMapChipIndexByPosition(startPos);
    for (int i = 0; i < kPlayerIndexHistoryFrameCount_; ++i) {
        playerIndexHistory_[i] = startIndex;
    }
    playerIndexHistoryCursor_ = 0;
    playerIndexHistoryInitialized_ = true;
    playerIndexOneSecAgo_ = startIndex;

    // 相机同步
    if (camera_) {
        camera_->SetTranslate(startPos + Vector3{ 0,0,-40 });
        prevCameraPos_ = camera_->GetTransform().translate;
    }

    if (playerCamera_) {
        playerCamera_->SetMapBounds(
            mapChipField_.GetMapMinPosition(),
            mapChipField_.GetMapMaxPosition()
        );
    }

    // 現在のマップに応じて転送門リストを更新
    if (portalMgr_) {
        portalMgr_->ClearPortals();
    }

    // ========== map1（開始マップ） ==========
    if (mapPath == "Resources/map/map.csv") {
        if (portalMgr_) {
            portalMgr_->AddPortal(
                { 26, 11 },
                "Resources/map/map2.csv",
                mapChipField_.GetMapChipPositionByIndex(2, 1)
            );
        }
    }
    // ========== map2（中央マップ / Hub） ==========
    else if (mapPath == "Resources/map/map2.csv") {
        if (portalMgr_) {
            // Hub → map1 に戻る門
            portalMgr_->AddPortal(
                { 2, 1 },
                "Resources/map/map.csv",
                mapChipField_.GetMapChipPositionByIndex(26, 11)
            );

            if (hubProgress_ >= 0) {
                portalMgr_->AddPortal(
                    { 11, 5 },
                    "Resources/map/map3.csv",
                    mapChipField_.GetMapChipPositionByIndex(2, 1)
                );
            }
            if (hubProgress_ >= 1) {
                portalMgr_->AddPortal(
                    { 14, 5 },
                    "Resources/map/map4.csv",
                    mapChipField_.GetMapChipPositionByIndex(2, 1)
                );
            }
            if (hubProgress_ >= 2) {
                portalMgr_->AddPortal(
                    { 23, 1 },
                    "Resources/map/map5.csv",
                    mapChipField_.GetMapChipPositionByIndex(2, 1)
                );
            }
            if (hubProgress_ >= 3) {
                Vector3 finalStart = mapChipField_.GetMapChipPositionByIndex(2, 1);
                portalMgr_->AddPortal({ 12, 14 }, "Resources/map/map6.csv", finalStart);
                portalMgr_->AddPortal({ 13, 14 }, "Resources/map/map6.csv", finalStart);
            }
        }
    }
    // ========== 各子ステージ内部: Hub へ戻す ==========
    else {
        if (portalMgr_) {
            if (mapPath == "Resources/map/map3.csv") {
                portalMgr_->AddPortal({ 61, 1 }, "Resources/map/map2.csv",
                    mapChipField_.GetMapChipPositionByIndex(11, 5));
            }
            else if (mapPath == "Resources/map/map4.csv") {
                portalMgr_->AddPortal({ 69, 1 }, "Resources/map/map2.csv",
                    mapChipField_.GetMapChipPositionByIndex(14, 5));
            }
            else if (mapPath == "Resources/map/map5.csv") {
                portalMgr_->AddPortal({ 81, 1 }, "Resources/map/map2.csv",
                    mapChipField_.GetMapChipPositionByIndex(23, 1));
            }
            else if (mapPath == "Resources/map/map6.csv") {
                portalMgr_->AddPortal({ 89, 1 }, "Resources/map/map2.csv",
                    mapChipField_.GetMapChipPositionByIndex(12, 14));
            }
        }
    }
}

void GameScene::HandlePlayerOnMovingPlatforms()
{
    if (!player_) return;
    if (movingPlatforms_.empty()) return;
    if (player_->IsDead()) return;

    Vector3 pos = player_->GetPosition();
    Vector3 vel = player_->GetVelocity();

    const float halfW = player_->GetWidth() * 0.5f;
    const float halfH = player_->GetHeight() * 0.5f;

    // movingPlatforms_ は vector<unique_ptr<MovingPlatform>>
    for (auto& platformPtr : movingPlatforms_) {
        MovingPlatform* platform = platformPtr.get();
        if (!platform) continue;

        MapChipField::Rect r = platform->GetRect();

        float left = pos.x - halfW;
        float right = pos.x + halfW;
        float bottom = pos.y - halfH;
        float top = pos.y + halfH;

        bool overlapX = !(right <= r.left || left >= r.right);
        bool overlapY = !(top <= r.bottom || bottom >= r.top);
        if (!overlapX || !overlapY) {
            continue;
        }

        // X / Y 方向の最小めり込み量を計算
        float penX = (std::min)(r.right - left, right - r.left);
        float penY = (std::min)(r.top - bottom, top - r.bottom);

        if (penX < penY) {
            // 水平方向に分離（側面を遮る）
            float centerPlayerX = pos.x;
            float centerRectX = (r.left + r.right) * 0.5f;
            if (centerPlayerX < centerRectX) {
                pos.x -= penX;   // プレイヤーが左側 → 左へ押し出す
            }
            else {
                pos.x += penX;   // プレイヤーが右側 → 右へ押し出す
            }
            vel.x = 0.0f;
        }
        else {
            // 垂直方向に分離（頭上 / 足元を遮る）
            float centerPlayerY = pos.y;
            float centerRectY = (r.bottom + r.top) * 0.5f;

            if (centerPlayerY < centerRectY) {
                // 下から上へ足場の底にぶつかった
                pos.y -= penY;
                if (vel.y > 0.0f) {
                    vel.y = 0.0f;
                }
            }
            else {
                // 上から足場を踏んだ → 地面として扱い、その上に乗って一緒に動く
                player_->SetPosition(pos);
                player_->SetVelocity(vel);
                player_->LandOnExternalGround(r.top);

                // 補正後の pos / vel を取得し直す
                pos = player_->GetPosition();
                vel = player_->GetVelocity();

                // プレイヤーを足場と一緒に移動させる（このフレームの足場移動量）
                Vector3 delta = platform->GetPosition() - platform->GetPrevPosition();
                pos = pos + delta;
            }
        }
    }

    player_->SetPosition(pos);
    player_->SetVelocity(vel);
}



// ================== Boss トリガー演出（カメラ推 Boss + 名字 + 回プレイヤー） ==================
BossEnemy* GameScene::FindBossEnemy()
{
    for (auto& e : enemies_) {
        if (!e) { continue; }
        if (e->GetType() != EnemyType::Boss) { continue; }
        if (auto* b = dynamic_cast<BossEnemy*>(e.get())) {
            return b;
        }
    }
    return nullptr;
}

Vector3 GameScene::ConstrainCameraToMap(const Vector3& desiredPos, float fovY, float cameraZ) const
{
    if (!camera_) { return desiredPos; }

    Vector3 mapMin = mapChipField_.GetMapMinPosition();
    Vector3 mapMax = mapChipField_.GetMapMaxPosition();

    float halfViewHeight = std::abs(cameraZ) * std::tan(fovY * 0.5f);
    float halfViewWidth  = halfViewHeight * camera_->GetAspectRatio();

    float minX = mapMin.x + halfViewWidth;
    float maxX = mapMax.x - halfViewWidth;
    float minY = mapMin.y + halfViewHeight;
    float maxY = mapMax.y - halfViewHeight;

    // マップが小さすぎる場合は、カメラを中央に固定する
    if (maxX < minX) { minX = maxX = (mapMin.x + mapMax.x) * 0.5f; }
    if (maxY < minY) { minY = maxY = (mapMin.y + mapMax.y) * 0.5f; }

    Vector3 out = desiredPos;
    out.x = std::clamp(out.x, minX, maxX);
    out.y = std::clamp(out.y, minY, maxY);
    return out;
}

void GameScene::StartBossIntro(BossEnemy* boss)
{
    if (!boss || !camera_) { return; }
    introBoss_ = boss;
    bossIntroPhase_ = BossIntroPhase::ToBoss;
    bossIntroTimer_ = 0.0f;

    // プレイヤーを固定: 演出中はトリガー瞬間の位置を保つ
    if (player_) {
        gBossIntroFreezePlayer = true;
        gBossIntroFrozenPlayerPos = player_->GetPosition();
        player_->SetVelocity({ 0.0f, 0.0f, 0.0f });
        player_->SetPosition(gBossIntroFrozenPlayerPos);
    }

    bossIntroStartCamPos_ = camera_->GetTransform().translate;
    bossIntroStartFovY_   = camera_->GetFovY();

    // 戻り段階のデフォルト終点: トリガー時のプレイヤーカメラへ戻す（現在の追従カメラへ戻したい場合は終点を毎フレーム再計算してもよい）
    bossIntroBackStartCamPos_ = bossIntroStartCamPos_;
    bossIntroBackStartFovY_   = bossIntroStartFovY_;

    bossNameVisible_ = false;
}

void GameScene::UpdateBossIntro(float dt)
{
    if (!camera_) { return; }
    if (bossIntroPhase_ == BossIntroPhase::None) { return; }

    // Boss が事前に倒されている / 存在しない場合は、演出を即終了する
    if (!introBoss_ || introBoss_->IsDead()) {
        bossIntroPhase_ = BossIntroPhase::None;
        introBoss_ = nullptr;
        bossNameVisible_ = false;
        gBossIntroFreezePlayer = false;
        return;
    }

    bossIntroTimer_ += dt;

    const Vector3 bossPos = introBoss_->GetPosition();

    switch (bossIntroPhase_) {
    case BossIntroPhase::ToBoss:
    {
        float t = bossIntroToBossDur_ > 0.0f ? (bossIntroTimer_ / bossIntroToBossDur_) : 1.0f;
        if (t > 1.0f) { t = 1.0f; }
        float s = SmoothStep01(t);

        float fov = LerpFloat(bossIntroStartFovY_, bossIntroBossFovY_, s);
        float z   = LerpFloat(bossIntroStartCamPos_.z, bossIntroBossZ_, s);

        // 目標点（★ 全体を少し下へずらす: bossIntroBossCamOffset_.y が負）
        Vector3 desiredTarget = {
            bossPos.x + bossIntroBossCamOffset_.x,
            bossPos.y + bossIntroBossCamOffset_.y,
            z
        };

        // ★ 先に目標を制約してから補間する。境界付近では「境界に沿って滑る」ようになり、突然切り詰められない
        Vector3 target = ConstrainCameraToMap(desiredTarget, fov, z);

        Vector3 newPos = LerpVec3(bossIntroStartCamPos_, target, s);
        newPos.z = z;

        camera_->SetFovY(fov);
        camera_->SetTranslate(newPos);
        camera_->Update();

        if (t >= 1.0f) {
            bossIntroPhase_ = BossIntroPhase::ShowName;
            bossIntroTimer_ = 0.0f;
            bossNameVisible_ = true;
        }
        break;
    }

    case BossIntroPhase::ShowName:
    {
        bossNameVisible_ = true;

        float fov = bossIntroBossFovY_;
        float z   = bossIntroBossZ_;

        Vector3 desiredTarget = {
            bossPos.x + bossIntroBossCamOffset_.x,
            bossPos.y + bossIntroBossCamOffset_.y,
            z
        };
        Vector3 target = ConstrainCameraToMap(desiredTarget, fov, z);

        camera_->SetFovY(fov);
        camera_->SetTranslate(target);
        camera_->Update();

        if (bossIntroTimer_ >= bossIntroShowDur_) {
            bossIntroPhase_ = BossIntroPhase::BackToPlayer;
            bossIntroTimer_ = 0.0f;

            bossIntroBackStartCamPos_ = camera_->GetTransform().translate;
            bossIntroBackStartFovY_   = camera_->GetFovY();

            bossNameVisible_ = false;
        }
        break;
    }

    case BossIntroPhase::BackToPlayer:
    {
        bossNameVisible_ = false;

        float t = bossIntroBackDur_ > 0.0f ? (bossIntroTimer_ / bossIntroBackDur_) : 1.0f;
        if (t > 1.0f) { t = 1.0f; }
        float s = SmoothStep01(t);

        // 目標: 「プレイヤー追従カメラ」の位置へ戻る（ここでは Initialize 内の SetOffset と一致する offset を使う）
        Vector3 playerPos = player_ ? player_->GetPosition() : Vector3{ 0,0,0 };
        Vector3 desiredTarget = {
            playerPos.x,
            playerPos.y,
            0.0f
        };

        // FOV / Z も同期して戻す（トリガー瞬間のプレイヤーカメラを終点として使う）
        float fov = LerpFloat(bossIntroBackStartFovY_, bossIntroStartFovY_, s);
        float z   = LerpFloat(bossIntroBackStartCamPos_.z, bossIntroStartCamPos_.z, s);

        desiredTarget.z = z;

        // ★ 同様に、先に目標を制約してから補間する（さらに「開始位置」も現在の fov / z で少し制約する）
        //  こうすると境界付近でも、視口の変化に合わせて滑らかに境界沿いへ動き、急に切り詰められない
        Vector3 startPos = { bossIntroBackStartCamPos_.x, bossIntroBackStartCamPos_.y, z };
        startPos = ConstrainCameraToMap(startPos, fov, z);

        Vector3 target = ConstrainCameraToMap(desiredTarget, fov, z);
        Vector3 newPos = LerpVec3(startPos, target, s);
        newPos.z = z;

        camera_->SetFovY(fov);
        camera_->SetTranslate(newPos);
        camera_->Update();

        if (t >= 1.0f) {
            bossIntroPhase_ = BossIntroPhase::None;
            gBossIntroFreezePlayer = false;

            // 正式に戦闘開始
            if (introBoss_) {
                introBoss_->TriggerBattleNow();
            }
            introBoss_ = nullptr;
            bossNameVisible_ = false;
        }
        break;
    }

    default:
        break;
    }
}