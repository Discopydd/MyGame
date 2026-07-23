#pragma once
#include <vector>
#include "Particle.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include <memory>
/// <summary>
/// ParticleEmitterに関する処理と状態を管理するクラスです。
/// </summary>
class ParticleEmitter
{
public:

    /// <summary>
    /// ParticleEmitterが保持するリソースを破棄します。
    /// </summary>
    ~ParticleEmitter();
    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize();  
    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="objCommon">3Dオブジェクト生成に使用する共通描画処理。</param>
    /// <param name="sprCommon">スプライト生成に使用する共通描画処理。</param>
    void Initialize(MyEngine::Object3dCommon* objCommon, MyEngine::SpriteCommon* sprCommon);

    /// <summary>
    /// Emit処理を実行します。
    /// </summary>
    /// <param name="count">生成数。</param>
    /// <param name="type">生成または設定する種類。</param>
    /// <param name="modelOrTexture">処理対象のオブジェクトへのポインタ。</param>
    /// <param name="spawnPos">生成時のワールド座標。</param>
    /// <param name="minSpeed">処理に使用するminSpeedの値。</param>
    /// <param name="maxSpeed">処理に使用するmaxSpeedの値。</param>
    /// <param name="minLife">処理に使用するminLifeの値。</param>
    /// <param name="maxLife">処理に使用するmaxLifeの値。</param>
    /// <param name="horizontalBias">処理に使用するhorizontalBiasの値。</param>
    /// <param name="randomColor">処理に使用するrandomColorの値。</param>
    void Emit(int count,
        ParticleType type,
        const char* modelOrTexture,
        const MyEngine::Vector3& spawnPos,
        float minSpeed,
        float maxSpeed,
        float minLife,
        float maxLife, float horizontalBias = 0.0f, bool randomColor = false);

    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    void Update(float dt);
    /// <summary>
    /// 3D要素を画面へ描画します。
    /// </summary>
    void Draw3D();
    /// <summary>
    /// 2D要素を画面へ描画します。
    /// </summary>
    void Draw2D();
    /// <summary>
    /// Wind Modeを設定します。
    /// </summary>
    /// <param name="enable">機能を有効にする場合は true。</param>
    void SetWindMode(bool enable)            { windMode_ = enable; }
    /// <summary>
    /// Use Original Sprite Sizeを設定します。
    /// </summary>
    /// <param name="on">機能を有効にする場合は true。</param>
    void SetUseOriginalSpriteSize(bool on)   { useOriginalSpriteSize_ = on; }
    /// <summary>
    /// Max Particlesを設定します。
    /// </summary>
    /// <param name="max">最大座標。</param>
    void SetMaxParticles(size_t max) { maxParticles_ = max; }
    /// <summary>
    /// Snow Modeを設定します。
    /// </summary>
    /// <param name="enable">機能を有効にする場合は true。</param>
    void SetSnowMode(bool enable)           { snowMode_ = enable; }

        // このエミッタの 3D 粒子をカメラの平行移動に追従させる
    /// <summary>
    /// Follow Cameraを設定します。
    /// </summary>
    /// <param name="follow">カメラへ追従させる場合は true。</param>
    void SetFollowCamera(bool follow) { followCamera_ = follow; }

    // 毎フレームのカメラ移動量を渡し、粒子位置の補正に使う
    /// <summary>
    /// Camera Moveを適用します。
    /// </summary>
    /// <param name="delta">外部から加える移動量。</param>
    void ApplyCameraMove(const MyEngine::Vector3& delta);

private:
    MyEngine::Object3dCommon* objCommon_ = nullptr;
    MyEngine::SpriteCommon* sprCommon_   = nullptr;

    std::vector<Particle> particles_;

    // ★ 所有権を持ち、unique_ptr でプールを管理する
    std::vector<std::unique_ptr<MyEngine::Object3d>> modelPool_;
    std::vector<std::unique_ptr<MyEngine::Sprite>>   spritePool_;

    bool windMode_ = false;
    bool useOriginalSpriteSize_ = false;
    size_t maxParticles_ = 200;

    bool snowMode_              = false;

    bool followCamera_   = false;

};
