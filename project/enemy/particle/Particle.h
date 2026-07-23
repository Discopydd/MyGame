#pragma once
#include "MyMath.h"
#include "ParticleType.h"

/// <summary>
/// 1個のパーティクルの変換情報、速度、色、寿命、種類を保持する構造体。
/// </summary>
struct Particle
{
    ParticleType type = ParticleType::Model3D;

    // 基本属性
    MyEngine::Vector3 position{};
    MyEngine::Vector3 velocity{};
    MyEngine::Vector3 accel{};

    float scale = 1.0f;
    float rotation = 0.0f;
    float rotationSpeed = 0.0f;

    float life = 1.0f;      // 残り寿命
    float maxLife = 1.0f;   // 初期寿命

    // MyEngine::Sprite 粒子で使用する色
    MyEngine::Vector4 color = {1,1,1,1};

    bool IsAlive() const { return life > 0; }
};
