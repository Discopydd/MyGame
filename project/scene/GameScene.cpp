#include "GameScene.h"
#include <numbers>
#include <LoadingScene.h>
#include "SceneManager.h"
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

    player_ = new Player();
    player_->Initialize(object3dCommon_, camera_);


    playerCamera_ = new PlayerCamera();
    playerCamera_->Initialize(camera_, player_, &mapChipField_);
    playerCamera_->SetOffset({ 0, 0.0f, -40.0f });
    playerCamera_->SetFollowSpeed(0.1f);
    playerCamera_->SetConstrainToMap(true);

    LoadMap("Resources/map/map.csv", {3,3,0});
    std::string textureFilePath[] = { "Resources/skill_icon.png", "Resources/gray.png" };

    skillSprite_ = new Sprite();
    skillSprite_->Initialize(spriteCommon_, textureFilePath[0]);
    skillSprite_->SetPosition({ 50.0f, 50.0f }); // 左上角
    skillSprite_->SetSize({ 32.0f, 32.0f });

    grayOverlaySprite_ = new Sprite();
    grayOverlaySprite_->Initialize(spriteCommon_, textureFilePath[1]);
    grayOverlaySprite_->SetPosition({ 50.0f, 50.0f });
    grayOverlaySprite_->SetSize({ 32.0f, 32.0f });

     isMapLoading_ = false;
    loadingTimer_ = 0.0f;
}

void GameScene::Update() {
    const float deltaTime = 1.0f / 60.0f;
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

            if (sceneManager_) sceneManager_->ClearOverlayScene();

            // 真正加载地图
            LoadMap("Resources/map/map.csv", { 3,3,0 });
        }
        return;
    }

    // 3️⃣ 传送门加载计时
    if (isPortalLoading_) {
        portalLoadingTimer_ += deltaTime;
        if (portalLoadingTimer_ >= LOADING_DURATION) {
            isPortalLoading_ = false;
            if (sceneManager_) sceneManager_->ClearOverlayScene();

            // 真正加载地图
            LoadMap(portalMapPath_, portalStartPos_);
        }
        return;
    }
    camera_->Update();
    imguiManager_->Begin();
    input_->Update();

    for (auto* block : mapBlocks_) {
        block->Update();
    }
    player_->Update(input_, mapChipField_);
    playerCamera_->Update();
    MapChipField::IndexSet playerIndex = mapChipField_.GetMapChipIndexByPosition(player_->GetPosition());

    bool onAnyPortal = false;
    for (auto& portal : portals_) {
        bool isOnPortal = (playerIndex.xIndex == portal.index.xIndex &&
            playerIndex.yIndex == portal.index.yIndex);
        if (isOnPortal && !wasOnPortal_) {
            StartLoadingMap(portal.targetMap, portal.targetStartPos, true);
        }
        if (isOnPortal) onAnyPortal = true;
    }
    wasOnPortal_ = onAnyPortal;
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
    if (input_->TriggerKey(DIK_P)) {
        SoundManager::GetInstance()->Play("fanfare", false, 1.0f);
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
    srvManager_->PreDraw();
    object3dCommon_->CommonDraw();

    for (auto* block : mapBlocks_) {
        block->Draw();
    }
     player_->Draw();
    spriteCommon_->CommonDraw();
    skillSprite_->Draw();
    if (!player_->CanDash()) {
        grayOverlaySprite_->Draw();
    }
    // ParticleManager::GetInstance()->Draw();
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
    for (auto* block : mapBlocks_) delete block;
    mapBlocks_.clear();

    mapChipField_.LoadMapChipCsv(mapPath);
    GenerateBlocks();

    // 设置玩家起点
    player_->SetPosition(startPos);

    // 相机同步
    camera_->SetTranslate(startPos + Vector3{ 0,0,-40 });
    playerCamera_->SetMapBounds(mapChipField_.GetMapMinPosition(), mapChipField_.GetMapMaxPosition());
    // 根据当前地图更新传送门列表
    portals_.clear();
    if (mapPath == "Resources/map/map.csv") {
        portals_.push_back({ {26,11}, "Resources/map/map2.csv", {3,3,0} });
    }
    else if (mapPath == "Resources/map/map2.csv") {
        portals_.push_back({ {24,8}, "Resources/map/map.csv", {3,3,0} });
    }
    wasOnPortal_ = false;

    player_->Update(input_, mapChipField_);
    for (auto* block : mapBlocks_) block->Update();
    playerCamera_->Update();
    camera_->Update();
}