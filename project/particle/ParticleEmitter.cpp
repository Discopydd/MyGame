#include "ParticleEmitter.h"
#include <cstdlib>
#include <cmath>
static float RandRange(float a, float b)
{
    float t = (float)rand() / RAND_MAX;
    return a + (b - a)*t;
}
ParticleEmitter::~ParticleEmitter()
{
    Finalize();
}
void ParticleEmitter::Finalize()
{
    // 删除 3D 粒子对象池
    for (auto* o : modelPool_) {
        delete o;
    }
    modelPool_.clear();

    // 删除 2D 粒子对象池
    for (auto* s : spritePool_) {
        delete s;
    }
    spritePool_.clear();

    // 清空粒子数据
    particles_.clear();
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
    for (int i = 0; i < count; i++)
    {
        // 1) 找一个可以复用的粒子槽位（life <= 0）
        int freeIndex = -1;
        for (size_t idx = 0; idx < particles_.size(); ++idx) {
            if (!particles_[idx].IsAlive()) {
                freeIndex = static_cast<int>(idx);
                break;
            }
        }

        bool needNewVisual = false;

        // 2) 没有可复用的，就看看能否开一个新槽位
        if (freeIndex == -1) {
            if (particles_.size() >= maxParticles_) {
                // 已经满了，不再生成新的，直接跳出
                break;
            }
            particles_.emplace_back();                // 新增一个空粒子
            freeIndex = static_cast<int>(particles_.size() - 1);
            needNewVisual = true;                     // 第一次用这个槽位，需要创建对应的 Sprite/Object3d
        }

        // 现在有了一个可用的粒子槽位
        Particle& p = particles_[freeIndex];
        p = Particle();
        p.type = type;
        p.position = spawnPos;

        float speed = RandRange(minSpeed, maxSpeed);

          if (windMode_ && type == ParticleType::Sprite2D) {
            // 屏幕坐标：x 向右为正, y 向下为正
            // 想要从右上飞到左下：dx < 0, dy > 0
            float dirX = RandRange(-1.0f, -0.6f);   // 向左一点点随机
            float dirY = RandRange(0.3f,  0.9f);    // 向下，略有变化

            float len = std::sqrt(dirX * dirX + dirY * dirY);
            if (len > 0.0f) {
                dirX /= len;
                dirY /= len;
            }
            p.velocity = { dirX * speed, dirY * speed, 0.0f };

            // 旋转朝速度方向
            p.rotation = std::atan2(dirY, dirX);

            // 用 rotationSpeed 作为“摆动相位”的随机种子（后面 Update 用）
            p.rotationSpeed = RandRange(-3.14159f, 3.14159f);

            // 风的缩放：接近原图大小（如果你没用原尺寸，这个可以略调一点点）
            p.scale = RandRange(0.8f, 1.2f);

            // 一开始完全透明，后面在 Update 里做淡入
            p.color = { 1.0f, 1.0f, 1.0f, 0.0f };
        }
        else {
            // ===== 普通粒子：保持你原来的随机方向逻辑 =====
            p.velocity = {
                RandRange(-1, 1) * speed,
                RandRange(0.3f, 1) * speed,
                RandRange(-1, 1) * speed
            };

            p.scale         = RandRange(0.2f, 0.5f);
            p.rotationSpeed = RandRange(-1, 1);

            if (p.type == ParticleType::Sprite2D) {
                p.color = { 1.0f, 1.0f, 1.0f, 0.7f };
            }
        }

        p.life    = RandRange(minLife, maxLife);
        p.maxLife = p.life;
        p.scale   = RandRange(0.2f, 0.5f);
        p.rotationSpeed = RandRange(-1, 1);

        if (p.type == ParticleType::Sprite2D) {
            p.color = { 1.0f, 1.0f, 1.0f, 0.7f };  // 保持你原来的风的透明度
        }

        // 3) 只在“第一次使用这个槽位”的时候创建视觉对象，
        //    以后复用同一个槽位就不再 new 新对象
        if (type == ParticleType::Model3D) {
            if (needNewVisual) {
                Object3d* o = new Object3d();
                o->Initialize(objCommon_);
                o->SetModel(modelOrTex);  // 模型路径
                o->SetEnableLighting(false);
                modelPool_.push_back(o);
            }
        }
        else { // Sprite 粒子
            if (needNewVisual) {
                Sprite* s = new Sprite();
                s->Initialize(sprCommon_, modelOrTex); // 图片路径

                // 如果你有“使用原图尺寸”的逻辑，可以在这里决定要不要 SetSize
                // s->SetSize({32, 32});

                s->SetVisible(true);
                spritePool_.push_back(s);
            }
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

         float lifeRatio = p.life / p.maxLife;   // 从 1 -> 0
        float age       = p.maxLife - p.life;   // 已经活了多久
        float age01     = age / p.maxLife;      // 0 -> 1

        // ===== Sprite 粒子：更自然的 淡入/保持/淡出 =====
        if (p.type == ParticleType::Sprite2D) {
            float alpha = 0.0f;

            // 前 20% 时间：从 0 -> 1 淡入
            if (age01 < 0.2f) {
                alpha = age01 / 0.2f;
            }
            // 中间 60%：维持全亮
            else if (age01 < 0.8f) {
                alpha = 1.0f;
            }
            // 最后 20%：从 1 -> 0 淡出
            else {
                alpha = (1.0f - age01) / 0.2f;
            }

            p.color.w = alpha * 0.7f;   // 最大亮度 0.7，可按需要调整
        }

        // ===== 基础直线运动 =====
        p.velocity += p.accel * dt;
        p.position += p.velocity * dt;

        // ===== 风模式：在直线基础上加一点“左右摆动” + 旋转对齐速度 =====
        if (windMode_ && p.type == ParticleType::Sprite2D) {
            // 用 rotationSpeed 当随机相位，让不同粒子不同步
            float phase = p.rotationSpeed;
            float sway  = std::sin(age * 8.0f + phase) * 25.0f; // 8 = 频率, 25 = 摆动幅度(像素)

            // 根据当前速度计算“侧向”方向（与速度垂直）
            Vector3 v = p.velocity;
            float len = std::sqrt(v.x * v.x + v.y * v.y);
            if (len > 0.001f) {
                Vector3 side = { -v.y / len, v.x / len, 0.0f }; // 垂直于速度
                p.position.x += side.x * sway * dt;
                p.position.y += side.y * sway * dt;

                // 让纹理朝向运动方向
                p.rotation = std::atan2(v.y, v.x);
            }
        }
        else {
            // 非风的情况就保持原来的旋转逻辑
            p.rotation += p.rotationSpeed * dt;
        }

        if (p.type == ParticleType::Model3D)
        {
            Object3d* o = modelPool_[modelID++];
            o->SetTranslate(p.position);
            o->SetRotate({ 0, p.rotation, 0 });
            o->SetScale({ p.scale,p.scale,p.scale });
            o->Update();
        }
        else
        {
            Sprite* s = spritePool_[spriteID++];
            s->SetPosition({ p.position.x, p.position.y });
            s->SetRotation(p.rotation);
            s->SetColor(p.color);

            // 只有在没有“用原图尺寸”时才重设 Size
            if (!useOriginalSpriteSize_) {
                s->SetSize({ 32 * p.scale, 32 * p.scale });
            }

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
