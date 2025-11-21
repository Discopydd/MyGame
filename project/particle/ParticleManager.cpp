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
    auto* e = new ParticleEmitter();
    e->Initialize(objCommon_, sprCommon_);
    emitters_.push_back(e);
    return e;
}

void ParticleManager::Update(float dt)
{
    for (auto* e : emitters_) {
        e->Update(dt);
    }
}

void ParticleManager::Draw3D()
{
    for (auto* e : emitters_) {
        e->Draw3D();
    }
}

void ParticleManager::Draw2D()
{
    for (auto* e : emitters_) {
        e->Draw2D();
    }
}

void ParticleManager::Finalize()
{
    for (auto* e : emitters_) {
        delete e;
    }
    emitters_.clear();
}
