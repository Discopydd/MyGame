#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Camera.h"
#include "MyMath.h"
#include <vector>
#include <memory>

// GameScene 内にすでにある構造体。ここでも同じ定義を使う
struct HintSprite {
    std::unique_ptr<Sprite> sprite;
    Vector3 worldPos{};
};

// GameScene 内で実装されている WorldToScreen。ここでは宣言のみ行う
Vector3 WorldToScreen(const Vector3& worldPos, Camera* camera);

class HintUIManager {
public:
    HintUIManager() = default;
    ~HintUIManager() = default;

    void Initialize(SpriteCommon* spriteCommon, Camera* camera);
    void Finalize();

    // このマネージャに GameScene 内の HintSprite / コンテナを渡す
    void SetSpaceHint(HintSprite* spaceHint)   { spaceHint_ = spaceHint; }
    void SetShiftHint(HintSprite* shiftHint)   { shiftHint_ = shiftHint; }
    void SetSprintHint(HintSprite* sprintHint) { sprintHint_ = sprintHint; }
    void SetUpHints(std::vector<HintSprite>* upHints) { upHints_ = upHints; }

    // 毎フレーム更新：上下の揺れを計算し、ワールド座標から画面座標へ変換する
    void Update(float dt);

    // GameScene::Draw() の「中間レイヤー Sprite」内で呼び出す
    void Draw();

private:
    SpriteCommon* spriteCommon_ = nullptr;
    Camera*       camera_       = nullptr;

    HintSprite* spaceHint_  = nullptr;
    HintSprite* shiftHint_  = nullptr;
    HintSprite* sprintHint_ = nullptr;
    std::vector<HintSprite>* upHints_ = nullptr;

    // 上下に揺らすためのパラメータ（元の GameScene にあった 3 つ）
    float bobTime_      = 0.0f;
    float bobAmplitude_ = 6.0f;   // 移動ピクセル（上下±6）
    float bobSpeed_     = 3.0f;   // 周波数（大きいほど速く揺れる）

    // Move hint icons (key_A / arrow_left / key_D / arrow_right)
    std::unique_ptr<Sprite> moveKeyA_;
    std::unique_ptr<Sprite> moveArrowL_;
    std::unique_ptr<Sprite> moveKeyD_;
    std::unique_ptr<Sprite> moveArrowR_;
};
