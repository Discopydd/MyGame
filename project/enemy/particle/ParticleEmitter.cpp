#include "ParticleEmitter.h"
#include <cstdlib>
#include <cmath>
using namespace MyEngine;
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
    modelPool_.clear();
    spritePool_.clear();
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
        // 1) 再利用できる粒子スロットを探す（life <= 0）
        int freeIndex = -1;
        for (size_t idx = 0; idx < particles_.size(); ++idx) {
            if (!particles_[idx].IsAlive()) {
                freeIndex = static_cast<int>(idx);
                break;
            }
        }

        bool needNewVisual = false;

        // 2) 再利用可能なスロットがなければ、新しいスロットを確保できるか確認する
        if (freeIndex == -1) {
            if (particles_.size() >= maxParticles_) {
                // すでに上限に達しているため、これ以上は新規生成せず終了する
                break;
            }
            particles_.emplace_back();                // 空の粒子を 1 つ追加する
            freeIndex = static_cast<int>(particles_.size() - 1);
            needNewVisual = true;                     // このスロットを初めて使うので、対応する Sprite/Object3d を生成する
        }

        // 使用可能な粒子スロットを確保できた
        Particle& p = particles_[freeIndex];
        p = Particle();
        p.type = type;
        p.position = spawnPos;

        float speed = RandRange(minSpeed, maxSpeed);

        if (windMode_ && type == ParticleType::Sprite2D) {
            // 画面座標: x は右が正、y は下が正
            // 右上から左下へ飛ばす: dx < 0, dy > 0
            float dirX = RandRange(-1.0f, -0.6f);   // 左方向に少しランダムを持たせる
            float dirY = RandRange(0.3f, 0.9f);    // 下方向にややばらつきを持たせる

            float len = std::sqrt(dirX * dirX + dirY * dirY);
            if (len > 0.0f) {
                dirX /= len;
                dirY /= len;
            }
            p.velocity = { dirX * speed, dirY * speed, 0.0f };

            // 回転を速度方向に合わせる
            p.rotation = std::atan2(dirY, dirX);

            // rotationSpeed を「揺れ位相」のランダム種として使う（後で Update で使用）
            p.rotationSpeed = RandRange(-3.14159f, 3.14159f);

            // 風パーティクルの拡大率: 元画像に近い大きさ（原寸を使わない場合は少し調整可能）
            p.scale = RandRange(0.8f, 1.2f);

            // 開始時は完全に透明にし、後で Update 内でフェードインさせる
            p.color = { 1.0f, 1.0f, 1.0f, 0.0f };
        }
        // ===== 雪モード: 3D の snow.obj を上から下へ、やや斜めに漂わせる =====
        else if (snowMode_ && type == ParticleType::Model3D) {
            // まず風モードと同様に、画面上の方向をランダムに決める（右上 -> 左下）
            float dirScreenX = RandRange(-1.0f, -0.6f); // 画面上で左方向
            float dirScreenY = RandRange(0.3f, 0.9f);   // 画面上で下方向

            float len2D = std::sqrt(dirScreenX * dirScreenX + dirScreenY * dirScreenY);
            if (len2D > 0.0f) {
                dirScreenX /= len2D;
                dirScreenY /= len2D;
            }

            // ワールド座標系の方向に変換する: 
            //  画面 X → ワールド X（左右）
            //  画面 Y の下方向 → ワールド -Y（下方向）
            float dirX = dirScreenX;
            float dirY = -dirScreenY; // 負符号に注意。ワールドでは下方向が -Y
            float dirZ = RandRange(-0.1f, 0.1f); // 前後方向に少しだけ揺らぎを与える

            float len3D = std::sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
            if (len3D > 0.0f) {
                dirX /= len3D;
                dirY /= len3D;
                dirZ /= len3D;
            }

            p.velocity = { dirX * speed, dirY * speed, dirZ * speed };

            // 少し横方向の「風の加速度」と軽い揺れを与える
            p.accel = {
                RandRange(-0.15f, 0.15f),
                0.0f,
                RandRange(-0.10f, 0.10f)
            };

            p.scale = RandRange(0.4f, 0.8f);
            p.rotationSpeed = RandRange(-0.5f, 0.5f);
        }
        else {
            // ===== 通常粒子: 元のランダム方向ロジックを維持する =====
            float sideSign = 0.0f;
            if (horizontalBias > 0.1f)      sideSign = 1.0f;
            else if (horizontalBias < -0.1f) sideSign = -1.0f;

            float dirX;
            if (sideSign == 0.0f) {
                // 横方向指定がない場合: 左右に少しだけランダムを与える
                dirX = RandRange(-0.12f, 0.12f);
            }
            else {
                // 横方向指定がある場合: 比較的強い横成分を与える
                dirX = RandRange(0.5f, 0.9f) * sideSign;
            }
            float dirY;
            float dirZ;

            if (randomColor && type == ParticleType::Model3D) {
                // ⭐ dash 用のカラフルな星: 上下方向に少し散らす
                dirY = RandRange(-1.1f, -0.3f);    // 下向きが強いものから、ほぼ水平なものまで混ぜる
                dirZ = RandRange(-0.25f, 0.25f);   // 前後方向にも少し散らす
            }
            else {
                // その他の 3D / 2D 粒子: 元のやや狭い範囲を維持する
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

            // 尾のパーティクルはやや小さめにする
            p.scale = RandRange(0.18f, 0.30f);
            p.rotationSpeed = RandRange(-1.2f, 1.2f);

            if (p.type == ParticleType::Sprite2D) {
                p.color = { 1.0f, 1.0f, 1.0f, 0.7f };
            }

            // この通常 3D 粒子にはデフォルトで少し重力を与える（任意）
            if (p.type == ParticleType::Model3D) {
                p.accel = { 0.0f, -9.8f * 1.2f, 0.0f }; // 下向きに加速させ、尾を少し長く見せる
            }
            if (randomColor && p.type == ParticleType::Model3D) {
                int c = rand() % 6;
                switch (c) {
                case 0: p.color = { 1.0f, 0.4f, 0.4f, 1.0f }; break; // 赤
                case 1: p.color = { 1.0f, 0.8f, 0.3f, 1.0f }; break; // 黄
                case 2: p.color = { 0.4f, 1.0f, 0.4f, 1.0f }; break; // 緑
                case 3: p.color = { 0.4f, 0.8f, 1.0f, 1.0f }; break; // 青
                case 4: p.color = { 0.9f, 0.4f, 1.0f, 1.0f }; break; // 紫
                case 5: p.color = { 1.0f, 0.6f, 0.9f, 1.0f }; break; // ピンク
                }
            }
            else {
                // デフォルト: ランダムにしない場合はそのまま白色を使う
                p.color = { 1.0f, 1.0f, 1.0f, 1.0f };
            }
        }

        p.life = RandRange(minLife, maxLife);
        p.maxLife = p.life;

        // 3) 「このスロットを初めて使うとき」だけ見た目用オブジェクトを生成し、
        //    以後は同じスロットを再利用し、新しいオブジェクトは生成しない
        if (type == ParticleType::Model3D) {
            if (needNewVisual) {
                auto o = std::make_unique<Object3d>();
                o->Initialize(objCommon_);
                o->SetModel(modelOrTex);
                o->SetCamera(objCommon_->GetDefaultCamera());
                o->SetEnableLighting(false);
                modelPool_.push_back(std::move(o));
            }
        }
        else { // Sprite 粒子
            if (needNewVisual) {
                auto s = std::make_unique<Sprite>();
                s->Initialize(sprCommon_, modelOrTex);
                s->SetVisible(true);
                spritePool_.push_back(std::move(s));
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

         float lifeRatio = p.life / p.maxLife;   // 1 -> 0
        float age       = p.maxLife - p.life;   // 経過した生存時間
        float age01     = age / p.maxLife;      // 0 -> 1

        // ===== Sprite 粒子: より自然なフェードイン / 維持 / フェードアウト =====
        if (p.type == ParticleType::Sprite2D) {
            float alpha = 0.0f;

            // 最初の 20% の時間: 0 -> 1 にフェードイン
            if (age01 < 0.2f) {
                alpha = age01 / 0.2f;
            }
            // 中間の 60%: 最大輝度を維持
            else if (age01 < 0.8f) {
                alpha = 1.0f;
            }
            // 最後の 20%: 1 -> 0 にフェードアウト
            else {
                alpha = (1.0f - age01) / 0.2f;
            }

            p.color.w = alpha * 0.7f;   // 最大明るさは 0.7。必要に応じて調整可能
        }

        // ===== 基礎直線移動 =====
        p.velocity += p.accel * dt;
        p.position += p.velocity * dt;

        // ===== 風モード: 直線移動に少し「左右の揺れ」を加え、回転を速度に合わせる =====
        if (windMode_ && p.type == ParticleType::Sprite2D) {
            // rotationSpeed をランダム位相として使い、粒子ごとの動きをずらす
            float phase = p.rotationSpeed;
            float sway  = std::sin(age * 8.0f + phase) * 25.0f; // 8 = 周波数、25 = 揺れ幅（ピクセル）

            // 現在の速度から「横方向」（速度に垂直）を計算する
            Vector3 v = p.velocity;
            float len = std::sqrt(v.x * v.x + v.y * v.y);
            if (len > 0.001f) {
                Vector3 side = { -v.y / len, v.x / len, 0.0f }; // 速度に垂直
                p.position.x += side.x * sway * dt;
                p.position.y += side.y * sway * dt;

                // テクスチャの向きを移動方向に合わせる
                p.rotation = std::atan2(v.y, v.x);
            }
        }
        else {
            // 風モードでない場合は、元の回転ロジックをそのまま使う
            p.rotation += p.rotationSpeed * dt;
        }
        if (snowMode_ && p.type == ParticleType::Model3D) {
            float phase = p.rotationSpeed;       // 雪片ごとに動きをずらす
            float swayX = std::sin(age * 1.8f + phase) * 0.15f; // 振幅も周波数も小さめ
            float swayZ = std::cos(age * 1.3f + phase) * 0.12f;

            p.position.x += swayX * dt;
            p.position.z += swayZ * dt;

            // 回転は「ふわっと翻る」ように見せる
            p.rotation += p.rotationSpeed * dt * 0.6f;
        }
        if (p.type == ParticleType::Model3D)
        {
            Object3d* o = modelPool_[modelID++].get();
            o->SetTranslate(p.position);
            o->SetRotate({ 0, p.rotation, 0 });
            o->SetScale({ p.scale,p.scale,p.scale });
            o->SetColor(p.color);

            o->Update();
        }
        else
        {
            Sprite* s = spritePool_[spriteID++].get();
            s->SetPosition({ p.position.x, p.position.y });
            s->SetRotation(p.rotation);
            s->SetColor(p.color);

            // 「元画像サイズを使わない」場合のみ、Size を再設定する
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

    // 移動量がなければ走査する必要はない
    if (delta.x == 0.0f && delta.y == 0.0f && delta.z == 0.0f) {
        return;
    }

    for (auto& p : particles_) {
        if (!p.IsAlive()) {
            continue;
        }

        // ここでは「雪」の 3D 粒子のみを処理する
        if (snowMode_ && p.type == ParticleType::Model3D) {
            p.position.x += delta.x;
            p.position.y += delta.y;
            p.position.z += delta.z;
        }
    }
}
