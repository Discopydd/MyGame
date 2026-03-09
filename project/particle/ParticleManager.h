#pragma once
#include <vector>
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "ParticleEmitter.h"
#include <memory>
// すべての粒子エミッタを一括管理する
class ParticleManager
{
public:
    ParticleManager() = default;
    ~ParticleManager();   // デストラクタで自動解放する

    // 初期化: 3D / 2D の共通オブジェクトを受け取る
    void Initialize(Object3dCommon* objectCommon, SpriteCommon* spriteCommon);

    // 新しいエミッタを生成し、マネージャがライフサイクルを管理する
    ParticleEmitter* CreateEmitter();

    // 毎フレームすべてのエミッタを更新する
    void Update(float dt);

    // 3D 粒子（モデル粒子）を描画する
    void Draw3D();

    // 2D 粒子（Sprite 粒子）を描画する
    void Draw2D();

    // すべてのエミッタを手動でクリアする（任意。シーン切り替え時にも明示的に呼び出せる）
    void Finalize();

private:
    Object3dCommon* objCommon_ = nullptr;
    SpriteCommon* sprCommon_   = nullptr;
    std::vector<std::unique_ptr<ParticleEmitter>> emitters_;
};
