#include "ParticleEmitter.h"
#include <cstdlib>

static float RandRange(float a, float b)
{
    float t = (float)rand() / RAND_MAX;
    return a + (b - a)*t;
}

void ParticleEmitter::Initialize(Object3dCommon* objCommon, SpriteCommon* sprCommon)
{
    objCommon_ = objCommon;
    sprCommon_ = sprCommon;
}

void ParticleEmitter::Emit(int count,
                           ParticleType type,
                           const char* modelOrTex,
                           const Vector3& spawnPos,
                           float minSpeed,
                           float maxSpeed,
                           float minLife,
                           float maxLife)
{
    for (int i=0; i<count; i++)
    {
        Particle p;
        p.type = type;
        p.position = spawnPos;

        float speed = RandRange(minSpeed, maxSpeed);
        p.velocity = {
            RandRange(-1,1)*speed,
            RandRange(0.3f,1)*speed,
            RandRange(-1,1)*speed
        };

        p.life = RandRange(minLife, maxLife);
        p.maxLife = p.life;
        p.scale = RandRange(0.2f, 0.5f);
        p.rotationSpeed = RandRange(-1, 1);

        particles_.push_back(p);

        // 构建视觉对象
        if (type == ParticleType::Model3D)
        {
            Object3d* o = new Object3d();
            o->Initialize(objCommon_);
            o->SetModel(modelOrTex);  // 模型路径
            o->SetEnableLighting(false);
            modelPool_.push_back(o);
        }
        else // Sprite 粒子
        {
            Sprite* s = new Sprite();
            s->Initialize(sprCommon_, modelOrTex); // 图片路径
            s->SetSize({32,32});
            s->SetVisible(true);
            spritePool_.push_back(s);
        }
    }
}

void ParticleEmitter::Update(float dt)
{
    size_t modelID = 0;
    size_t spriteID = 0;

    for (auto& p : particles_) {
        if (!p.IsAlive()) {
            continue;
        }

        p.life -= dt;
        if (p.life <= 0) continue;

        p.velocity += p.accel * dt;
        p.position += p.velocity * dt;
        p.rotation += p.rotationSpeed * dt;

        if (p.type == ParticleType::Model3D)
        {
            Object3d* o = modelPool_[modelID++];
            o->SetTranslate(p.position);
            o->SetRotate({0, p.rotation, 0});
            o->SetScale({p.scale,p.scale,p.scale});
            o->Update();
        }
        else
        {
            Sprite* s = spritePool_[spriteID++];
            s->SetPosition({p.position.x, p.position.y});
            s->SetRotation(p.rotation);
            s->SetColor(p.color);
            s->SetSize({32*p.scale,32*p.scale});
            s->Update();
        }
    }
}

void ParticleEmitter::Draw3D()
{
    size_t idx = 0;
    for (auto& p : particles_) {
        if (p.type == ParticleType::Model3D && p.IsAlive()) {
            modelPool_[idx++]->Draw();
        }
    }
}

void ParticleEmitter::Draw2D()
{
    size_t idx = 0;
    for (auto& p : particles_) {
        if (p.type == ParticleType::Sprite2D && p.IsAlive()) {
            spritePool_[idx++]->Draw();
        }
    }
}
