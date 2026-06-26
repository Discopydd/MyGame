#pragma once
#include <vector>
#include "Particle.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include <memory>
class ParticleEmitter
{
public:

    ~ParticleEmitter();
    void Finalize();  
    void Initialize(MyEngine::Object3dCommon* objCommon, MyEngine::SpriteCommon* sprCommon);

    void Emit(int count,
        ParticleType type,
        const char* modelOrTexture,
        const MyEngine::Vector3& spawnPos,
        float minSpeed,
        float maxSpeed,
        float minLife,
        float maxLife, float horizontalBias = 0.0f, bool randomColor = false);

    void Update(float dt);
    void Draw3D();
    void Draw2D();
    void SetWindMode(bool enable)            { windMode_ = enable; }
    void SetUseOriginalSpriteSize(bool on)   { useOriginalSpriteSize_ = on; }
    void SetMaxParticles(size_t max) { maxParticles_ = max; }
    void SetSnowMode(bool enable)           { snowMode_ = enable; }

        // このエミッタの 3D 粒子をカメラの平行移動に追従させる
    void SetFollowCamera(bool follow) { followCamera_ = follow; }

    // 毎フレームのカメラ移動量を渡し、粒子位置の補正に使う
    void ApplyCameraMove(const MyEngine::Vector3& delta);

private:
    MyEngine::Object3dCommon* objCommon_ = nullptr;
    MyEngine::SpriteCommon* sprCommon_   = nullptr;

    std::vector<Particle> particles_;

    // ★ 所有権を持ち、unique_ptr でプールを管理する
    std::vector<std::unique_ptr<MyEngine::Object3d>> modelPool_;
    std::vector<std::unique_ptr<MyEngine::Sprite>>   spritePool_;

    bool windMode_ = false;
    bool useOriginalSpriteSize_ = false;
    size_t maxParticles_ = 200;

    bool snowMode_              = false;

    bool followCamera_   = false;

};
