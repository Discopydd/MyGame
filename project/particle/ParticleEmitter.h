#pragma once
#include <vector>
#include "Particle.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "SpriteCommon.h"
#include "Sprite.h"

class ParticleEmitter
{
public:

    ~ParticleEmitter();
    void Finalize();  
    void Initialize(Object3dCommon* objCommon, SpriteCommon* sprCommon);

    void Emit(int count,
        ParticleType type,
        const char* modelOrTexture,
        const Vector3& spawnPos,
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

        // 让这个发射器的 3D 粒子跟随相机平移
    void SetFollowCamera(bool follow) { followCamera_ = follow; }

    // 每帧把相机的位移量传进来，用来修正粒子位置
    void ApplyCameraMove(const Vector3& delta);

private:
    Object3dCommon* objCommon_ = nullptr;
    SpriteCommon* sprCommon_   = nullptr;

    std::vector<Particle> particles_;
    std::vector<Object3d*> modelPool_;
    std::vector<Sprite*> spritePool_;

    bool windMode_ = false;
    bool useOriginalSpriteSize_ = false;
    size_t maxParticles_ = 200;

    bool snowMode_              = false;

    bool followCamera_   = false;

};
