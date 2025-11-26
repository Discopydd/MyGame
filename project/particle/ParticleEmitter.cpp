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
    float maxLife, float horizontalBias, bool randomColor)
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
            float dirY = RandRange(0.3f, 0.9f);    // 向下，略有变化

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
        // ===== 雪模式：3D snow.obj，从上往下飘，略微偏斜 =====
        else if (snowMode_ && type == ParticleType::Model3D) {
            // 先按风的规则随机一个屏幕方向（右上 -> 左下）
            float dirScreenX = RandRange(-1.0f, -0.6f); // 屏幕向左
            float dirScreenY = RandRange(0.3f, 0.9f);   // 屏幕向下

            float len2D = std::sqrt(dirScreenX * dirScreenX + dirScreenY * dirScreenY);
            if (len2D > 0.0f) {
                dirScreenX /= len2D;
                dirScreenY /= len2D;
            }

            // 转成世界方向：
            //   屏幕 X → 世界 X（左右）
            //   屏幕 Y 向下 → 世界 -Y（向下）
            float dirX = dirScreenX;
            float dirY = -dirScreenY; // 注意取负号，世界里向下是 -Y
            float dirZ = RandRange(-0.1f, 0.1f); // 前后给一点小扰动即可

            float len3D = std::sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
            if (len3D > 0.0f) {
                dirX /= len3D;
                dirY /= len3D;
                dirZ /= len3D;
            }

            p.velocity = { dirX * speed, dirY * speed, dirZ * speed };

            // 给一点横向“风加速度”和轻微摆动
            p.accel = {
                RandRange(-0.15f, 0.15f),
                0.0f,
                RandRange(-0.10f, 0.10f)
            };

            p.scale = RandRange(0.4f, 0.8f);
            p.rotationSpeed = RandRange(-0.5f, 0.5f);
        }
        else {
            // ===== 普通粒子：保持你原来的随机方向逻辑 =====
            float sideSign = 0.0f;
            if (horizontalBias > 0.1f)      sideSign = 1.0f;
            else if (horizontalBias < -0.1f) sideSign = -1.0f;

            float dirX;
            if (sideSign == 0.0f) {
                // 没有偏向：左右随机一点点
                dirX = RandRange(-0.12f, 0.12f);
            }
            else {
                // 有偏向：给一个比较强的侧向分量
                dirX = RandRange(0.5f, 0.9f) * sideSign;
            }
            float dirY;
            float dirZ;

            if (randomColor && type == ParticleType::Model3D) {
                // ⭐ dash 彩色星星：上下散开多一点
                dirY = RandRange(-1.1f, -0.3f);    // 有的更朝下，有的几乎水平
                dirZ = RandRange(-0.25f, 0.25f);   // 前后也稍微散开一点
            }
            else {
                // 其它 3D 粒子 / 2D 粒子：保持原来的比较窄的范围
                dirY = RandRange(-1.0f, -0.85f);
                dirZ = RandRange(-0.12f, 0.12f);
            }

            float len = std::sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
            if (len > 0.0f) {
                dirX /= len;
                dirY /= len;
                dirZ /= len;
            }

            p.velocity = { dirX * speed, dirY * speed, dirZ * speed };

            // 尾气块一般比较小
            p.scale = RandRange(0.18f, 0.30f);
            p.rotationSpeed = RandRange(-1.2f, 1.2f);

            if (p.type == ParticleType::Sprite2D) {
                p.color = { 1.0f, 1.0f, 1.0f, 0.7f };
            }

            // 让这种普通 3D 粒子默认有一点重力（可选）
            if (p.type == ParticleType::Model3D) {
                p.accel = { 0.0f, -9.8f * 1.2f, 0.0f }; // 往下加速，尾巴拉长一点
            }
            if (randomColor && p.type == ParticleType::Model3D) {
                int c = rand() % 6;
                switch (c) {
                case 0: p.color = { 1.0f, 0.4f, 0.4f, 1.0f }; break; // 红
                case 1: p.color = { 1.0f, 0.8f, 0.3f, 1.0f }; break; // 黄
                case 2: p.color = { 0.4f, 1.0f, 0.4f, 1.0f }; break; // 绿
                case 3: p.color = { 0.4f, 0.8f, 1.0f, 1.0f }; break; // 蓝
                case 4: p.color = { 0.9f, 0.4f, 1.0f, 1.0f }; break; // 紫
                case 5: p.color = { 1.0f, 0.6f, 0.9f, 1.0f }; break; // 粉
                }
            }
            else {
                // 默认：不随机，就用白色
                p.color = { 1.0f, 1.0f, 1.0f, 1.0f };
            }
        }

        p.life = RandRange(minLife, maxLife);
        p.maxLife = p.life;

        // 3) 只在“第一次使用这个槽位”的时候创建视觉对象，
        //    以后复用同一个槽位就不再 new 新对象
        if (type == ParticleType::Model3D) {
            if (needNewVisual) {
                Object3d* o = new Object3d();
                o->Initialize(objCommon_);
                o->SetModel(modelOrTex);  // 模型路径
                o->SetCamera(objCommon_->GetDefaultCamera());
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
        if (snowMode_ && p.type == ParticleType::Model3D) {
            float phase = p.rotationSpeed;       // 让每片雪不同步
            float swayX = std::sin(age * 1.8f + phase) * 0.15f; // 幅度/频率都很小
            float swayZ = std::cos(age * 1.3f + phase) * 0.12f;

            p.position.x += swayX * dt;
            p.position.z += swayZ * dt;

            // 让旋转更像“轻轻翻滚”
            p.rotation += p.rotationSpeed * dt * 0.6f;
        }
        if (p.type == ParticleType::Model3D)
        {
            Object3d* o = modelPool_[modelID++];
            o->SetTranslate(p.position);
            o->SetRotate({ 0, p.rotation, 0 });
            o->SetScale({ p.scale,p.scale,p.scale });
            o->SetColor(p.color);

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

void ParticleEmitter::ApplyCameraMove(const Vector3& delta)
{
    if (!followCamera_) {
        return;
    }

    // 没有移动就不必遍历
    if (delta.x == 0.0f && delta.y == 0.0f && delta.z == 0.0f) {
        return;
    }

    for (auto& p : particles_) {
        if (!p.IsAlive()) {
            continue;
        }

        // 这里只处理“雪花”的 3D 粒子
        if (snowMode_ && p.type == ParticleType::Model3D) {
            p.position.x += delta.x;
            p.position.y += delta.y;
            p.position.z += delta.z;
        }
    }
}
