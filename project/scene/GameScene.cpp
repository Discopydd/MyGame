#include "GameScene.h"
#include <numbers>
#include <scene/LoadingScene.h>
#include "SceneManager.h"
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
                // === 新增：道具 ===
                const uint32_t key = PackIdx(x, y);
                auto found = pickedItems_.find(currentMapPath_);
                bool alreadyPicked = (found != pickedItems_.end() && found->second.count(key));

                if (alreadyPicked) {
                    // ★ 该格已拾取过 → 本次不生成可见体
                    continue;
                }

                // 生成一个道具可见体（示例用 cube；你也可换成 item.obj）
                Object3d* item = new Object3d();
                item->Initialize(object3dCommon_);
                item->SetModel("coin/coin.obj");         // TODO: 替换为你的 item 模型
                item->SetCamera(camera_);
                // 让道具略浮起一点更显眼（可选）
                Vector3 itemPos = position;  // ← 用 position，而不是 pos
                itemPos.y += 0.4f;
                item->SetTranslate(itemPos);
                item->SetEnableLighting(true);
                item->SetDirectionalLightIntensity(2.0f);
                item->SetPointLightIntensity(2.0f);
                items_.push_back({ x, y, item });
            }
        }
    }
}

void GameScene::Initialize() {
    winApp_ = WinApp::GetInstance();
    dxCommon_ = DirectXCommon::GetInstance();
    input_ = Input::GetInstance();
    srvManager_ = SrvManager::GetInstance();

    spriteCommon_ = new SpriteCommon();
    spriteCommon_->Initialize(dxCommon_);


    TextureManager::GetInstance()->Initialize(dxCommon_, srvManager_);

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
    hpStrips_.reserve(hpSegments_);
    for (int i = 0; i < hpSegments_; ++i) {
        auto* seg = new Object3d();
        seg->Initialize(object3dCommon_);
        seg->SetModel("strip/strip.obj");
        seg->SetCamera(camera_);
        // 体感缩放：根据你的 strip 模型大小微调
        seg->SetScale({ 0.001f, 0.001f, 0.001f });
        hpStrips_.push_back(seg);
    }
    player_ = new Player();
    player_->Initialize(object3dCommon_, camera_);


    playerCamera_ = new PlayerCamera();
    playerCamera_->Initialize(camera_, player_, &mapChipField_);
    playerCamera_->SetOffset({ 0, 0.0f, -40.0f });
    playerCamera_->SetFollowSpeed(0.1f);
    playerCamera_->SetConstrainToMap(true);

    //LoadMap("Resources/map/map.csv", { 3,3,0 });
    std::string textureFilePath[] = { "Resources/skill_icon.png", "Resources/gray.png" };

    skillSprite_ = new Sprite();
    skillSprite_->Initialize(spriteCommon_, textureFilePath[0]);
    skillSprite_->SetPosition({ 40.0f, 80.0f }); // 左上角
    skillSprite_->SetSize({ 32.0f, 32.0f });

    grayOverlaySprite_ = new Sprite();
    grayOverlaySprite_->Initialize(spriteCommon_, textureFilePath[1]);
    grayOverlaySprite_->SetPosition({ 40.0f, 80.0f });
    grayOverlaySprite_->SetSize({ 32.0f, 32.0f });

     // === Coin UI 管理器 ===
    coinUI_ = new CoinUIManager();
    coinUI_->Initialize(spriteCommon_, object3dCommon_, camera_, hpNdcZ_);

    // 一开始显示当前总金币数（通常是 0）
    coinUI_->SetTotalCoin(totalCoinCollected_);


    isMapLoading_ = false;
    loadingTimer_ = 0.0f;

    portalHintSprite_ = new Sprite();
    portalHintSprite_->Initialize(spriteCommon_, "Resources/letterE.png"); // 你的提示图
    portalHintSprite_->SetPosition({ 0.0f, 0.0f }); // 初始位置，稍后动态更新
    portalHintSprite_->SetSize({ 32.0f, 32.0f });
    portalHintSprite_->SetVisible(false); // 默认隐藏

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

}

void GameScene::Update() {
    const float deltaTime = 1.0f / 60.0f;
    hintBobTime_ += deltaTime;
    if (coinUI_) {
        coinUI_->Update(deltaTime);
    }
    float hintBobOffset = std::sinf(hintBobTime_ * hintBobSpeed_) * hintBobAmplitude_;
    input_->Update();
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
                    sceneManager_->SetNextScene(new TitleScene());
                }
                return;
            }

            // 2-3) 推入 Loading 叠加场景（只推一次）
            if (!fade_->OverlayPushed()) {
                if (!pendingGameClear_ && !returnToTitle_) {
                    if (sceneManager_) {
                        sceneManager_->SetOverlayScene(new LoadingScene());
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
    // 淡入淡出/加载/演出期间都不可操作
    player_->Update(canControl ? input_ : nullptr, mapChipField_);

    camera_->Update();
    // 若尚未进入GameOver，检测HP
    if (player_->GetHP() <= 0.0f) {
        if (gameOver_ && !gameOver_->IsPlaying()) {
            gameOver_->Start();
        }
    }

     // GameOver 状态机推进（交给管理器）
    if (gameOver_) {
        gameOver_->Update(deltaTime);
    }
    // GameClear 状态机推进
    if (gameClear_) {
        gameClear_->Update(deltaTime);
    }
        // === 在 GameClear 演出期间按 Space → 回标题 ===
    if (gameClear_ && gameClear_->IsPlaying() && input_ && input_->TriggerKey(DIK_SPACE)) {

        if (sceneManager_) {
            sceneManager_->ClearOverlayScene();
            sceneManager_->SetNextScene(new TitleScene());
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


    float hpRatio = player_ ? player_->GetHpRatio() : 1.0f;
    hpVisibleCount_ = (int)std::ceil(hpRatio * hpSegments_);
    hpVisibleCount_ = (std::max)(0, (std::min)(hpVisibleCount_, hpSegments_));


    // ===== 屏幕基点（左上向内偏移） =====
    const float pad = 16.0f;
    float baseX = pad + hpInsetX_;
    float baseY = pad + hpInsetY_;

    // ===== 给每一段计算屏幕坐标 → 反投到世界 → Update =====
    for (int i = 0; i < hpSegments_; ++i) {
        float sx = baseX + i * (hpSegPixelW_ + hpGapPixel_);
        float sy = baseY;
        Vector3 world = ScreenToWorld(sx, sy, hpNdcZ_, camera_);
        hpStrips_[i]->SetTranslate(world);

        // 可选：轻微缩放衰减/高亮首段等，这里先保持一致
        // 可选：若你的 Object3d 支持朝向/关闭剔除，可在此设置
        hpStrips_[i]->Update();
    }
    for (auto* block : mapBlocks_) {
        block->Update();
    }
    for (auto& it : items_) {
        if (!it.obj) continue;

        // 让道具绕Y轴缓慢旋转
        Vector3 rot = it.obj->GetRotate();
        rot.y += 0.05f;                  // 旋转速度（弧度/帧）
        it.obj->SetRotate(rot);
        it.obj->Update();
    }

    if (spaceHint_.sprite) {
        Vector3 screen = WorldToScreen(spaceHint_.worldPos, camera_);
        spaceHint_.sprite->SetPosition({ screen.x, screen.y + hintBobOffset });
        spaceHint_.sprite->Update();
    }

    for (auto& h : upHints_) {
        if (!h.sprite) continue;
        Vector3 s = WorldToScreen(h.worldPos, camera_);
        h.sprite->SetPosition({ s.x, s.y + hintBobOffset });
        h.sprite->Update();
    }
    if (shiftHint_.sprite) {
        Vector3 screen = WorldToScreen(shiftHint_.worldPos, camera_);
        shiftHint_.sprite->SetPosition({ screen.x, screen.y + hintBobOffset });
        shiftHint_.sprite->Update();
    }

    if (sprintHint_.sprite) {
        Vector3 screen = WorldToScreen(sprintHint_.worldPos, camera_);
        sprintHint_.sprite->SetPosition({ screen.x, screen.y + hintBobOffset });
        sprintHint_.sprite->Update();
    }

    MapChipField::IndexSet playerIndex = mapChipField_.GetMapChipIndexByPosition(player_->GetPosition());
    if (mapChipField_.GetMapChipTypeByIndex(playerIndex.xIndex, playerIndex.yIndex) == MapChipType::kItem) {

        const uint32_t packed = PackIdx(playerIndex.xIndex, playerIndex.yIndex);

        // 若从未登记过：登记“已拾取”，删除可见体 & 给予奖励
        if (!pickedItems_[currentMapPath_].count(packed)) {
            pickedItems_[currentMapPath_].insert(packed);

            // 从 items_ 里找到同格子对象，删除
            for (auto it = items_.begin(); it != items_.end(); ++it) {
                if (it->x == playerIndex.xIndex && it->y == playerIndex.yIndex) {
                    if (it->obj) delete it->obj;
                    items_.erase(it);
                    break;
                }
            }
            ++totalCoinCollected_;
            if (coinUI_) {
                coinUI_->SetTotalCoin(totalCoinCollected_);
            }
            // 奖励示例：回血+音效（按你的需要换掉）
            // SoundManager::GetInstance()->Play("item_pick", false, 1.0f);
            if (player_) {
                // 假设你想加点血
                // player_->Heal(10.0f);
            }
        }
    }
    bool onAnyPortal = false;
    for (auto& portal : portals_) {
        bool isOnPortal = (playerIndex.xIndex == portal.index.xIndex &&
            playerIndex.yIndex == portal.index.yIndex);
        if (isOnPortal) {
            // 如果按下E键，才触发传送
            if (canControl && input_->TriggerKey(DIK_E)) {

                // ==== 如果是从子关卡回到 Hub(map2)，更新解锁进度 ====
                auto itStage = hubStageByMap_.find(currentMapPath_);
                bool triggerGameClear = false;
                if (currentMapPath_ == "Resources/map/map.csv") {
                    triggerGameClear = true;
                }
                if (portal.targetMap == "Resources/map/map2.csv" && itStage != hubStageByMap_.end()) {
                    int stageIndex = itStage->second;   // 这是第几关(0~3)
                    // 通关第 N 关 → 至少解锁到 N+1
                    if (hubProgress_ < stageIndex + 1) {
                        hubProgress_ = stageIndex + 1;
                        if (hubProgress_ >= 4) {
                            allStagesCleared_ = true;   // 所有关卡完成
                            // 在这里你可以做 GameClear 处理
                            // 例： if (sceneManager_) { sceneManager_->ChangeScene(new GameClearScene()); }
                            triggerGameClear = true;
                        }
                    }
                }
                if (triggerGameClear) {
                    // 通关：不再直接开始 GameClear，而是：
                    // 1) 先黑幕淡出到全黑
                    // 2) 在 FadingOut 逻辑里 pendingGameClear_ 分支中启动 GameClear
                    if (!pendingGameClear_ && gameClear_ && !gameClear_->IsPlaying() && fade_) {
                        pendingGameClear_ = true;

                        fade_->SetAlpha(0.0f);
                        fade_->SetReachedBlack(false);
                        fade_->SetBlackHoldFrames(0);
                        fade_->SetOverlayPushed(false);

                        // 开始淡出到全黑
                        fade_->SetPhase(FadePhase::FadingOut);
                        if (Sprite* s = fade_->GetSprite()) {
                            s->SetVisible(true);
                        }
                    }
                }
                else {
                    // ==== 现有的传送处理 ====
                    pendingPortalMapPath_ = portal.targetMap;
                    pendingPortalStartPos_ = portal.targetStartPos;
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
            onAnyPortal = true;
            break;
        }
    }
    wasOnPortal_ = onAnyPortal;
    if (portalHintSprite_) {
        if (onAnyPortal && canControl) {
            portalHintSprite_->SetVisible(true);
            Vector3 playerPos = player_->GetPosition();
            playerPos.x -= 0.25f;
            playerPos.y += 2.0f; // 玩家头顶偏移
            Vector3 screenPos = WorldToScreen(playerPos, camera_);
            portalHintSprite_->SetPosition({ screenPos.x, screenPos.y });
        }
        else {
            portalHintSprite_->SetVisible(false);
        }
    }
    if (!nextMapToLoad_.empty()) {
        LoadMap(nextMapToLoad_, nextMapStartPos_);
        nextMapToLoad_.clear();
    }
    float cooldownRatio = 0.0f;
    if (!player_->CanDash()) {
        cooldownRatio = player_->GetDashCooldown() / player_->GetDashCooldownDuration();
        cooldownRatio = (std::max)(0.0f, (std::min)(cooldownRatio, 1.0f)); // 限制在0~1
    }

    if (cooldownRatio > 0.0f) {
        float fullHeight = 32.0f;
        float visibleHeight = fullHeight * cooldownRatio;
        grayOverlaySprite_->SetTextureLeftTop({ 0.0f, 0.0f });
        grayOverlaySprite_->SetTextureSize({ 32.0f, visibleHeight });
        grayOverlaySprite_->SetSize({ 32.0f, visibleHeight });
    }
    skillSprite_->Update();
    grayOverlaySprite_->Update();
    portalHintSprite_->Update();
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
        case MapChipType::kBlock: typeName = "Block"; break;
        case MapChipType::kPortal: typeName = "Portal"; break;
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

    // ================== 1) 3D 场景（地图、HP 3D条等） ==================
    srvManager_->PreDraw();
    object3dCommon_->CommonDraw();

    // 地图方块
    for (auto* block : mapBlocks_) {
        block->Draw();
    }
    // 道具
    for (auto& it : items_) {
        if (it.obj) {
            it.obj->Draw();
        }
    }
    // HP 3D 条段
    for (int i = 0; i < hpVisibleCount_; ++i) {
        hpStrips_[i]->Draw();
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
        if (spaceHint_.sprite) {
            spaceHint_.sprite->Draw();
        }

        // 所有 Up 提示
        for (auto& h : upHints_) {
            if (h.sprite) {
                h.sprite->Draw();
            }
        }
        if (shiftHint_.sprite) {
            shiftHint_.sprite->Draw();
        }
        if (sprintHint_.sprite) {
            sprintHint_.sprite->Draw();
        }
        // 技能图标
        if (skillSprite_) {
            skillSprite_->Draw();
        }

        // 冲刺CD灰罩
        if (!player_->CanDash() && grayOverlaySprite_) {
            grayOverlaySprite_->Draw();
        }

        // Coin 数字 UI（冒号 + 数字）
        if (coinUI_) {
            coinUI_->Draw2D();
        }

        // 传送门提示
        if (portalHintSprite_ && portalHintSprite_->IsVisible()) {
            portalHintSprite_->Draw();
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

    // ImGui（debug UI）
    imguiManager_->Draw();

    dxCommon_->End();
}

void GameScene::Finalize() {
    SoundManager::GetInstance()->Finalize();
    ParticleManager::GetInstance()->Finalize();
    TextureManager::GetInstance()->Finalize();
    ModelManager::GetInstants()->Finalize();
    imguiManager_->Finalize();

    delete camera_;
    delete playerCamera_;
    delete spriteCommon_;
    delete object3dCommon_;
    for (auto* block : mapBlocks_) {
        delete block;
    }
    mapBlocks_.clear();
    delete player_;
    delete imguiManager_;
    delete skillSprite_;
    delete grayOverlaySprite_;
    delete portalHintSprite_;
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

    for (auto* seg : hpStrips_) delete seg;
    hpStrips_.clear();
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
    for (auto& it : items_) {
        if (it.obj) { delete it.obj; it.obj = nullptr; }
    }
    items_.clear();
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

}

void GameScene::StartLoadingMap(const std::string& mapPath, const Vector3& startPos, bool isPortal = false) {
    if (sceneManager_) {
        LoadingScene* loadingScene = new LoadingScene();
        sceneManager_->SetOverlayScene(loadingScene);
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
    for (auto& it : items_) {
        if (it.obj) { delete it.obj; }
    }
    items_.clear();

    for (auto* block : mapBlocks_) delete block;
    mapBlocks_.clear();

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
            h.sprite->Initialize(spriteCommon_, "Resources/up.png");
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
    // 相机同步
    camera_->SetTranslate(startPos + Vector3{ 0,0,-40 });
    playerCamera_->SetMapBounds(mapChipField_.GetMapMinPosition(), mapChipField_.GetMapMaxPosition());
    // 根据当前地图更新传送门列表
    portals_.clear();

    // ========== map1（起始地图） ==========
    if (mapPath == "Resources/map/map.csv") {
        // map1 → 中心地图 map2
        portals_.push_back({
            {26,11},                           // 当前 map1 里的格子
            "Resources/map/map2.csv",          // 目标地图（Hub）
            mapChipField_.GetMapChipPositionByIndex(2,1) // 在 Hub 中的出生格 (2,1) = CSV(27,2)
            });
    }

    // ========== map2（中心地图 / Hub） ==========
    else if (mapPath == "Resources/map/map2.csv") {
        // 恒常存在：Hub → 返回 map1 的门
        portals_.push_back({
            {2,1},                             // Hub 中的 (2,1)（就是你现在的返回门）
            "Resources/map/map.csv",
            mapChipField_.GetMapChipPositionByIndex(26,11) // 回 map1 的落点
            });

        // ------- 解锁顺序的 4 个关卡入口 -------
        // 约定：所有关卡里玩家出生都在 (2,1)，你之后在关卡 csv 里对应摆好

        // ① CSV (23,11) → index (11,5)：第一关，一开始就解锁（hubProgress_ >= 0）
        if (hubProgress_ >= 0) {
            portals_.push_back({
                {11,5},                        // Hub 上的门位置
                "Resources/map/map3.csv",      // 第1关
                mapChipField_.GetMapChipPositionByIndex(2,1) // 在 map3 中出生位置
                });
        }

        // ② CSV (23,14) → index (14,5)：第二关，需要先通关第一关（hubProgress_ >= 1）
        if (hubProgress_ >= 1) {
            portals_.push_back({
                {14,5},
                "Resources/map/map4.csv",      // 第2关
                mapChipField_.GetMapChipPositionByIndex(2,1)
                });
        }

        // ③ CSV (27,23) → index (23,1)：第三关，需要先通关第二关（hubProgress_ >= 2）
        if (hubProgress_ >= 2) {
            portals_.push_back({
                {23,1},
                "Resources/map/map5.csv",      // 第3关
                mapChipField_.GetMapChipPositionByIndex(2,1)
                });
        }

        // ④ CSV (14,12)/(14,13) → index (12,14)/(13,14)：最终关入口，需要先通关第三关（hubProgress_ >= 3）
        if (hubProgress_ >= 3) {
            Vector3 finalStart = mapChipField_.GetMapChipPositionByIndex(2, 1);
            portals_.push_back({
                {12,14},                       // 左门
                "Resources/map/map6.csv",      // 最终关
                finalStart
                });
            portals_.push_back({
                {13,14},                       // 右门
                "Resources/map/map6.csv",
                finalStart
                });
        }
    }

    // ========== 各子关卡内部：返回 Hub ==========
    else {
        // 这里假设你在每张关卡地图里也放了一个 '2' 当出口，
        // 把 {x,y} 换成那张图里出口的索引即可

        if (mapPath == "Resources/map/map3.csv") {
            portals_.push_back({
                {2,1},                         // TODO: 改成 map3 里出口的格子 index
                "Resources/map/map2.csv",      // 回到 Hub
                mapChipField_.GetMapChipPositionByIndex(11,5) // Hub 中出生也放在 (2,1)
                });
        }
        else if (mapPath == "Resources/map/map4.csv") {
            portals_.push_back({
                {2,1},                         // TODO
                "Resources/map/map2.csv",
                mapChipField_.GetMapChipPositionByIndex(14,5)
                });
        }
        else if (mapPath == "Resources/map/map5.csv") {
            portals_.push_back({
                {2,1},                         // TODO
                "Resources/map/map2.csv",
                mapChipField_.GetMapChipPositionByIndex(23,1)
                });
        }
        else if (mapPath == "Resources/map/map6.csv") {
            portals_.push_back({
                {2,1},                         // TODO（如果最终关也要回 Hub 的话）
                "Resources/map/map2.csv",
                mapChipField_.GetMapChipPositionByIndex(12,14)
                });
        }
    }

    wasOnPortal_ = false;

    player_->Update(input_, mapChipField_);
    for (auto* block : mapBlocks_) block->Update();
    playerCamera_->Update();
    camera_->Update();
}