#include "ParticleManager.h"

ParticleManager::~ParticleManager()
{
    Finalize();
}

void ParticleManager::Initialize(Object3dCommon* obj, SpriteCommon* spr)
{
    objCommon_ = obj;
    sprCommon_ = spr;
}

ParticleEmitter* ParticleManager::CreateEmitter()
{
    auto emitter = std::make_unique<ParticleEmitter>();
    emitter->Initialize(objCommon_, sprCommon_);

    ParticleEmitter* raw = emitter.get();
    emitters_.push_back(std::move(emitter));
    return raw;
}

void ParticleManager::Update(float dt)
{
   for (auto& e : emitters_) {
        e->Update(dt);
    }
}

void ParticleManager::Draw3D()
{
    for (auto& e : emitters_) {
        e->Draw3D();
    }
}

void ParticleManager::Draw2D()
{
    for (auto& e : emitters_) {
        e->Draw2D();
    }
}

void ParticleManager::Finalize()
{
   for (auto& e : emitters_) {
        if (e) {
            e->Finalize();
        }
    }
    emitters_.clear();
}
