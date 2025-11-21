#pragma once
#include "MyMath.h"
#include "ParticleType.h"

struct Particle
{
    ParticleType type = ParticleType::Model3D;

    // 基本属性
    Vector3 position{};
    Vector3 velocity{};
    Vector3 accel{};

    float scale = 1.0f;
    float rotation = 0.0f;
    float rotationSpeed = 0.0f;

    float life = 1.0f;      // 剩余生命
    float maxLife = 1.0f;   // 初始生命

    // sprite 粒子需要的颜色
    Vector4 color = {1,1,1,1};

    bool IsAlive() const { return life > 0; }
};
