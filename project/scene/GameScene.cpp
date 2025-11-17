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
static Vector3 ScreenToWorld(float screenX, float screenY, float ndcZ, Camera* camera)
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

     // ==== 右上角 Coin UI（3D 模型） ====
    coinUiObj_ = new Object3d();
    coinUiObj_->Initialize(object3dCommon_);
    coinUiObj_->SetModel("coin_ui/coin_ui.obj");
    coinUiObj_->SetCamera(camera_);
    coinUiObj_->SetScale({ 0.0025f, 0.0025f, 0.0025f });
    coinUiObj_->SetEnableLighting(true);
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
    // ==== Coin 数字 UI (Sprite) ====
    coinColonSprite_ = new Sprite();
    coinColonSprite_->Initialize(spriteCommon_, "Resources/numbers/colon.png");
    coinColonSprite_->SetSize({ 24.0f, 24.0f });

     // 先全部初始化成 0.png，之后在 UpdateCoinCountUI_ 里切换贴图
    for (int i = 0; i < 3; ++i) {
        coinDigitSprites_[i] = new Sprite();
        coinDigitSprites_[i]->Initialize(spriteCommon_, "Resources/numbers/0.png");
        coinDigitSprites_[i]->SetSize({ 24.0f, 24.0f });
    }

    isMapLoading_ = false;
    loadingTimer_ = 0.0f;

    portalHintSprite_ = new Sprite();
    portalHintSprite_->Initialize(spriteCommon_, "Resources/letterE.png"); // 你的提示图
    portalHintSprite_->SetPosition({ 0.0f, 0.0f }); // 初始位置，稍后动态更新
    portalHintSprite_->SetSize({ 32.0f, 32.0f });
    portalHintSprite_->SetVisible(false); // 默认隐藏

    // ===== 全屏黑幕由 GameScene 统一管理 =====
    fadeSprite_ = new Sprite();
    fadeSprite_->Initialize(spriteCommon_, "Resources/black.png");
    fadeSprite_->SetPosition({ 0, 0 });
    fadeSprite_->SetSize({ (float)WinApp::kClientWidth, (float)WinApp::kClientHeight });

    // ===== Intro 视觉元素 =====
    // 初次进入 GameScene：先黑(1.0)，加载完成后淡入
    fadeAlpha_ = 1.0f;
    fadeSprite_->SetColor({ 1,1,1,fadeAlpha_ });
    fadePhase_ = FadePhase::LoadingHold;  // 由你现有的加载流程推进到 FadingIn
    const float W = (float)WinApp::kClientWidth;
    const float H = (float)WinApp::kClientHeight;
    const float barH = H * 0.25f;
    // 电影黑边（两条黑色Sprite，从屏幕外滑入）
    letterboxTop_ = new Sprite();
    letterboxTop_->Initialize(spriteCommon_, "Resources/black.png");
    letterboxTop_->SetSize({ (float)WinApp::kClientWidth, (float)WinApp::kClientHeight * 0.14f });
    letterboxTop_->SetPosition({ 0.0f, -WinApp::kClientHeight * 0.14f }); // 起始在屏幕外
    letterboxTop_->SetColor({ 1,1,1,0 }); // 先透明以免突兀

    letterboxBottom_ = new Sprite();
    letterboxBottom_->Initialize(spriteCommon_, "Resources/black.png");
    letterboxBottom_->SetSize({ (float)WinApp::kClientWidth, (float)WinApp::kClientHeight * 0.14f });
    letterboxBottom_->SetPosition({ 0.0f, (float)WinApp::kClientHeight }); // 起始在屏幕外
    letterboxBottom_->SetColor({ 1,1,1,0 });

    // 暗角（可用一张灰色或黑色贴图近似；也可后续换真正vignette贴图）
    vignette_ = new Sprite();
    vignette_->Initialize(spriteCommon_, "Resources/gray.png");
    vignette_->SetSize({ (float)WinApp::kClientWidth, (float)WinApp::kClientHeight });
    vignette_->SetPosition({ 0.0f, 0.0f });
    vignette_->SetColor({ 1,1,1,0.0f }); // 逐步淡入

    // 开场标题（可替换成你的关卡名贴图）
    introTitle_ = new Sprite();
    introTitle_->Initialize(spriteCommon_, "Resources/GameTitle.png");
    introTitle_->SetPosition({ 0.0f, WinApp::kClientHeight * 0.15f });
    introTitle_->SetColor({ 1,1,1,0 }); // 先透明

    // Skip 提示
    skipHint_ = new Sprite();
    skipHint_->Initialize(spriteCommon_, "Resources/Start.png"); // 先复用Start.png字样
    skipHint_->SetPosition({ 0.0f, WinApp::kClientHeight * 0.42f });
    skipHint_->SetColor({ 1,1,1,0 });
    skipHint_->SetVisible(false);
    gameOverSprite_ = new Sprite();
    gameOverSprite_->Initialize(spriteCommon_, "Resources/GameOver.png"); // 请把图片放到 Resources/
    gameOverSprite_->SetSize(gameOverSize_);
    gameOverSprite_->SetVisible(false);

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
    coinUiLightTime_  += deltaTime;
    float hintBobOffset = std::sinf(hintBobTime_ * hintBobSpeed_) * hintBobAmplitude_;
    input_->Update();
    // —— 是否允许玩家操作（淡出/加载/淡入期间 & 开场演出期间都禁止）——
    const bool isFading = (fadePhase_ != FadePhase::None);
    const bool inIntro = (introStarted_ && introState_ != IntroState::Done);
    const bool canControl = !(isFading || inIntro);
    const bool inGameOver = (gameOverState_ != GameOverState::None && gameOverState_ != GameOverState::Done);
    // ===== Intro 驱动（在加载/淡出等早退之前执行，但不盖过Loading）=====
    if (fadePhase_ == FadePhase::None) {
        UpdateIntro_(deltaTime);
        // 在 Intro 未完成时屏蔽输入与地图交互（仅渲染）
        if (introState_ != IntroState::Done) {
            // 更新必要的可见元素
            if (letterboxTop_) letterboxTop_->Update();
            if (letterboxBottom_) letterboxBottom_->Update();
            if (vignette_) vignette_->Update();
            if (introTitle_) introTitle_->Update();
            if (skipHint_) skipHint_->Update();
        }
    }
    // ===== 画面淡入淡出状态机（优先执行）=====
    if (fadePhase_ == FadePhase::FadingOut) {
        // 渐黑
        fadeAlpha_ += fadeSpeed_;
        if (fadeAlpha_ > 1.0f) fadeAlpha_ = 1.0f;

        if (fadeSprite_) {
            // 确保黑幕可见并更新颜色
            fadeSprite_->SetVisible(true);
            fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, fadeAlpha_ });
            fadeSprite_->Update();
        }

        // 到达纯黑：本帧先不切场景，先停一帧纯黑（与 Title 同步）
        if (fadeAlpha_ >= 1.0f) {
            if (!reachedBlack_) {
                reachedBlack_ = true;
                blackHoldFrames_ = 1;   // 纯黑保留1帧（可调为2~3）
                return;                 // 本帧结束，保证能看到完整纯黑
            }
            if (blackHoldFrames_ > 0) {
                --blackHoldFrames_;     // 消耗纯黑保留帧
                return;
            }

            // 保留帧结束 → 叠加 Loading 叠层（仅一次）
            if (!overlayPushed_) {
                if (sceneManager_) {
                    sceneManager_->SetOverlayScene(new LoadingScene());
                }
                overlayPushed_ = true;
            }

            // 如为传送门触发，则在此刻真正开始加载（已经保证出现过完整纯黑）
            if (pendingPortalLoad_) {
                pendingPortalLoad_ = false;
                StartLoadingMap(pendingPortalMapPath_, pendingPortalStartPos_, true);
            }

            // 进入 LoadingHold 等待加载完成
            fadePhase_ = FadePhase::LoadingHold;
            return;
        }

        return; // 仍在变黑过程中
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
            fadePhase_ = FadePhase::FadingIn;
            if (playIntroOnThisMap_ && !introStarted_) {
                StartIntro_();             // 播放完整演出
                playIntroOnThisMap_ = false; // 播放完重置
            }
            if (player_) {
                player_->ResetForMapTransition(true);
            }
            if (input_) {
                input_->ResetAllKeys();
            }
        }
        if (fadeSprite_) fadeSprite_->Update();
        return;
    }

    // 3️⃣ 传送门加载计时
    if (isPortalLoading_) {
        portalLoadingTimer_ += deltaTime;
        if (portalLoadingTimer_ >= LOADING_DURATION) {
            isPortalLoading_ = false;
            

            // 真正加载地图
            LoadMap(portalMapPath_, portalStartPos_);
            if (sceneManager_) sceneManager_->ClearOverlayScene();
            fadePhase_ = FadePhase::FadingIn;
            if (!introStarted_) {
                StartIntro_();
            }
            if (player_) {
                player_->ResetForMapTransition(true);
            }
            if (input_) {
                input_->ResetAllKeys();
            }
        }
        if (fadeSprite_) fadeSprite_->Update();
        return;
    }
    imguiManager_->Begin();
    playerCamera_->Update();
    // 淡入淡出/加载/演出期间都不可操作
    player_->Update(canControl ? input_ : nullptr, mapChipField_);

    camera_->Update();
    // 若尚未进入GameOver，检测HP
if (gameOverState_ == GameOverState::None && player_->GetHP() <= 0.0f) {
    StartGameOver_();
}

// GameOver状态机推进（会在其中屏蔽玩家操作）
if (gameOverState_ != GameOverState::None && gameOverState_ != GameOverState::Done) {
    UpdateGameOver_(deltaTime);
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
        rot.y += 0.025f;                  // 旋转速度（弧度/帧），可调 0.02f~0.1f
        it.obj->SetRotate(rot);

        it.obj->Update();
    }
     for (auto& it : items_) {
        if (!it.obj) continue;

        // 让道具绕Y轴缓慢旋转
        Vector3 rot = it.obj->GetRotate();
        rot.y += 0.05f;                  // 旋转速度（弧度/帧）
        it.obj->SetRotate(rot);
        it.obj->Update();
    }

    // ==== Coin UI：右上角的 3D coin 模型 ====
     if (coinUiObj_) {
         const float uiPad = 20.0f;
         const float digitW = 24.0f;
         const float digitGap = 2.0f;

         // 与 UpdateCoinCountUI_ 使用同一套公式：预留 3 位数字空间
         float colonX = (float)WinApp::kClientWidth - uiPad - 3.0f * (digitW + digitGap);
         float colonY = uiPad + 10.0f;

         // coin 放在冒号左边一点
         float coinScreenX = colonX - 20.0f;   // 32 可以按你的模型大小再调
         float coinScreenY = colonY + 12.0f;   // 轻微下移，让居中一点

         Vector3 coinWorld = ScreenToWorld(coinScreenX, coinScreenY, hpNdcZ_, camera_);
         coinUiObj_->SetTranslate(coinWorld);
         // 基础亮度（越大整体越亮）
         const float baseI = 0.6f;
         // 闪烁幅度（越大变化越明显）
         const float ampI = 1.4f;
         // 闪烁速度（越大闪得越快）
         const float speedI = 6.0f;

         // sin 波从 [-1,1] 映射到 [0,1]
         float wave = (std::sinf(coinUiLightTime_ * speedI) + 1.0f) * 0.5f;
         float dirIntensity = baseI + ampI * wave;  // 大概 0.6 ~ 2.0 之间变动

         coinUiObj_->SetDirectionalLightIntensity(dirIntensity);
         coinUiObj_->Update();
     }

     if (spaceHint_.sprite) {
        Vector3 screen = WorldToScreen(spaceHint_.worldPos, camera_);
        spaceHint_.sprite->SetPosition({ screen.x, screen.y + hintBobOffset});
        spaceHint_.sprite->Update();
    }

     for (auto& h : upHints_) {
         if (!h.sprite) continue;
         Vector3 s = WorldToScreen(h.worldPos, camera_);
         h.sprite->SetPosition({ s.x, s.y + hintBobOffset});
         h.sprite->Update();
     }
     if (shiftHint_.sprite) {
         Vector3 screen = WorldToScreen(shiftHint_.worldPos, camera_);
         shiftHint_.sprite->SetPosition({ screen.x, screen.y + hintBobOffset});
         shiftHint_.sprite->Update();
     }

     if (sprintHint_.sprite) {
         Vector3 screen = WorldToScreen(sprintHint_.worldPos, camera_);
         sprintHint_.sprite->SetPosition({ screen.x, screen.y + hintBobOffset});
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
            coinCount_ = totalCoinCollected_;
            UpdateCoinCountUI_();
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
                if (portal.targetMap == "Resources/map/map2.csv" && itStage != hubStageByMap_.end()) {
                    int stageIndex = itStage->second;   // 这是第几关(0~3)
                    // 通关第 N 关 → 至少解锁到 N+1
                    if (hubProgress_ < stageIndex + 1) {
                        hubProgress_ = stageIndex + 1;
                        if (hubProgress_ >= 4) {
                            allStagesCleared_ = true;   // 所有关卡完成
                            // 在这里你可以做 GameClear 处理
                            // 例： if (sceneManager_) { sceneManager_->ChangeScene(new GameClearScene()); }
                        }
                    }
                }

                // ==== 现有的传送处理 ====
                pendingPortalMapPath_ = portal.targetMap;
                pendingPortalStartPos_ = portal.targetStartPos;
                pendingPortalLoad_ = true;

                fadeAlpha_ = 0.0f;
                reachedBlack_ = false;
                blackHoldFrames_ = 0;
                overlayPushed_ = false;
                // 开始淡出到全黑
                fadePhase_ = FadePhase::FadingOut;
                if (fadeSprite_) {
                    fadeSprite_->SetVisible(true);
                    fadeSprite_->SetColor({ 1,1,1,fadeAlpha_ });
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
    if (fadePhase_ == FadePhase::FadingIn) {
        fadeAlpha_ -= fadeSpeed_;
        if (fadeAlpha_ < 0.0f) {
            fadeAlpha_ = 0.0f;
            fadePhase_ = FadePhase::None; // 淡入完成
            // ★ 淡入完成→启动开场演出
            if (!introStarted_) {
                StartIntro_();
            }
        }
        if (fadeSprite_) {
            fadeSprite_->SetColor({ 1,1,1,fadeAlpha_ });
            fadeSprite_->Update();
        }
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

    // ================== 1) 3D 场景（地图、HP 3D条等） ==================
    srvManager_->PreDraw();
    object3dCommon_->CommonDraw();

    // 地图方块
    for (auto* block : mapBlocks_) {
        block->Draw();
    }
    for (auto& it : items_) {
        if (it.obj) it.obj->Draw();
    }
    // HP 3D 条段（如果你想永远最上，可以挪到玩家后面，这里先保持原样）
    for (int i = 0; i < hpVisibleCount_; ++i) {
        hpStrips_[i]->Draw();
    }

    // ================== 2) 中间层：交互提示 Sprite ==================
    spriteCommon_->CommonDraw();

    if (spaceHint_.sprite) {
        spaceHint_.sprite->Draw();
    }

    // 所有 Up
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


    dxCommon_->ClearDepthBuffer();
    // ================== 3) 前景 3D：玩家（盖住提示） ==================
    // 再次切回 3D PSO（如果需要的话）
    srvManager_->PreDraw();
    object3dCommon_->CommonDraw();
    player_->Draw();
    if (coinUiObj_) {
        coinUiObj_->Draw();
    }
    // ================== 4) 最前景 UI Sprite ==================
    spriteCommon_->CommonDraw();

    // 技能图标
    if (skillSprite_) {
        skillSprite_->Draw();
    }

    // 冲刺CD灰罩
    if (!player_->CanDash() && grayOverlaySprite_) {
        grayOverlaySprite_->Draw();
    }
     // Coin 数字 UI（冒号 + 数字）
    if (coinColonSprite_) {
        coinColonSprite_->Draw();
    }
    for (int i = 0; i < 3; ++i) {
        if (coinDigitSprites_[i]) {
            coinDigitSprites_[i]->Draw();
        }
    }
    // 传送门提示
    if (portalHintSprite_ && portalHintSprite_->IsVisible()) {
        portalHintSprite_->Draw();
    }

    // Intro / 黑边 / 暗角 / 标题 / Skip 提示
    DrawIntro_();

    // 黑幕淡入淡出
    if (fadeSprite_) {
        fadeSprite_->Draw();
    }

    // GameOver
    DrawGameOver_();

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
    if (fadeSprite_) { delete fadeSprite_; fadeSprite_ = nullptr; }
    delete letterboxTop_;    letterboxTop_ = nullptr;
    delete letterboxBottom_; letterboxBottom_ = nullptr;
    delete vignette_;        vignette_ = nullptr;
    delete introTitle_;      introTitle_ = nullptr;
    delete skipHint_;        skipHint_ = nullptr;
    for (auto* seg : hpStrips_) delete seg;
    hpStrips_.clear();
    if (gameOverSprite_) { delete gameOverSprite_; gameOverSprite_ = nullptr; }
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
    if (coinUiObj_) {
        delete coinUiObj_;
        coinUiObj_ = nullptr;
    }
    if (coinColonSprite_) {
        delete coinColonSprite_;
        coinColonSprite_ = nullptr;
    }
    for (int i = 0; i < 3; ++i) {
        if (coinDigitSprites_[i]) {
            delete coinDigitSprites_[i];
            coinDigitSprites_[i] = nullptr;
        }
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
    coinCount_     = totalCoinCollected_;
    lastCoinCount_ = -1;           // 强制刷新一次
    UpdateCoinCountUI_();
    // --- 判断是否播放演出 ---
    if (mapPath == "Resources/map/map.csv") {
        playIntroOnThisMap_ = true;
        // 重置 Intro 状态
        introStarted_ = false;
        introState_ = IntroState::None;
        introSkippable_ = true;
    }
    else {
        playIntroOnThisMap_ = false;
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
void GameScene::StartIntro_() {
    if (player_) {
        player_->ResetForMapTransition(true);  // 停止移动、跳跃、冲刺等
    }
    if (input_) input_->ResetAllKeys();
    introStarted_ = true;
    introState_ = IntroState::BarsIn;
    introT_ = 0.0f;
    introSkippable_ = true;
    // 以玩家当前位置为环绕中心
    camPivot_ = player_->GetPosition();

    // 提前把相机拉到远景位（不会突变，因为刚好淡入完成）
    //camera_->SetTranslate(camStartPos_ + camPivot_);

    // 初值
    camOrbitDeg_ = -25.0f;      // 从-25度侧面开始
    shakeTime_ = 0.0f;
    shakeAmp_  = 0.0f;

    // 让黑边立即可见
    if (letterboxTop_)    letterboxTop_->SetColor({1,1,1,1});
    if (letterboxBottom_) letterboxBottom_->SetColor({1,1,1,1});
    if (vignette_)        vignette_->SetColor({1,1,1,0});
    if (introTitle_)      introTitle_->SetColor({1,1,1,0});
    if (skipHint_)        { skipHint_->SetColor({1,1,1,0}); skipHint_->SetVisible(false); }
}

void GameScene::UpdateIntro_(float dt) {
    if (introState_ == IntroState::None || introState_ == IntroState::Done) return;

    // 跳过
    if (introSkippable_ && input_ && (input_->TriggerKey(DIK_SPACE) || input_->TriggerKey(DIK_RETURN) || input_->TriggerKey(DIK_E))) {
        introSkippable_ = false;
        introState_ = IntroState::BarsOut;
        introT_ = 0.0f;
        shakeTime_ = 0.0f;
        shakeAmp_ = 0.0f;
        return;
    }

    introT_ += dt;

    switch (introState_) {
    case IntroState::BarsIn: {
        // 黑边 0.6s 内滑入
        float d = (std::min)(introT_ / 0.175f, 1.0f);
        d = EaseOutCubic_(d);
        const float H = (float)WinApp::kClientHeight;
        const float barH = H * 0.25f;

        // 显式给出起点与终点（无需假设锚点）
        const float topStart = -barH;     // 顶部条从屏幕上方外侧进入
        const float topEnd = 0.0f;      // 进入后贴紧顶部（y=0）

        const float botStart = H;         // 底部条从屏幕下方外侧进入
        const float botEnd = H - barH;  // 进入后贴紧底部

        const float topY = topStart + (topEnd - topStart) * d;
        const float botY = botStart + (botEnd - botStart) * d;

        if (letterboxTop_)    letterboxTop_->SetPosition({ 0.0f, topY });
        if (letterboxTop_)    letterboxTop_->SetSize({ (float)WinApp::kClientWidth,  barH });
        if (letterboxBottom_) letterboxBottom_->SetPosition({ 0.0f, botY });
        if (letterboxBottom_) letterboxBottom_->SetSize({ (float)WinApp::kClientWidth,  barH });
        // 暗角淡入到 0.25
        if (vignette_) vignette_->SetColor({ 1,1,1, 0.25f * d });

        if (introT_ >= 0.35f) { introState_ = IntroState::OrbitZoom; introT_ = 0.0f; }
        break;
    }
    case IntroState::OrbitZoom: {
        // 1.8s 环绕+推进
        float d = (std::min)(introT_ / 0.5f, 1.0f);
        float e = EaseInOutSine_(d);


        // 标题在中后段淡入
        if (introTitle_) {
            float titleAlpha = std::clamp((d - 0.35f) / 0.35f, 0.0f, 1.0f);
            introTitle_->SetColor({ 1,1,1, titleAlpha });
        }

        // Skip 提示闪烁
        if (skipHint_) {
            skipHint_->SetVisible(true);
            float blink = (sinf(introT_ * 12.0f) * 0.5f + 0.5f) * 0.85f; // 0~0.85
            skipHint_->SetColor({ 1,1,1, blink });
        }

        if (introT_ >= 0.8f) { introState_ = IntroState::TitleShow; introT_ = 0.0f; }
        break;
    }
    case IntroState::TitleShow: {
        // 0.9s 标题停留，暗角稍加强到 0.35
        float d = (std::min)(introT_ / 0.25f, 1.0f);
        if (vignette_) vignette_->SetColor({1,1,1, 0.25f + 0.10f * d});

        // 轻微二段震动
        if (introT_ < 0.2f) { shakeTime_ = 0.2f; shakeAmp_ = 0.12f; }

        if (introT_ >= 0.25f) { introState_ = IntroState::BarsOut; introT_ = 0.0f; }
        break;
    }
    case IntroState::BarsOut: {
        float d = (std::min)(introT_ / 0.15f, 1.0f);
        d = EaseOutCubic_(d);

        const float H = (float)WinApp::kClientHeight;
        const float barH = H * 0.25f;

        // 顶部：从 on-screen(0) → off-screen(-barH)
        const float topStart = 0.0f;
        const float topEnd = -barH;

        // 底部：从 on-screen(H - barH) → off-screen(H)
        const float botStart = H - barH;
        const float botEnd = H;

        const float topY = topStart + (topEnd - topStart) * d;
        const float botY = botStart + (botEnd - botStart) * d;

        if (letterboxTop_)    letterboxTop_->SetPosition({ 0.0f, topY });
        if (letterboxTop_)    letterboxTop_->SetSize({ (float)WinApp::kClientWidth,  barH });
        if (letterboxBottom_) letterboxBottom_->SetPosition({ 0.0f, botY });
        if (letterboxBottom_) letterboxBottom_->SetSize({ (float)WinApp::kClientWidth,  barH });
        if (vignette_)   vignette_->SetColor({ 1,1,1, (1.0f - d) * 0.35f });
        if (introTitle_) introTitle_->SetColor({ 1,1,1, (1.0f - d) });
        if (skipHint_)   skipHint_->SetColor({ 1,1,1, (1.0f - d) });

        if (introT_ >= 0.5f) { introState_ = IntroState::Done; introT_ = 0.0f; }
        break;
    }
    default: break;
    }

    // 更新摇镜计时
    if (shakeTime_ > 0.0f) {
        shakeTime_ -= dt;
        if (shakeTime_ < 0.0f) shakeTime_ = 0.0f;
    }
}

void GameScene::DrawIntro_() {
    if (introState_ == IntroState::None || !spriteCommon_) return;

    spriteCommon_->CommonDraw();

     //标题与暗角
    if (vignette_)        vignette_->Draw();
    if (introTitle_)      introTitle_->Draw();
     // 黑边
    if (letterboxTop_)    letterboxTop_->Draw();
    if (letterboxBottom_) letterboxBottom_->Draw();
    // Skip 提示
    //if (skipHint_ && skipHint_->IsVisible()) skipHint_->Draw();
}

void GameScene::ApplyScreenShake_(Vector3& camPos) {
    if (shakeTime_ <= 0.0f || shakeAmp_ <= 0.0f) return;
    // 简单伪随机噪声（帧相干）
    float nx = sinf(introT_ * 47.0f) * 0.5f + sinf(introT_ * 91.0f) * 0.5f;
    float ny = cosf(introT_ * 37.0f) * 0.5f + cosf(introT_ * 73.0f) * 0.5f;
    camPos.x += nx * shakeAmp_;
    camPos.y += ny * shakeAmp_ * 0.6f;
}
void GameScene::StartGameOver_() {
    gameOverState_ = GameOverState::Dying;
    gameOverT_ = 0.0f;

    // 触发玩家死亡演出（微跳+倒立坠落）
    if (player_) { player_->StartDeathFall(); }

    // 关掉提示等UI
    if (portalHintSprite_) { portalHintSprite_->SetVisible(false); }
}

void GameScene::UpdateGameOver_(float dt) {
    gameOverT_ += dt;

    switch (gameOverState_) {
    case GameOverState::Dying: {
        // 等待玩家跌出屏幕下方或超时，再开始黑幕淡出
        Vector3 sp = WorldToScreen(player_->GetPosition(), camera_);
        const float H = (float)WinApp::kClientHeight;
        if (sp.y > H + 60.0f || gameOverT_ > 2.0f) {
            gameOverState_ = GameOverState::FadeOut;
            gameOverT_ = 0.0f;

            // 准备黑幕：从 0 开始淡到 1
            fadeAlpha_ = 0.0f;
            if (fadeSprite_) {
                fadeSprite_->SetVisible(true);
                fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, fadeAlpha_ });
                fadeSprite_->Update();
            }
        }
        break;
    }

    case GameOverState::FadeOut: {
        // 0.9s 淡出到全黑（你可以调时长）
        float d = (std::min)(1.0f, gameOverT_ / 0.9f);
        // easeOutCubic
        float e = 1.0f - (1.0f - d) * (1.0f - d) * (1.0f - d);
        fadeAlpha_ = e;

        if (fadeSprite_) {
            fadeSprite_->SetVisible(true);
            fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, fadeAlpha_ });
            fadeSprite_->Update();
        }

        if (d >= 1.0f) {
            gameOverState_ = GameOverState::BlackHold;
            gameOverT_ = 0.0f;
        }
        break;
    }

    case GameOverState::BlackHold: {
        // 保持全黑 0.2s（可调）
        if (fadeSprite_) {
            fadeSprite_->SetVisible(true);
            fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
            fadeSprite_->Update();
        }

        if (gameOverT_ > 0.2f) {
            gameOverState_ = GameOverState::ShowTitle;
            gameOverT_ = 0.0f;

            // 计算 GameOver 精灵的起止位置（从上方滑到屏幕中央）
            const float W = (float)WinApp::kClientWidth;
            const float H = (float)WinApp::kClientHeight;
            gameOverEndPos_ = { (W - gameOverSize_.x) * 0.5f, (H - gameOverSize_.y) * 0.5f };
            gameOverStartPos_ = { gameOverEndPos_.x, -gameOverSize_.y - 40.0f };
            gameOverPos_ = gameOverStartPos_;
            if (gameOverSprite_) { gameOverSprite_->SetVisible(true); }
        }
        break;
    }

    case GameOverState::ShowTitle: {
        // 标题从上滑入：easeOutBack
        float d = (std::min)(1.0f, gameOverT_ / gameOverSlideTime_);
        float s = 1.70158f, p = d - 1.0f;
        float e = 1.0f + (p * p * ((s + 1.0f) * p + s));

        gameOverPos_.x = gameOverStartPos_.x + (gameOverEndPos_.x - gameOverStartPos_.x) * e;
        gameOverPos_.y = gameOverStartPos_.y + (gameOverEndPos_.y - gameOverStartPos_.y) * e;

        if (gameOverSprite_) {
            gameOverSprite_->SetPosition({ gameOverPos_.x, gameOverPos_.y });
            gameOverSprite_->Update();
        }

        // 期间保持全黑
        if (fadeSprite_) {
            fadeSprite_->SetVisible(true);
            fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
            fadeSprite_->Update();
        }

        if (d >= 1.0f) {
            gameOverState_ = GameOverState::Done;
            gameOverT_ = 0.0f;
        }
        break;
    }

    case GameOverState::Done:
    default:
        break;
    }
}

void GameScene::DrawGameOver_() {
    if (!spriteCommon_) { return; }
    spriteCommon_->CommonDraw();

    if (gameOverSprite_ && gameOverSprite_->IsVisible()) {
        gameOverSprite_->Draw();
    }
}

void GameScene::UpdateCoinCountUI_()
{
    if (!coinColonSprite_) {
        return;
    }

    const float uiPad    = 20.0f;
    const float digitW   = 24.0f;
    const float digitH   = 24.0f;
    const float digitGap = 2.0f;

    // 预留三位数字的空间，靠近右上角
    float colonX = (float)WinApp::kClientWidth  - uiPad - 3.0f * (digitW + digitGap);
    float colonY = uiPad + 10.0f;

    // 冒号位置
    coinColonSprite_->SetPosition({ colonX, colonY });
    coinColonSprite_->SetVisible(true);
    coinColonSprite_->Update();
    // 规范化到 0~999
    int c = coinCount_;
    if (c < 0)   c = 0;
    if (c > 999) c = 999;

    // 拆成三位
    int d0 = c / 100;
    int d1 = (c / 10) % 10;
    int d2 = c % 10;

    int digits[3] = { d0, d1, d2 };
    int numDigits = (c >= 100) ? 3 : (c >= 10 ? 2 : 1);

    // 先全部隐藏
    for (int i = 0; i < 3; ++i) {
        if (coinDigitSprites_[i]) {
            coinDigitSprites_[i]->SetVisible(false);
        }
    }

    // 数字贴图路径（按你资源实际情况修改）
    const char* digitTex[10] = {
        "Resources/numbers/0.png","Resources/numbers/1.png","Resources/numbers/2.png","Resources/numbers/3.png","Resources/numbers/4.png",
        "Resources/numbers/5.png","Resources/numbers/6.png","Resources/numbers/7.png","Resources/numbers/8.png","Resources/numbers/9.png",
    };

    // 让数字靠右（最低位靠近右侧）
    float x = colonX + digitW + digitGap;
    float y = colonY;

    int startIndex = 3 - numDigits; // 例如 numDigits=1 -> 从 d2 开始

    int spriteIdx = 0;
    for (int i = startIndex; i < 3; ++i) {
        int d = digits[i];
        if (!coinDigitSprites_[spriteIdx]) {
            ++spriteIdx;
            continue;
        }

        // 重新初始化，切换到对应数字贴图
        coinDigitSprites_[spriteIdx]->Initialize(spriteCommon_, digitTex[d]);
        coinDigitSprites_[spriteIdx]->SetSize({ digitW, digitH });
        coinDigitSprites_[spriteIdx]->SetPosition({ x, y });
        coinDigitSprites_[spriteIdx]->SetVisible(true);
        coinDigitSprites_[spriteIdx]->Update();

        x += digitW + digitGap;
        ++spriteIdx;
    }

    lastCoinCount_ = c;
}