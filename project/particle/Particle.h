#pragma once
#include "MyMath.h"
#include "ParticleType.h"

/// <summary>
/// Particleで使用する関連データをまとめて保持する構造体です。
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

    /// <summary>
    /// Aliveかを判定します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool IsAlive() const { return life > 0; }
};
