#include "GameScene.h"
#include <numbers>
void GameScene::GenerateBlocks() {
    for (uint32_t y = 0; y < MapChipField::kNumBlockVirtical; y++) {
        for (uint32_t x = 0; x < MapChipField::kNumBlockHorizontal; x++) {
            // 获取当前格子类型
            MapChipType type = mapChipField_.GetMapChipTypeByIndex(x, y);

            // 如果是方块（kBlock），创建3D对象
            if (type == MapChipType::kBlock) {
                Object3d* block = new Object3d();
                block->Initialize(object3dCommon_);
                block->SetModel("cube/cube.obj");       // 使用方块模型
                block->SetCamera(camera_);
                // 设置方块位置（根据格子索引转换为世界坐标）
                Vector3 position = mapChipField_.GetMapChipPositionByIndex(x, y);
                block->SetTranslate(position);

                // 添加到方块列表
                mapBlocks_.push_back(block);
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
    camera_->SetTranslate({ 0, 0, -10 });
    object3dCommon_->SetDefaultCamera(camera_);

    ModelManager::GetInstants()->LoadModel("cube/cube.obj");

    mapChipField_.LoadMapChipCsv("Resources/map.csv");
    GenerateBlocks();
}

void GameScene::Update() {
    camera_->Update();
    imguiManager_->Begin();
    input_->Update();

    for (auto* block : mapBlocks_) {
        block->Update();
    }
    if (input_->TriggerKey(DIK_SPACE)) {
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
    spriteCommon_->CommonDraw();
    for (auto* sprite : sprites_) {
        sprite->Draw();
    }
    // ParticleManager::GetInstance()->Draw();
    imguiManager_->Draw();
    dxCommon_->End();
}

void GameScene::Finalize() {
    ParticleManager::GetInstance()->Finalize();
    TextureManager::GetInstance()->Finalize();
    ModelManager::GetInstants()->Finalize();
    imguiManager_->Finalize();

    delete camera_;
    delete spriteCommon_;
    delete object3dCommon_;
    for (auto* block : mapBlocks_) {
        delete block;
    }
    mapBlocks_.clear();
    delete imguiManager_;
    delete particleEmitter_;

    for (auto* sprite : sprites_) {
        delete sprite;
    }
}
