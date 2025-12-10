#include "GameScene.h"
#include <numbers>
#include <scene/LoadingScene.h>
#include "SceneManager.h"
#include <cstdlib>
namespace {
    // 0.0f = 完全没有伤害高度, 1.0f = 整个格子都算刺
    constexpr float kSpikeHeightRatio = 0.5f;
}

static float RandRangeFloat(float a, float b)
{
    float t = static_cast<float>(rand()) / RAND_MAX;
    return a + (b - a) * t;
}
// 将3D世界坐标转换为屏幕坐标
Vector3 WorldToScreen(const Vector3& worldPos, Camera* camera)
{
    // 先构建齐次坐标
    float x = worldPos.x;
    float y = worldPos.y;
    float z = worldPos.z;
    float w = 1.0f;

    // VP 矩阵
    const Matrix4x4& vp = camera->GetViewprojectionMatrix();

    // 变换到 clip space
    float clipX = x * vp.m[0][0] + y * vp.m[1][0] + z * vp.m[2][0] + w * vp.m[3][0];
    float clipY = x * vp.m[0][1] + y * vp.m[1][1] + z * vp.m[2][1] + w * vp.m[3][1];
    float clipZ = x * vp.m[0][2] + y * vp.m[1][2] + z * vp.m[2][2] + w * vp.m[3][2];
    float clipW = x * vp.m[0][3] + y * vp.m[1][3] + z * vp.m[2][3] + w * vp.m[3][3];

    // 透视除法
    if (clipW != 0.0f)
    {
        clipX /= clipW;
        clipY /= clipW;
        clipZ /= clipW;
    }

    // NDC (-1~1) -> 屏幕坐标
    float screenX = (clipX * 0.5f + 0.5f) * float(WinApp::kClientWidth);
    float screenY = (1.0f - (clipY * 0.5f + 0.5f)) * float(WinApp::kClientHeight);
    float ndcZ    = clipZ * 0.5f + 0.5f;
    return { screenX, screenY, ndcZ };
}
// 将屏幕像素坐标(x,y)映射到世界坐标，ndcZ∈[0,1]：0=近裁剪面, 1=远裁剪面
Vector3 ScreenToWorld(float screenX, float screenY, float ndcZ, Camera* camera)
{
    const Matrix4x4& vp = camera->GetViewprojectionMatrix();
    Matrix4x4 invVP = Math::Inverse(vp); // 需要你项目里的矩阵求逆函数

    float ndcX = (screenX / float(WinApp::kClientWidth)) * 2.0f - 1.0f;
    float ndcY = -(screenY / float(WinApp::kClientHeight)) * 2.0f + 1.0f; // 注意Y翻转

    Vector3 world = Math::TransformCoordLocal(Vector3{ ndcX, ndcY, ndcZ }, invVP);
    return world;
}

void GameScene::GenerateBlocks() {
    for (uint32_t y = 0; y < mapChipField_.numBlockVertical_; y++) {
        for (uint32_t x = 0; x < mapChipField_.numBlockHorizontal_; x++) {
            // 获取当前格子类型
            MapChipType type = mapChipField_.GetMapChipTypeByIndex(x, y);
            Vector3 position = mapChipField_.GetMapChipPositionByIndex(x, y);
            // 如果是方块（kBlock），创建3D对象
            if (type == MapChipType::kBlock) {
                Object3d* block = new Object3d();
                block->Initialize(object3dCommon_);
                block->SetModel("cube/cube.obj");       // 使用方块模型
                block->SetCamera(camera_);
                // 设置方块位置（根据格子索引转换为世界坐标）
                block->SetTranslate(position);
                // 添加到方块列表
                mapBlocks_.push_back(block);
            }
            else if (type == MapChipType::kBlock2) {
                Object3d* block2 = new Object3d();
                block2->Initialize(object3dCommon_);
                block2->SetModel("cube2/cube2.obj");
                block2->SetCamera(camera_);
                block2->SetTranslate(position);
                mapBlocks_.push_back(block2);
            }
            else if (type == MapChipType::kPortal) {
                // 创建传送门可视化对象（例如使用不同颜色的方块）
                Object3d* portal = new Object3d();
                portal->Initialize(object3dCommon_);
                portal->SetModel("door/Door.obj"); // 特殊传送门模型
                portal->SetCamera(camera_);
                portal->SetTranslate(position);
                mapBlocks_.push_back(portal);
            }
            else if (type == MapChipType::kItem) {
                if (!itemMgr_) { continue; }

                // 若该格在此地图已经被拾取过，则不再生成
                if (!itemMgr_->CanSpawnItem(currentMapPath_, x, y)) {
                    continue;
                }

                auto item = std::make_unique<Object3d>();
                item->Initialize(object3dCommon_);
                item->SetModel("coin/coin.obj");
                item->SetCamera(camera_);

                Vector3 itemPos = position;
                itemPos.y += 0.4f;
                item->SetTranslate(itemPos);
                item->SetEnableLighting(true);
                item->SetDirectionalLightIntensity(2.0f);
                item->SetPointLightIntensity(2.0f);

                itemMgr_->RegisterItem(currentMapPath_, x, y, std::move(item));
            }
            else if (type == MapChipType::kSpike) {
                Object3d* spike = new Object3d();
                spike->Initialize(object3dCommon_);
                spike->SetModel("strip/strip.obj");   // 使用地刺模型
                spike->SetCamera(camera_);

                // 视情况微调一下高度（例如让刺从地面冒出来一点）
                Vector3 spikePos = position;
                spikePos.y -= 0.1f;                   // 根据模型大小自己调
                spike->SetTranslate(spikePos);
                spike->SetLightingMode(2);
                mapBlocks_.push_back(spike);
            }
            else if (type == MapChipType::kWater) {     // 水方块
                Object3d* water = new Object3d();
                water->Initialize(object3dCommon_);
                water->SetModel("water/water.obj");
                water->SetCamera(camera_);
                water->SetTranslate(position);
                Vector4 color = water->GetColor();
                color.w = 0.5f;
                water->SetColor(color);
                waterBlocks_.push_back(water);
            }
            else if (type == MapChipType::kEnemy) {
                uint8_t subID = mapChipField_.GetMapChipSubIDByIndex(x, y);

                EnemyType eType = EnemyType::Type0;
                if (subID == 1) {
                    eType = EnemyType::Type1;
                }

                Enemy* enemy = new Enemy();
                enemy->Initialize(object3dCommon_, camera_, position, eType);
                enemies_.push_back(enemy);
            }
        }
    }
    for (uint32_t y = 0; y < mapChipField_.numBlockVertical_; ++y) {
        uint32_t x = 0;
        while (x < mapChipField_.numBlockHorizontal_) {
            MapChipType t = mapChipField_.GetMapChipTypeByIndex(x, y);
            if (t != MapChipType::kMoveHorizontal) {
                ++x;
                continue;
            }

            // 找这一条连续 5 的长度
            uint32_t startX = x;
            uint32_t endX = x;
            while (endX + 1 < mapChipField_.numBlockHorizontal_ &&
                mapChipField_.GetMapChipTypeByIndex(endX + 1, y) == MapChipType::kMoveHorizontal) {
                ++endX;
            }

            int length = static_cast<int>(endX - startX + 1);

            Vector3 leftPos = mapChipField_.GetMapChipPositionByIndex(startX, y);
            Vector3 rightPos = mapChipField_.GetMapChipPositionByIndex(endX, y);
            Vector3 center{};
            center.x = (leftPos.x + rightPos.x) * 0.5f;
            center.y = leftPos.y;
            center.z = leftPos.z;

            auto* platform = new MovingPlatform();
            platform->Initialize(
                object3dCommon_,
                camera_,
                center,
                MovingPlatform::Axis::Horizontal,
                movingPlatformSpeed_,   // 右→左速度，负数就反向
                length
            );
            movingPlatforms_.push_back(platform);

            x = endX + 1; // 跳过这一段
        }
    }

    // === 生成上下移动平台（连续的 kMoveUD 一条为一个 MovingPlatform）===
    for (uint32_t y = 0; y < mapChipField_.numBlockVertical_; ++y) {
        uint32_t x = 0;
        while (x < mapChipField_.numBlockHorizontal_) {
            MapChipType t = mapChipField_.GetMapChipTypeByIndex(x, y);
            if (t != MapChipType::kMoveVertical) {
                ++x;
                continue;
            }

            // 找这一行里连续 6 的长度
            uint32_t startX = x;
            uint32_t endX = x;
            while (endX + 1 < mapChipField_.numBlockHorizontal_ &&
                mapChipField_.GetMapChipTypeByIndex(endX + 1, y) == MapChipType::kMoveVertical) {
                ++endX;
            }

            int length = static_cast<int>(endX - startX + 1);

            Vector3 leftPos = mapChipField_.GetMapChipPositionByIndex(startX, y);
            Vector3 rightPos = mapChipField_.GetMapChipPositionByIndex(endX, y);
            Vector3 center{};
            center.x = (leftPos.x + rightPos.x) * 0.5f;   // 中点
            center.y = leftPos.y;
            center.z = leftPos.z;

            auto* platform = new MovingPlatform();
            platform->Initialize(
                object3dCommon_,
                camera_,
                center,
                MovingPlatform::Axis::Vertical,          // ★ 上下移动
                movingPlatformSpeed_,
                length                                    // ★ 这一条上有多少个 6
            );
            movingPlatforms_.push_back(platform);

            x = endX + 1; // 跳过这一段 6
        }
    }
}

void GameScene::Initialize() {
    winApp_ = WinApp::GetInstance();
    dxCommon_ = DirectXCommon::GetInstance();
    input_ = Input::GetInstance();
    srvManager_ = SrvManager::GetInstance();

    spriteCommon_ = SpriteCommon::GetInstance();


    TextureManager::GetInstance()->Initialize(dxCommon_, srvManager_);


    const std::string kSkyTexPath = "Resources/sky_bg.png";

        backgroundSprite_ = new Sprite();
        backgroundSprite_->Initialize(spriteCommon_, kSkyTexPath);

        // 左上角对齐屏幕
        backgroundSprite_->SetPosition({ 0.0f, 0.0f });
        backgroundSprite_->SetSize({
            (float)WinApp::kClientWidth,
            (float)WinApp::kClientHeight
            });
    imguiManager_ = new ImGuiManager();
    imguiManager_->Initialize(winApp_, dxCommon_, srvManager_);

    object3dCommon_ = new Object3dCommon();
    object3dCommon_->Initialize(dxCommon_);

    ModelManager::GetInstants()->Initialize(dxCommon_);
    SoundManager* soundMgr = SoundManager::GetInstance();
    soundMgr->Initialize();
    soundMgr->LoadWav("fanfare", "resources/fanfare.wav");

    camera_ = new Camera();
    camera_->SetRotate({ 0, 0, 0 });
    object3dCommon_->SetDefaultCamera(camera_);

    ModelManager::GetInstants()->LoadModel("cube/cube.obj");
    ModelManager::GetInstants()->LoadModel("player/player.obj");
    ModelManager::GetInstants()->LoadModel("door/Door.obj");
    ModelManager::GetInstants()->LoadModel("strip/strip.obj");        // 载入模型
    ModelManager::GetInstants()->LoadModel("coin/coin.obj");
    ModelManager::GetInstants()->LoadModel("coin_ui/coin_ui.obj");
    ModelManager::GetInstants()->LoadModel("snow/snow.obj");
    ModelManager::GetInstants()->LoadModel("jump/jump.obj");
    ModelManager::GetInstants()->LoadModel("star/star.obj");
    ModelManager::GetInstants()->LoadModel("hurd/hurd.obj");
    ModelManager::GetInstants()->LoadModel("cube2/cube2.obj"); 
    ModelManager::GetInstants()->LoadModel("water/water.obj");
    ModelManager::GetInstants()->LoadModel("enemy0/enemy0.obj");
    ModelManager::GetInstants()->LoadModel("enemy1/enemy1.obj");

    player_ = new Player();
    player_->Initialize(object3dCommon_, camera_);
    // === HP 3D 条管理器 ===
    hpBar_ = new HPBar3DManager();
    hpBar_->Initialize(object3dCommon_, camera_, player_, hpNdcZ_);

    playerCamera_ = new PlayerCamera();
    playerCamera_->Initialize(camera_, player_, &mapChipField_);
    playerCamera_->SetOffset({ 0, 0.0f, -40.0f });
    playerCamera_->SetFollowSpeed(0.1f);
    playerCamera_->SetConstrainToMap(true);

    prevCameraPos_ = camera_->GetTransform().translate;

     // === 冲刺技能 UI 管理器 ===
    dashUI_ = new DashUIManager();
    dashUI_->Initialize(spriteCommon_, player_);

     // === Coin UI 管理器 ===
    coinUI_ = new CoinUIManager();
    coinUI_->Initialize(spriteCommon_, object3dCommon_, camera_, hpNdcZ_);

    // 一开始显示当前总金币数（通常是 0）
    coinUI_->SetTotalCoin(totalCoinCollected_);

      // === Hint UI 管理器 ===
    hintUI_ = new HintUIManager();
    hintUI_->Initialize(spriteCommon_, camera_);

    // 把 GameScene 里的 HintSprite 指针交给管理器
    hintUI_->SetSpaceHint(&spaceHint_);
    hintUI_->SetShiftHint(&shiftHint_);
    hintUI_->SetSprintHint(&sprintHint_);
    hintUI_->SetUpHints(&upHints_);

        // === Item 管理器 ===
    itemMgr_ = new ItemManager();
    itemMgr_->Initialize(object3dCommon_, camera_);
     // === Portal 管理器 ===
    portalMgr_ = new PortalManager();
    portalMgr_->Initialize(spriteCommon_, camera_);

    isMapLoading_ = false;
    loadingTimer_ = 0.0f;

       // === Fade 管理器 ===
    fade_ = new FadeManager();
    fade_->Initialize(spriteCommon_);

       // === Intro 管理器 ===
    intro_ = new IntroManager();
    intro_->Initialize(spriteCommon_, input_);

    // === GameOver 管理器 ===
    gameOver_ = new GameOverManager();
    gameOver_->Initialize(spriteCommon_);

    // === GameClear 管理器 ===
    gameClear_ = new GameClearManager();
    gameClear_->Initialize(spriteCommon_, object3dCommon_, camera_, hpNdcZ_);

    particleMgr_ = new ParticleManager();
    particleMgr_->Initialize(object3dCommon_, spriteCommon_);
    emitter2D_ = particleMgr_->CreateEmitter();  // 用来发 2D 粒子
    emitter3D_ = particleMgr_->CreateEmitter();  // 用来发 3D 粒子
    windEmitter_ = particleMgr_->CreateEmitter();
    snowEmitter_ = particleMgr_->CreateEmitter();
    dashStarEmitter_ = particleMgr_->CreateEmitter();
    if (windEmitter_) {
        windEmitter_->SetWindMode(true);
        windEmitter_->SetUseOriginalSpriteSize(true);
        windEmitter_->SetMaxParticles(40);
    }
    
    if (snowEmitter_) {
        // 让这个发射器进入“雪花模式”，下面会在 ParticleEmitter 里实现
        snowEmitter_->SetSnowMode(true);
        // 同屏最多 200 片雪，别太多
        snowEmitter_->SetMaxParticles(200);
        snowEmitter_->SetFollowCamera(true);
    }
    if (dashStarEmitter_) {
        dashStarEmitter_->SetMaxParticles(150);   // 自己喜欢可以再调
        dashStarEmitter_->SetSnowMode(false);
        dashStarEmitter_->SetWindMode(false);
        dashStarEmitter_->SetFollowCamera(false); // 尾气留在世界里即可
    }
    // === Hub（map2）的关卡配置 ===
    hubStageByMap_.clear();
    //  第1关: map3.csv
    //  第2关: map4.csv
    //  第3关: map5.csv
    //  最终关: map6.csv
    hubStageByMap_["Resources/map/map3.csv"] = 0; // Stage 0
    hubStageByMap_["Resources/map/map4.csv"] = 1; // Stage 1
    hubStageByMap_["Resources/map/map5.csv"] = 2; // Stage 2
    hubStageByMap_["Resources/map/map6.csv"] = 3; // Stage 3 (最终关)
    hubProgress_ = 0;
    allStagesCleared_ = false;

    playerIndexHistoryCursor_ = 0;
    playerIndexHistoryInitialized_ = false;
    playerIndexOneSecAgo_ = MapChipField::IndexSet{};
}

void GameScene::Update() {
    const float deltaTime = 1.0f / 60.0f;
    input_->Update();
    backgroundSprite_->Update();
    // —— 是否允许玩家操作（淡出/加载/淡入期间 & 开场演出期间都禁止）——
    const bool isFading = (fade_ && fade_->GetPhase() != FadePhase::None);
    const bool inIntro    = (intro_ && intro_->IsPlaying());
    const bool inGameOver  = (gameOver_ && gameOver_->IsPlaying());
    const bool inGameClear = (gameClear_ && gameClear_->IsPlaying());
    const bool canControl = !(isFading || inIntro || inGameOver || inGameClear);
   
       // ===== Intro 驱动（在加载/淡出等早退之前执行，但不盖过Loading）=====
    if (fade_ && fade_->GetPhase() == FadePhase::None && intro_) {
        intro_->Update(deltaTime);
        // Intro 自己会更新内部 sprite 的属性，这里不用再手动 Update
    }

      // ===== 画面淡入淡出状态机（优先执行）=====
    if (fade_ && fade_->GetPhase() == FadePhase::FadingOut) {
        // 1) alpha 逐帧增加
        float a = fade_->GetAlpha();
        a += fade_->GetSpeed();
        if (a > 1.0f) a = 1.0f;
        fade_->SetAlpha(a);

        // 2) 完全变黑后的处理
        if (a >= 1.0f) {
            // 2-1) 在纯黑上停留若干帧
            if (!fade_->ReachedBlack()) {
                fade_->SetReachedBlack(true);
                // 第一次到纯黑，先 return，让这一帧只显示纯黑
                return;
            } else if (fade_->GetBlackHoldFrames() > 0) {
                fade_->SetBlackHoldFrames(fade_->GetBlackHoldFrames() - 1);
                return;
            }

            // 2-2) 从 GameClear / GameOver 回标题
            if (returnToTitle_) {
                returnToTitle_ = false;
                if (sceneManager_) {
                    sceneManager_->ClearOverlayScene();
                    sceneManager_->SetNextScene(std::make_unique<TitleScene>());
                }
                return;
            }

            // 2-3) 推入 Loading 叠加场景（只推一次）
            if (!fade_->OverlayPushed()) {
                if (!pendingGameClear_ && !returnToTitle_) {
                    if (sceneManager_) {
                        sceneManager_->SetOverlayScene(std::make_unique<LoadingScene>());
                    }
                }
                fade_->SetOverlayPushed(true);
            }

            // 2-4) 传送门：在此刻真正开始加载
            if (pendingPortalLoad_) {
                pendingPortalLoad_ = false;
                StartLoadingMap(pendingPortalMapPath_, pendingPortalStartPos_, true);
                fade_->SetPhase(FadePhase::LoadingHold);
                return;
            }

            // 2-5) 通关：全黑状态下启动 GameClear 演出
            if (pendingGameClear_) {
                pendingGameClear_ = false;
                if (sceneManager_) {
                    sceneManager_->ClearOverlayScene();
                }
                if (gameClear_ && !gameClear_->IsPlaying()) {
                    gameClear_->Start();
                }
                // 保持黑幕为全黑，由 GameClear 自己画背景
                fade_->SetAlpha(1.0f);
                fade_->SetPhase(FadePhase::None);
                return;
            }

            // 2-6) 普通情况：从全黑切到淡入
            fade_->SetPhase(FadePhase::FadingIn);
            return;
        }

        // 还在从 0 → 1 的过程中
        return;
    }

    // 在本帧后段会处理 FadingIn（如下）
    if (shouldStartLoading_) {
        shouldStartLoading_ = false;
        StartLoadingMap("Resources/map/map.csv", { 3,3,0 }, false);
        return; // 本帧先显示 LoadingScene
    }
    // 2️⃣ 初始加载计时
    if (isMapLoading_) {
        loadingTimer_ += deltaTime;
        if (loadingTimer_ >= LOADING_DURATION) {
            isMapLoading_ = false;

            // 真正加载地图
            LoadMap("Resources/map/map.csv", { 3,3,0 });
            if (sceneManager_) sceneManager_->ClearOverlayScene();
            if (fade_) fade_->SetPhase(FadePhase::FadingIn);

            if (player_) {
                player_->ResetForMapTransition(true);
            }
            if (input_) {
                input_->ResetAllKeys();
            }
        }
        else {
            if (fade_) fade_->Update(deltaTime);
            return;
        }
    }


    // 3️⃣ 传送门加载计时
    if (isPortalLoading_) {
        portalLoadingTimer_ += deltaTime;
        if (portalLoadingTimer_ >= LOADING_DURATION) {
            isPortalLoading_ = false;

            // 真正加载地图
            LoadMap(portalMapPath_, portalStartPos_);
            if (sceneManager_) sceneManager_->ClearOverlayScene();
            if (fade_) fade_->SetPhase(FadePhase::FadingIn);

            if (player_) {
                player_->ResetForMapTransition(true);
            }
            if (input_) {
                input_->ResetAllKeys();
            }
        }
        else {
            if (fade_) {
                fade_->Update(deltaTime);
                return;
            }
        }
    }
    imguiManager_->Begin();
    playerCamera_->Update();

    for (auto* p : movingPlatforms_) {
        p->Update(deltaTime, mapChipField_, movingPlatforms_);
    }
    for (auto* e : enemies_) {
        if (e) {
            e->Update(deltaTime);
        }
    }
    // 淡入淡出/加载/演出期间都不可操作
    player_->Update(canControl ? input_ : nullptr, mapChipField_);

  // ========= 阶段1：两条移动平台互相夹住玩家 =========
    crushedByPlatformThisFrame_ = false;
    damagedByEnemyThisFrame_    = false;
    if (player_ && !player_->IsDead() && !player_->IsInvincible()) {

        Vector3 pPos = player_->GetPosition();
        float halfW = player_->GetWidth() * 0.5f;
        float halfH = player_->GetHeight() * 0.5f;

        float left   = pPos.x - halfW;
        float right  = pPos.x + halfW;
        float bottom = pPos.y - halfH;
        float top    = pPos.y + halfH;

        int overlapPlatformCount = 0;
        for (auto* plat : movingPlatforms_) {
            if (!plat) continue;
            MapChipField::Rect r = plat->GetRect();
            bool overlapX = !(right <= r.left || left >= r.right);
            bool overlapY = !(top   <= r.bottom || bottom >= r.top);
            if (overlapX && overlapY) {
                ++overlapPlatformCount;
                if (overlapPlatformCount >= 2) {
                    // 同时和两条平台重叠 ⇒ 直接判定为被夹住
                    crushedByPlatformThisFrame_ = true;
                    break;
                }
            }
        }
    }
     // ========= 玩家与敌人的碰撞 =========
    if (player_ && !player_->IsDead()) {

        Vector3 pPos = player_->GetPosition();
        float   pHalfW = player_->GetWidth() * 0.5f;
        float   pHalfH = player_->GetHeight() * 0.5f;
        Vector3 pVel = player_->GetVelocity();

        float pLeft = pPos.x - pHalfW;
        float pRight = pPos.x + pHalfW;
        float pBottom = pPos.y - pHalfH;
        float pTop = pPos.y + pHalfH;

        for (auto* enemy : enemies_) {
            if (!enemy) { continue; }

            Vector3 ePos = enemy->GetPosition();
            float   eHalfW = enemy->GetWidth() * 0.5f;
            float   eHalfH = enemy->GetHeight() * 0.5f;

            float eLeft = ePos.x - eHalfW;
            float eRight = ePos.x + eHalfW;
            float eBottom = ePos.y - eHalfH;
            float eTop = ePos.y + eHalfH;

            bool overlapX = !(pRight <= eLeft || pLeft >= eRight);
            bool overlapY = !(pTop <= eBottom || pBottom >= eTop);
            if (!overlapX || !overlapY) {
                continue;
            }

            // ====== 判定是否“从上方踩到”敌人 ======
            const float stompTolerance = 0.15f;
            bool isStomp =
                (pVel.y <= 0.0f) &&          // 玩家正在往下
                (pBottom >= ePos.y) &&       // 脚大致在敌人中间以下
                (pBottom <= eTop + stompTolerance);

            if (isStomp) {
                // ☆ 踩到敌人：敌人闪烁，玩家弹一下，不受伤
                enemy->StartHitReaction(0.4f);   // 闪烁 0.4 秒，可自己调

                Vector3 newVel = pVel;
                newVel.y = 0.7f;                // 踩完向上弹的力度，可自己调手感
                player_->SetVelocity(newVel);

                // 不给玩家伤害，处理完当前敌人就继续下一个
                continue;
            }
            else {
                // ☆ 不是从上面踩 ⇒ 视为被敌人撞到，玩家受伤
                if (!player_->IsInvincible()) {
                    damagedByEnemyThisFrame_ = true;
                }
            }
        }
    }
    // 先做平台上的落地 / 分离 / 搭乘
    if (!crushedByPlatformThisFrame_) {
        HandlePlayerOnMovingPlatforms();
    }

    // ========= 阶段2：被平台推进方块 / 刺 / 门，或者推出地图边界 =========
    if (player_ && !player_->IsDead() && !player_->IsInvincible()) {
        Vector3 pPos = player_->GetPosition();
        float halfW = player_->GetWidth() * 0.5f;
        float halfH = player_->GetHeight() * 0.5f;

        float left   = pPos.x - halfW;
        float right  = pPos.x + halfW;
        float bottom = pPos.y - halfH;
        float top    = pPos.y + halfH;

        // 2a) 若此时玩家 AABB 已经扎进任何 Block/Spike/Portal，就认为是被平台挤进去
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

                // Block / Block2：整格
                if (t == MapChipType::kBlock || t == MapChipType::kBlock2) {
                    bool overlapX = !(right <= r.left || left >= r.right);
                    bool overlapY = !(top <= r.bottom || bottom >= r.top);
                    if (overlapX && overlapY) {
                        crushedByPlatformThisFrame_ = true;
                        break;
                    }
                }
                // Spike：只用底部 kSpikeHeightRatio 的高度
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


        // 2b) 被平台推到地图外（超出上下边界）也算挤压
        if (!crushedByPlatformThisFrame_) {
            Vector3 mapMin = mapChipField_.GetMapMinPosition();
            Vector3 mapMax = mapChipField_.GetMapMaxPosition();

            // 这里加一点点裕度，避免浮点抖动
            if (top > mapMax.y + 0.01f || bottom < mapMin.y - 0.01f) {
                crushedByPlatformThisFrame_ = true;
            }
        }
    }

    if (particleMgr_ && emitter3D_ && player_->ConsumeDoubleJumpEvent()) {
        Vector3 playerPos = player_->GetPosition();

        // 可以微微抬高一点，让粒子从身体附近炸开
        Vector3 spawnPos = playerPos + Vector3{ 0.0f, 0.8f, 0.0f };
        float horizontalBias = player_->IsFacingRight() ? -1.0f : 1.0f;
        emitter3D_->Emit(
            14,                        // 粒子数量，自己喜欢可以调 12~24
            ParticleType::Model3D,     // 3D 粒子
            "jump/jump.obj",           // 暂定使用 cube 模型
            spawnPos,
            4.0f, 8.0f,                // 速度范围：向四周炸开的感觉
            0.25f, 0.45f,horizontalBias                 // 生命周期：短一点，看起来干脆利落
        );
    }

    camera_->Update();
    // === 冲刺星星尾气 ===
    if (dashStarEmitter_ && player_->IsDashing()) {
        Vector3 pos = player_->GetPosition();
        float h = player_->GetHeight();

        // 脚附近
        pos.y -= h * 0.4f;

        // 根据冲刺方向，把生成点往“身后”挪一点
        Vector3 dashDir = player_->GetDashDirection(); // x>0 右冲, x<0 左冲
        pos.x -= dashDir.x * 0.8f;

        dashStarEmitter_->Emit(
            1,                      // 每帧几颗星星
            ParticleType::Model3D,
            "star/star.obj",
            pos,
            2.0f, 4.0f,             // 速度
            0.08f, 0.16f,            // 寿命
            0.0f,true
        );
    }
     {
        Vector3 camPos   = camera_->GetTransform().translate;
        Vector3 camDelta = camPos - prevCameraPos_;
        prevCameraPos_   = camPos;

        if (snowEmitter_) {
            snowEmitter_->ApplyCameraMove(camDelta);
        }
    }
    // 若尚未进入GameOver，检测HP
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
     // GameOver 状态机推进（交给管理器）
    if (gameOver_) {
        gameOver_->Update(deltaTime);
    }
    // GameClear 状态机推进
    if (gameClear_) {
        gameClear_->Update(deltaTime);
    }
   // ==== 刮风粒子效果（整屏 & 只在 map 中）====
    if (windEmitter_ && currentMapPath_ == "Resources/map/map5.csv") {

        windSpawnTimer_ -= deltaTime;
        if (windSpawnTimer_ <= 0.0f) {
            // 发射频率稍微高一点，整个屏幕都会有风线
            windSpawnTimer_ = 0.1f;   // 约 33 条/秒，可自行调

            const float margin = 50.0f;

            // 发射区域：屏幕右上角一块区域
            // x 在 画面右侧 60% ~ 画面外一点
            float spawnX = RandRangeFloat(
                WinApp::kClientWidth * 0.6f,
                WinApp::kClientWidth + margin
            );
            // y 在 画面上方 ~ 上半部分
            float spawnY = RandRangeFloat(
                -margin,
                WinApp::kClientHeight * 0.4f
            );

            Vector3 spawnPos = { spawnX, spawnY, 0.0f };

            // 这里的 min/maxSpeed 是“每秒移动的像素量”，
            // 取 700~1200 左右，可以在 1~1.5 秒内从右上飘到左下
            windEmitter_->Emit(
                1,
                ParticleType::Sprite2D,
                "Resources/wind.dds",
                spawnPos,
                700.0f, 1200.0f,   // 速度 (px/s)
                1.0f, 1.5f         // 生命周期 (秒)
            );
        }
    }
    if (snowEmitter_ && currentMapPath_ == "Resources/map/map5.csv") {

        // 想要的平均发射间隔（秒）
        const float snowInterval = 0.1f;   // 约 25 次/秒，可以自己调

        snowSpawnTimer_ -= deltaTime;

        // 用 while 是为了兼容帧率波动，
        // 当前你的 deltaTime 固定 1/60，也没问题
        while (snowSpawnTimer_ <= 0.0f) {
            snowSpawnTimer_ += snowInterval;

            const float margin = 75.0f;

            float screenX = RandRangeFloat(
                -margin,
                WinApp::kClientWidth + margin
            );
            float screenY = -margin;

            float ndcZ = RandRangeFloat(0.45f, 0.65f);

            Vector3 worldPos = ScreenToWorld(screenX, screenY, ndcZ, camera_);

            // ✅ 每次只发 1 片（或者 2 片），多次累积起来就是“连续的雪”
            snowEmitter_->Emit(
                1,                          // 改小：1~2 片
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
        // === 在 GameClear 演出期间按 Space → 回标题 ===
    if (gameClear_ && gameClear_->IsPlaying() && input_ && input_->TriggerKey(DIK_SPACE)) {

        if (sceneManager_) {
            sceneManager_->ClearOverlayScene();
            sceneManager_->SetNextScene(std::make_unique<TitleScene>());
        }
    }

    // === 在 GameOver 演出期间按 Space → 回标题 ===
    if (gameOver_ && gameOver_->IsPlaying() && input_ && input_->TriggerKey(DIK_SPACE)) {

        if (!returnToTitle_ && fade_) {
            returnToTitle_ = true;

            // 重置黑幕参数，开始淡出到纯黑
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


    if (hpBar_) {
        hpBar_->Update(deltaTime);
    }
    for (auto* block : mapBlocks_) {
        block->Update();
    }
    for (auto* water : waterBlocks_) {
        water->Update();
    }
    if (itemMgr_) {
        itemMgr_->Update(deltaTime);
    }

    MapChipField::IndexSet playerIndex =
        mapChipField_.GetMapChipIndexByPosition(player_->GetPosition());
  // ===== 记录玩家所在格子的历史，用于“回到 1 秒前的位置” =====
    if (!playerIndexHistoryInitialized_) {
        // 初次：用当前格子填满整个缓冲区，避免读到垃圾数据
        for (int i = 0; i < kPlayerIndexHistoryFrameCount_; ++i) {
            playerIndexHistory_[i] = playerIndex;
        }
        playerIndexHistoryInitialized_ = true;
        playerIndexHistoryCursor_ = 0;
        playerIndexOneSecAgo_ = playerIndex;
    }

    // 1 秒前所在的格子 = 当前写入位置里存放的旧值
    playerIndexOneSecAgo_ = playerIndexHistory_[playerIndexHistoryCursor_];

    // 将本帧格子写入历史，并推进游标
    playerIndexHistory_[playerIndexHistoryCursor_] = playerIndex;
    playerIndexHistoryCursor_++;
    if (playerIndexHistoryCursor_ >= kPlayerIndexHistoryFrameCount_) {
        playerIndexHistoryCursor_ = 0;
    }

    // ===== 踩到地刺：扣血 1 格 & 回到 1 秒前所在格子 =====
    {
        MapChipType tileType =
            mapChipField_.GetMapChipTypeByIndex(playerIndex.xIndex, playerIndex.yIndex);

        bool onSpike = false;

        // 只对地刺格做“矮一点的判定区域”
        if (tileType == MapChipType::kSpike) {

            // 1) 地刺所在格子的矩形
            MapChipField::Rect spikeRect =
                mapChipField_.GetRectByIndex(playerIndex.xIndex, playerIndex.yIndex);

            // 2) 玩家当前 AABB
            Vector3 pPos = player_->GetPosition();
            float halfW = player_->GetWidth() * 0.5f;
            float halfH = player_->GetHeight() * 0.5f;

            float left = pPos.x - halfW;
            float right = pPos.x + halfW;
            float bottom = pPos.y - halfH;
            float top = pPos.y + halfH;

            // 3) 只用格子高度的 60% 作为“有效地刺高度”
            float tileHeight = spikeRect.top - spikeRect.bottom;

            MapChipField::Rect hitRect = spikeRect;
            hitRect.top = hitRect.bottom + tileHeight * kSpikeHeightRatio;

            // 4) 玩家 AABB 和“缩短后的地刺矩形”做重叠判定
            bool overlapX = !(right <= hitRect.left || left >= hitRect.right);
            bool overlapY = !(top <= hitRect.bottom || bottom >= hitRect.top);

            onSpike = overlapX && overlapY;
        }

        // 被平台夹死照旧用 crushedByPlatformThisFrame_（全格判定）
        if ((onSpike || crushedByPlatformThisFrame_|| damagedByEnemyThisFrame_) &&
            !player_->IsDead() && !player_->IsInvincible()) {

            MapChipField::IndexSet safeIndex = playerIndexOneSecAgo_;

            // 如果 1 秒前也在地刺上，就再往更早找一个“不是地刺”的格子
            MapChipType safeType =
                mapChipField_.GetMapChipTypeByIndex(safeIndex.xIndex, safeIndex.yIndex);

            if (safeType == MapChipType::kSpike|| safeType == MapChipType::kEnemy) {
                MapChipField::IndexSet firstNonSpike{};
                bool found = false;

                for (int i = 0; i < kPlayerIndexHistoryFrameCount_; ++i) {
                    const auto& candidate = playerIndexHistory_[i];
                    MapChipType type =
                        mapChipField_.GetMapChipTypeByIndex(candidate.xIndex, candidate.yIndex);
                    if (type != MapChipType::kSpike) {
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

            // 让玩家站在这个格子的“上表面”
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
        // 更新传送门提示图标（是否显示 + 位置）
    const PortalInfo* currentPortal = nullptr;
    if (portalMgr_) {
        portalMgr_->UpdateHint(playerIndex, player_->GetPosition(), canControl);
        currentPortal = portalMgr_->GetPortalAt(playerIndex);
    }

    if (itemMgr_) {
        bool picked = itemMgr_->OnPlayerStepOnTile(currentMapPath_, playerIndex, mapChipField_, player_);
        if (picked) {
            ++totalCoinCollected_;
            if (coinUI_) {
                coinUI_->SetTotalCoin(totalCoinCollected_);
            }
        }
    }
    if (currentPortal) {
        // 玩家正站在某个门格子上
        if (canControl && input_->TriggerKey(DIK_E)) {
            // ==== 如果是从子关卡回到 Hub(map2)，更新解锁进度 ====
            auto itStage = hubStageByMap_.find(currentMapPath_);
            bool triggerGameClear = false;
            if (currentPortal->targetMap == "Resources/map/map2.csv" && itStage != hubStageByMap_.end()) {
                int stageIndex = itStage->second;   // 这是第几关(0~3)
                if (hubProgress_ < stageIndex + 1) {
                    hubProgress_ = stageIndex + 1;
                    if (hubProgress_ >= 4) {
                        allStagesCleared_ = true;
                        triggerGameClear = true;
                    }
                }
            }

            if (triggerGameClear) {
                if (!pendingGameClear_ && gameClear_ && !gameClear_->IsPlaying() && fade_) {
                    pendingGameClear_ = true;

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
            else {
                // ==== 普通传送处理 ====
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
    }
    if (dashUI_) {
        dashUI_->Update(deltaTime);
    }
    if (input_->TriggerKey(DIK_P)) {
        SoundManager::GetInstance()->Play("fanfare", false, 1.0f);
    }
    // ===== FadingIn：从全黑淡入 =====
    if (fade_ && fade_->GetPhase() == FadePhase::FadingIn) {
        float a = fade_->GetAlpha();
        a -= fade_->GetSpeed();
        if (a < 0.0f) {
            a = 0.0f;
            fade_->SetPhase(FadePhase::None); // 淡入完成

            // 淡入完成→启动开场演出（只启动一次）
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
        // 位置信息
        Vector3 playerPos = player_->GetPosition();
        float playerPosArr[3] = { playerPos.x, playerPos.y, playerPos.z };
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", playerPos.x, playerPos.y, playerPos.z);
        MapChipField::IndexSet playerIndex = mapChipField_.GetMapChipIndexByPosition(player_->GetPosition());
        ImGui::Text("MapChip Index: (%d, %d)", playerIndex.xIndex, playerIndex.yIndex);

        // 添加当前格子类型信息
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

    // 是否处于 GameClear 演出中
    bool inGameClear = (gameClear_ && gameClear_->IsPlaying());
     // ================== 0) 背景天空（2D Sprite） ==================
    // 先切到 SRV / Sprite 的绘制状态，然后画一个全屏的天空
    srvManager_->PreDraw();
    spriteCommon_->CommonDraw();
    if (backgroundSprite_) {
        backgroundSprite_->Draw();
    }
    dxCommon_->ClearDepthBuffer();
    // ================== 1) 3D 场景（地图、HP 3D条等） ==================
    srvManager_->PreDraw();
    object3dCommon_->CommonDraw();

    // 地图方块
    for (auto* block : mapBlocks_) {
        block->Draw();
    }
    for (auto* p : movingPlatforms_) {
        p->Draw();
    }
    // === 敌人 ===
    for (auto* e : enemies_) {
        if (e) {
            e->Draw();
        }
    }
    // 道具
    if (itemMgr_) {
        itemMgr_->Draw3D();
    }
    // HP 3D 条段
    if (hpBar_) {
        hpBar_->Draw3D();
    }
    // ================== 1.5) GameClear 用全屏黑背景 ==================
    // 盖住上面的地图、道具等，让背景变成纯黑
    if (inGameClear && fade_) {
        Sprite* s = fade_->GetSprite();
        spriteCommon_->CommonDraw();
        if (s) {
            s->SetVisible(true); // 如果有需要，让它重新可见

            s->Draw();
        }
    }

    // ================== 2) 中间层：交互提示 Sprite ==================
    spriteCommon_->CommonDraw();
    if (!inGameClear) {
       if (hintUI_) {
            hintUI_->Draw();
        }

        // 冲刺技能 UI
        if (dashUI_) {
            dashUI_->Draw();
        }

        // Coin 数字 UI（冒号 + 数字）
        if (coinUI_) {
            coinUI_->Draw2D();
        }

        // 传送门提示
        if (portalMgr_) {
            portalMgr_->DrawHint();
        }
    }
    // ================== 3) 前景 3D：玩家（盖住提示） ==================
    dxCommon_->ClearDepthBuffer();

    srvManager_->PreDraw();
    object3dCommon_->CommonDraw();

     // 如果 GameClear 正在播放，就画 GameClear 的玩家，否则画正常玩家
    if (gameClear_ && gameClear_->IsPlaying()) {
        gameClear_->DrawPlayer();
    }
    else {
        player_->Draw();
        // ==== Coin UI：右上角的 3D coin 模型 ====
        if (coinUI_) {
            coinUI_->Draw3D();
        }
        if (particleMgr_) {
            particleMgr_->Draw3D();
        }
        for (auto* water : waterBlocks_) {
            if (water) {
                water->Draw();
            }
        }
    }
    // ================== 4) 最前景 UI Sprite ==================
    spriteCommon_->CommonDraw();

    // Intro / 黑边 / 暗角 / 标题 / Skip 提示
    if (intro_) {
        intro_->Draw();
    }

    // 黑幕淡入淡出（GameClear 时不用这个黑幕，避免挡住胜利画面）
    if (fade_  && !inGameClear) {
        fade_ ->Draw();
    }

    // GameOver
    if (gameOver_) {
        gameOver_->Draw();
    }

    // GameClear
    if (gameClear_) {
        gameClear_->DrawTitle();
    }
    if (particleMgr_) {
        particleMgr_->Draw2D();
    }
    // ImGui（debug UI）
    imguiManager_->Draw();

    dxCommon_->End();
}

void GameScene::Finalize() {
    SoundManager::GetInstance()->Finalize();
    TextureManager::GetInstance()->Finalize();
    ModelManager::GetInstants()->Finalize();
    imguiManager_->Finalize();

    delete camera_;
    delete playerCamera_;
    delete object3dCommon_;
    if (backgroundSprite_) {
        delete backgroundSprite_;
        backgroundSprite_ = nullptr;
    }

    for (auto* block : mapBlocks_) {
        delete block;
    }
    mapBlocks_.clear();
    for (auto* water : waterBlocks_) {
        delete water;
    }
    waterBlocks_.clear();
    for (auto* e : enemies_) {
        delete e;
    }
    enemies_.clear();

    delete player_;
    delete imguiManager_;
    if (dashUI_) {
        dashUI_->Finalize();
        delete dashUI_;
        dashUI_ = nullptr;
    }
    if (portalMgr_) {
        portalMgr_->Finalize();
        delete portalMgr_;
        portalMgr_ = nullptr;
    }
    if (fade_) {
        fade_->Finalize();
        delete fade_;
        fade_ = nullptr;
    }

    if (intro_) {
        intro_->Finalize();
        delete intro_;
        intro_ = nullptr;
    }

    if (hpBar_) {
        hpBar_->Finalize();
        delete hpBar_;
        hpBar_ = nullptr;
    }
    if (gameOver_) {
        gameOver_->Finalize();
        delete gameOver_;
        gameOver_ = nullptr;
    }

    if (spaceHint_.sprite) {
        delete spaceHint_.sprite;
        spaceHint_.sprite = nullptr;
    }
    for (auto& h : upHints_) {
        if (h.sprite) {
            delete h.sprite;
            h.sprite = nullptr;
        }
    }
    upHints_.clear();
    if (shiftHint_.sprite) {
        delete shiftHint_.sprite;
        shiftHint_.sprite = nullptr;
    }
    if (sprintHint_.sprite) {
        delete sprintHint_.sprite;
        sprintHint_.sprite = nullptr;
    }
    if (itemMgr_) {
        itemMgr_->Finalize();
        delete itemMgr_;
        itemMgr_ = nullptr;
    }
    // ==== Coin UI 资源释放 ====
    if (coinUI_) {
        coinUI_->Finalize();
        delete coinUI_;
        coinUI_ = nullptr;
    }
    if (gameClear_) {
        gameClear_->Finalize();
        delete gameClear_;
        gameClear_ = nullptr;
    }
    if (hintUI_) {
        hintUI_->Finalize();
        delete hintUI_;
        hintUI_ = nullptr;
    }
    if (particleMgr_) {
        particleMgr_->Finalize();
        delete particleMgr_;
        particleMgr_ = nullptr;
        emitter2D_ = nullptr;
        emitter3D_ = nullptr;
        dashStarEmitter_ = nullptr;
    }
    for (auto* p : movingPlatforms_) {
        delete p;
    }
    movingPlatforms_.clear();
}

void GameScene::StartLoadingMap(const std::string& mapPath, const Vector3& startPos, bool isPortal = false) {
    if (sceneManager_) {
        sceneManager_->SetOverlayScene(std::make_unique<LoadingScene>());
    }
    if (isPortal) {
        // 传送门加载
        isPortalLoading_ = true;
        portalMapPath_ = mapPath;
        portalStartPos_ = startPos;
        portalLoadingTimer_ = 0.0f;
    }
    else {
        // 初始化加载
        isMapLoading_ = true;
        loadingTimer_ = 0.0f;
    }
}

void GameScene::LoadMap(const std::string& mapPath, const Vector3& startPos)
{

    // 记录本次地图路径
    currentMapPath_ = mapPath;

    // 清理旧 items 渲染对象
    if (itemMgr_) {
        itemMgr_->ClearVisuals();
    }

    for (auto* block : mapBlocks_) delete block;
    mapBlocks_.clear();
    for (auto* water : waterBlocks_) {
        delete water;
    }
    waterBlocks_.clear();
    for (auto* e : enemies_) delete e;
    enemies_.clear();

    for (auto* p : movingPlatforms_) delete p;
    movingPlatforms_.clear();
    if (spaceHint_.sprite) {
        delete spaceHint_.sprite;
        spaceHint_.sprite = nullptr;
    }
    for (auto& h : upHints_) {
        if (h.sprite) {
            delete h.sprite;
            h.sprite = nullptr;
        }
    }
    upHints_.clear();
    if (shiftHint_.sprite) {
        delete shiftHint_.sprite;
        shiftHint_.sprite = nullptr;
    }
    if (sprintHint_.sprite) {
        delete sprintHint_.sprite;
        sprintHint_.sprite = nullptr;
    }

    mapChipField_.LoadMapChipCsv(mapPath);
    GenerateBlocks();
     // ==== 刷新右上角 Coin UI：显示「总共拾取的 coin 数」 ====
    if (coinUI_) {
        coinUI_->SetTotalCoin(totalCoinCollected_);
    }
    // === 只在 map 生成 Space / Up 提示 ===
    if (mapPath == "Resources/map/map.csv") {

        // (5,2) → space.png
        spaceHint_.sprite = new Sprite();
        spaceHint_.sprite->Initialize(spriteCommon_, "Resources/space2.png");
        spaceHint_.sprite->SetSize({ 64.0f, 64.0f });
        spaceHint_.worldPos = mapChipField_.GetMapChipPositionByIndex(5, 2);
        spaceHint_.worldPos.y += 0.4f;
        // (19,6) → shift.png
        shiftHint_.sprite = new Sprite();
        shiftHint_.sprite->Initialize(spriteCommon_, "Resources/shift.png");
        shiftHint_.sprite->SetSize({ 64.0f, 48.0f });
        shiftHint_.worldPos = mapChipField_.GetMapChipPositionByIndex(19, 6);
        shiftHint_.worldPos.x -= 0.2f;
        shiftHint_.worldPos.y += 0.5f;
        // (20,6) → sprint.png
        sprintHint_.sprite = new Sprite();
        sprintHint_.sprite->Initialize(spriteCommon_, "Resources/sprint.png");
        sprintHint_.sprite->SetSize({ 48.0f, 48.0f });
        sprintHint_.worldPos = mapChipField_.GetMapChipPositionByIndex(20, 6);
        sprintHint_.worldPos.x -= 0.3f;
        sprintHint_.worldPos.y += 0.5f;
        // (6,2) → up.png
        auto makeUpHint = [&](int x, int y) {
            HintSprite h;
            h.sprite = new Sprite();
            h.sprite->Initialize(spriteCommon_, "Resources/up.dds");
            h.sprite->SetSize({ 32.0f, 32.0f });
            h.worldPos = mapChipField_.GetMapChipPositionByIndex(x, y);
            upHints_.push_back(h);
            };

        // (6,2), (11,4), (12,4)
        makeUpHint(6, 2);
        makeUpHint(11, 4);
        makeUpHint(12, 4);
    }
    // === Hub 地图（map2）：只显示一个方向箭头，指向“下一关的门” ===
    else if (mapPath == "Resources/map/map2.csv") {
        // 教学用的 Space/Shift 提示在 Hub 不显示
        spaceHint_.worldPos = { 0,0,0 };
        shiftHint_.worldPos = { 0,0,0 };
        sprintHint_.worldPos = { 0,0,0 };

        int nextX = -1;
        int nextY = -1;

        // 根据 hubProgress_ 决定箭头指向哪扇门
        // 门索引（在 map2 内）：
        //   map3: (11,5)
        //   map4: (14,5)
        //   map5: (23,1)
        //   map6: (12,14)
        if (hubProgress_ <= 0) {
            // 还没通关任何一张 → 指向去 map3 的门
            nextX = 11; nextY = 5;
        }
        else if (hubProgress_ == 1) {
            // 通关了 map3 → 指向去 map4 的门
            nextX = 14; nextY = 5;
        }
        else if (hubProgress_ == 2) {
            // 通关了 map4 → 指向去 map5 的门
            nextX = 23; nextY = 1;
        }
        else if (hubProgress_ == 3) {
            // 通关了 map5 → 指向去最终关 map6 的门
            nextX = 12; nextY = 14;     // 左边那扇门
        }
        else {
            // hubProgress_ >= 4 → 所有关卡通关，不再显示方向
        }

        if (nextX >= 0) {
            HintSprite h;
            h.sprite = new Sprite();
            h.sprite->Initialize(spriteCommon_, "Resources/up.png");
            h.sprite->SetSize({ 32.0f, 32.0f });
            h.sprite->SetRotation(std::numbers::pi_v<float>);
            h.worldPos = mapChipField_.GetMapChipPositionByIndex(nextX, nextY);
            h.worldPos.x += 0.4f;
            h.worldPos.y += 2.0f;   // 稍微抬高一点，在门上方飘
            upHints_.push_back(h);
        }
    }
    else {
        // 不是 map：确保不画提示
        spaceHint_.worldPos = { 0,0,0 };
        shiftHint_.worldPos = { 0,0,0 };
        sprintHint_.worldPos = { 0,0,0 };
    }

    // 设置玩家起点
    player_->SetPosition(startPos);
    player_->ResetForMapTransition(true);
    MapChipField::IndexSet startIndex = mapChipField_.GetMapChipIndexByPosition(startPos);
        for (int i = 0; i < kPlayerIndexHistoryFrameCount_; ++i) {
            playerIndexHistory_[i] = startIndex;
        }
        playerIndexHistoryCursor_ = 0;
        playerIndexHistoryInitialized_ = true;
        playerIndexOneSecAgo_ = startIndex;
    // 相机同步
    camera_->SetTranslate(startPos + Vector3{ 0,0,-40 });
    prevCameraPos_ = camera_->GetTransform().translate;
    playerCamera_->SetMapBounds(mapChipField_.GetMapMinPosition(), mapChipField_.GetMapMaxPosition());
    // 根据当前地图更新传送门列表
    if (portalMgr_) {
        portalMgr_->ClearPortals();
    }

       // ========== map1（起始地图） ==========
    if (mapPath == "Resources/map/map.csv") {
        if (portalMgr_) {
            portalMgr_->AddPortal(
                { 26, 11 },                                    // 当前 map1 里的格子
                "Resources/map/map2.csv",                      // 目标地图（Hub）
                mapChipField_.GetMapChipPositionByIndex(2, 1)  // 在 Hub 中的出生格
            );
        }
    }

    // ========== map2（中心地图 / Hub） ==========
    else if (mapPath == "Resources/map/map2.csv") {
        if (portalMgr_) {
            // Hub → 返回 map1 的门
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

    // ========== 各子关卡内部：返回 Hub ==========
    else {
        if (portalMgr_) {
            if (mapPath == "Resources/map/map3.csv") {
                portalMgr_->AddPortal(
                    { 2, 1 },
                    "Resources/map/map2.csv",
                    mapChipField_.GetMapChipPositionByIndex(11, 5)
                );
            }
            else if (mapPath == "Resources/map/map4.csv") {
                portalMgr_->AddPortal(
                    { 2, 1 },
                    "Resources/map/map2.csv",
                    mapChipField_.GetMapChipPositionByIndex(14, 5)
                );
            }
            else if (mapPath == "Resources/map/map5.csv") {
                portalMgr_->AddPortal(
                    { 2, 1 },
                    "Resources/map/map2.csv",
                    mapChipField_.GetMapChipPositionByIndex(23, 1)
                );
            }
            else if (mapPath == "Resources/map/map6.csv") {
                portalMgr_->AddPortal(
                    { 2, 1 },
                    "Resources/map/map2.csv",
                    mapChipField_.GetMapChipPositionByIndex(12, 14)
                );
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

    for (auto* platform : movingPlatforms_) {
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

        // 计算 X / Y 方向的最小穿透量
        float penX = (std::min)(r.right - left, right - r.left);
        float penY = (std::min)(r.top - bottom, top - r.bottom);

        if (penX < penY) {
            // 水平方向分离（挡住侧面）
            float centerPlayerX = pos.x;
            float centerRectX = (r.left + r.right) * 0.5f;
            if (centerPlayerX < centerRectX) {
                pos.x -= penX;   // 玩家在左 → 往左推
            }
            else {
                pos.x += penX;   // 玩家在右 → 往右推
            }
            vel.x = 0.0f;
        }
        else {
            // 垂直方向分离（挡住头顶/脚底）
            float centerPlayerY = pos.y;
            float centerRectY = (r.bottom + r.top) * 0.5f;

            if (centerPlayerY < centerRectY) {
                // 从下往上撞到平台底部
                pos.y -= penY;
                if (vel.y > 0.0f) vel.y = 0.0f;
            }
            else {
                // 从上踩到平台 → 当做地面 + 站在上面跟着动
                // 先让 Player 内部把“落地状态”处理好
                player_->SetPosition(pos);
                player_->SetVelocity(vel);
                player_->LandOnExternalGround(r.top);

                // 取一下修正后的 pos / vel
                pos = player_->GetPosition();
                vel = player_->GetVelocity();

                // 把玩家跟着平台一起移动（这一帧平台的位移）
                Vector3 delta = platform->GetPosition() - platform->GetPrevPosition();
                pos.x += delta.x;
            }
        }
    }

    player_->SetPosition(pos);
    player_->SetVelocity(vel);
}
