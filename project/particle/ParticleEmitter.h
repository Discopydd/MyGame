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
              float maxLife);

    void Update(float dt);
    void Draw3D();
    void Draw2D();

private:
    Object3dCommon* objCommon_ = nullptr;
    SpriteCommon* sprCommon_   = nullptr;

    std::vector<Particle> particles_;
    std::vector<Object3d*> modelPool_;
    std::vector<Sprite*> spritePool_;
};
