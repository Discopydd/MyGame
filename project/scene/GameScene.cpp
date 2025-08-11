#include "GameScene.h"
#include <numbers>

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
    box_ = new Object3d();
    box_->Initialize(object3dCommon_);
    box_->SetModel("cube/cube.obj");
    box_->SetCamera(camera_);
    box_->SetTranslate({ 0.0f, 0.0,0.0f });

}

void GameScene::Update() {
    camera_->Update();
    imguiManager_->Begin();
    input_->Update();
    rotation_.x -= 0.003f;
     box_->SetRotate({ 0.0f, rotation_.x, 0.0f });
    box_->Update();
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

    // ======= Material =======
    if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
        static bool enableLighting = box_->GetModel()->GetEnableLighting();
        if (ImGui::Checkbox("Enable Lighting", &enableLighting)) {
            box_->GetModel()->SetEnableLighting(enableLighting);
        }
    }
     // ======= Light =======
    if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
        static float lightColor[3] = { 1.0f, 1.0f, 1.0f };
        static float lightDirection[3] = { 0.0f, -1.0f, 0.0f };
        static float lightIntensity = 1.0f;

        if (ImGui::ColorEdit3("Light Color", lightColor)) {
            box_->GetDirectionalLightData()->color = { lightColor[0], lightColor[1], lightColor[2], 1.0f };
        }
        if (ImGui::DragFloat3("Light Direction", lightDirection, 0.01f, -1.0f, 1.0f)) {
            box_->GetDirectionalLightData()->direction =
                Math::Normalize(Vector3{ lightDirection[0], lightDirection[1], lightDirection[2] });
        }
        if (ImGui::DragFloat("Light Intensity", &lightIntensity, 0.01f, 0.0f, 5.0f)) {
            box_->GetDirectionalLightData()->intensity = lightIntensity;
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

    box_->Draw();
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
    delete box_;
    delete imguiManager_;
    delete particleEmitter_;

    for (auto* sprite : sprites_) {
        delete sprite;
    }
}
