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

    ModelManager::GetInstants()->LoadModel("plane.obj");
    ModelManager::GetInstants()->LoadModel("axis.obj");
    ModelManager::GetInstants()->LoadModel("terrain.obj");
    object3d_ = new Object3d();
    object3d_->Initialize(object3dCommon_);
    object3d_->SetModel("plane.obj");
    object3d_->SetCamera(camera_);
    terrain_ = new Object3d();
    terrain_->Initialize(object3dCommon_);
    terrain_->SetModel("terrain.obj");
    terrain_->SetCamera(camera_);
    terrain_->SetTranslate({ 0.0f, -1.0f,5.0f });
    std::string textureFilePath[] = { "Resources/monsterBall.png", "Resources/uvChecker.png" };
    for (uint32_t i = 0; i < 1; ++i) {
        Sprite* sprite = new Sprite();
        sprite->Initialize(spriteCommon_, textureFilePath[1]);
        sprite->SetPosition({ 200.0f * i, 0.0f });
        sprite->SetAnchorPoint({ 0.0f, 0.0f });
        sprite->SetIsFlipY(false);
        sprites_.push_back(sprite);
    }

    ParticleManager::GetInstance()->Initialize(dxCommon_, srvManager_, camera_);
    ParticleManager::GetInstance()->CreateparticleGroup("particle", "resources/circle.png");
    particleEmitter_ = new ParticleEmitter();
    particleEmitter_->Initialize("particle");
}

void GameScene::Update() {
    camera_->Update();
    imguiManager_->Begin();
    input_->Update();

    for (auto* sprite : sprites_) {
        sprite->Update();
    }

    particleEmitter_->Update();
    ParticleManager::GetInstance()->Update();


    object3d_->SetRotate({ 0.0f, rotation_.x, 0.0f });
    object3d_->Update();
    terrain_->Update();
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
        static bool enableLighting = object3d_->GetModel()->GetEnableLighting();
        if (ImGui::Checkbox("Enable Lighting", &enableLighting)) {
            object3d_->GetModel()->SetEnableLighting(enableLighting);
        }
    }

    // ======= Light =======
    if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
        static float lightColor[3] = { 1.0f, 1.0f, 1.0f };
        static float lightDirection[3] = { 0.0f, -1.0f, 0.0f };
        static float lightIntensity = 1.0f;

        if (ImGui::ColorEdit3("Light Color", lightColor)) {
            object3d_->GetDirectionalLightData()->color = { lightColor[0], lightColor[1], lightColor[2], 1.0f };
        }
        if (ImGui::DragFloat3("Light Direction", lightDirection, 0.01f, -1.0f, 1.0f)) {
            object3d_->GetDirectionalLightData()->direction =
                Math::Normalize(Vector3{ lightDirection[0], lightDirection[1], lightDirection[2] });
        }
        if (ImGui::DragFloat("Light Intensity", &lightIntensity, 0.01f, 0.0f, 5.0f)) {
            object3d_->GetDirectionalLightData()->intensity = lightIntensity;
        }
        // ======= Point Light =======
        if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            PointLight* point = object3d_->GetPointLightData();

            float pointPos[3] = { point->position.x, point->position.y, point->position.z };
            float pointColor[3] = { point->color.x, point->color.y, point->color.z };

            if (ImGui::DragFloat3("Point Light Position", pointPos, 0.1f)) {
                point->position = { pointPos[0], pointPos[1], pointPos[2] };
            }
            if (ImGui::ColorEdit3("Point Light Color", pointColor)) {
                point->color = { pointColor[0], pointColor[1], pointColor[2], 1.0f };
            }
            ImGui::DragFloat("Point Light Intensity", &point->intensity, 0.01f, 0.0f, 10.0f);
        }

        // ======= Spot Light =======
        if (ImGui::CollapsingHeader("Spot Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            SpotLight* spot = object3d_->GetSpotLightData();

            float spotPos[3] = { spot->position.x, spot->position.y, spot->position.z };
            float spotDir[3] = { spot->direction.x, spot->direction.y, spot->direction.z };
            float spotColor[3] = { spot->color.x, spot->color.y, spot->color.z };

            if (ImGui::DragFloat3("Spot Light Position", spotPos, 0.1f)) {
                spot->position = { spotPos[0], spotPos[1], spotPos[2] };
            }
            if (ImGui::DragFloat3("Spot Light Direction", spotDir, 0.1f)) {
                spot->direction = Math::Normalize(Vector3{ spotDir[0], spotDir[1], spotDir[2] });
            }
            if (ImGui::ColorEdit3("Spot Light Color", spotColor)) {
                spot->color = { spotColor[0], spotColor[1], spotColor[2], 1.0f };
            }

            ImGui::DragFloat("Spot Light Intensity", &spot->intensity, 0.01f, 0.0f, 10.0f);
            ImGui::DragFloat("Spot Light Distance", &spot->distance, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Spot Light Decay", &spot->decay, 0.01f, 0.0f, 10.0f);

            float angleDegrees = std::acos(spot->cosAngle) * (180.0f / std::numbers::pi_v<float>);
            if (ImGui::SliderFloat("Spot Light Angle (deg)", &angleDegrees, 1.0f, 90.0f)) {
                spot->cosAngle = std::cos(angleDegrees * std::numbers::pi_v<float> / 180.0f);
            }
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
    object3d_->Draw();
    terrain_->Draw();
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
    delete object3d_;
    delete terrain_;
    delete imguiManager_;
    delete particleEmitter_;

    for (auto* sprite : sprites_) {
        delete sprite;
    }
}
