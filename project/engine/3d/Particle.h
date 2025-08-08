#pragma once
#include"math/Transform.h"
#include"math/Matrix4x4.h"
#include"math/Vector4.h"
struct Particle
{
    Transform transform;
    Vector3 velocity;
    Vector4 color;
    float lifeTime;
    float currentTime;
};
struct ParticleForGPU
{
   Matrix4x4 WVP;
    Matrix4x4 World;
    Vector4 color;
};

std::random_device seedGenerator;
std::mt19937 randomEngine(seedGenerator());

Particle MakeNewParticle(std::mt19937& randomEngine) {
    std::uniform_real_distribution<float> distribution(-0.5f, 0.5f);
    std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
    std::uniform_real_distribution<float> distTime(1.0f, 2.0f);
    Particle particle;

    particle.transform.scale = { 0.25f, 0.5f, 0.5f };
    particle.transform.rotate = { 0.0f, 3.0f, 0.0f };
    particle.transform.translate = { 
        distribution(randomEngine), 
        distribution(randomEngine), 
        distribution(randomEngine) 
    };
    particle.velocity = { 
        distribution(randomEngine), 
        distribution(randomEngine), 
        distribution(randomEngine) 
    };
    particle.color = {
        distColor(randomEngine),
        distColor(randomEngine),
        distColor(randomEngine),1.0f
    };
    particle.lifeTime = distTime(randomEngine);
    particle.currentTime = 0;
    return particle;
}