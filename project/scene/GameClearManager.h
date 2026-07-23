#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "Camera.h"
#include "MyMath.h"
#include "WinApp.h"
#include <memory>
#include <vector>

/// <summary>
/// GameClearManagerに関する処理と状態を管理するクラスです。
/// </summary>
class GameClearManager {
public:
    enum class State { None, SlideTitle, PlayerShow, Done };

    /// <summary>
    /// GameClearManagerのインスタンスを生成します。
    /// </summary>
    GameClearManager() = default;
    /// <summary>
    /// GameClearManagerが保持するリソースを破棄します。
    /// </summary>
    ~GameClearManager() = default;

    // ポインタ形式で初期化
    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="spriteCommon">スプライト生成に使用する共通描画処理。</param>
    /// <param name="object3dCommon">3Dオブジェクト生成に使用する共通描画処理。</param>
    /// <param name="camera">描画および座標変換に使用するカメラ。</param>
    /// <param name="hpNdcZ">HP表示に使用するNDC空間のZ値。</param>
    void Initialize(MyEngine::SpriteCommon* spriteCommon,
                    MyEngine::Object3dCommon* object3dCommon,
                    MyEngine::Camera* camera,
                    float hpNdcZ);

    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize();

    /// <summary>
    /// Start処理を実行します。
    /// </summary>
    void Start();          // GameClear 演出を開始
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    void Update(float dt); // 状態機の更新（GameScene::Update 内で呼び出す）

    // 勝利画面の 2D 要素を描画
    /// <summary>
    /// Titleを描画します。
    /// </summary>
    void DrawTitle();

    // 状態確認
    /// <summary>
    /// Playingかを判定します。
    /// </summary>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool IsPlaying() const {
        return (state_ != State::None && state_ != State::Done);
    }
    /// <summary>
    /// Stateを取得します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
    State GetState() const { return state_; }

private:
    /// <summary>
    /// PortalMoteで使用する関連データをまとめて保持する構造体です。
    /// </summary>
    struct PortalMote {
        std::unique_ptr<MyEngine::Sprite> sprite;
        float angle = 0.0f;
        float radius = 0.0f;
        float angularSpeed = 0.0f;
        float radialSpeed = 0.0f;
        float driftY = 0.0f;
        float life = 0.0f;
        float maxLife = 0.0f;
        float baseSize = 16.0f;
        MyEngine::Vector4 color = { 1,1,1,1 };
    };

private:
    /// <summary>
    /// Moteを生成します。
    /// </summary>
    void SpawnMote_();
    /// <summary>
    /// Motesを更新します。
    /// </summary>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    void UpdateMotes_(float dt);

    /// <summary>
    /// Clamp 01処理を実行します。
    /// </summary>
    /// <param name="t">補間係数。</param>
    /// <returns>計算または取得した数値。</returns>
    float Clamp01_(float t) {
        if (t < 0.0f) { return 0.0f; }
        if (t > 1.0f) { return 1.0f; }
        return t;
    }

    /// <summary>
    /// Ease Out Back処理を実行します。
    /// </summary>
    /// <param name="t">補間係数。</param>
    /// <returns>計算または取得した数値。</returns>
    float EaseOutBack_(float t) {
        float c1 = 1.70158f;
        float c3 = c1 + 1.0f;
        float p  = t - 1.0f;
        return 1.0f + p * p * (c3 * p + c1);
    }

    /// <summary>
    /// Ease Out Cubic処理を実行します。
    /// </summary>
    /// <param name="t">補間係数。</param>
    /// <returns>計算または取得した数値。</returns>
    float EaseOutCubic_(float t) { return 1.0f - powf(1.0f - t, 3.0f); }

private:
    // 内部状態
    State state_ = State::None;
    float t_     = 0.0f;   // 汎用タイマー

    // === タイトル / 背景関連 ===
    std::unique_ptr<MyEngine::Sprite> backdropSprite_;
    std::unique_ptr<MyEngine::Sprite> portalRingSprite_;
    std::unique_ptr<MyEngine::Sprite> titleSprite_;
    std::unique_ptr<MyEngine::Sprite> panelSprite_;
    std::unique_ptr<MyEngine::Sprite> promptSprite_;

    std::vector<PortalMote> motes_;
    float moteSpawnTimer_ = 0.0f;

    MyEngine::Vector2 titleSize_   = { 1280.0f, 720.0f };
    MyEngine::Vector2 titlePos_{};
    MyEngine::Vector2 titleStartPos_{};
    MyEngine::Vector2 titleEndPos_{};
    float   titleSlideTime_ = 0.65f;

    MyEngine::Vector2 ringCenter_{};
    MyEngine::Vector2 ringBaseSize_ = { 560.0f, 560.0f };
    float   ringRotation_ = 0.0f;
    float   ringPulseT_   = 0.0f;
    float   uiFloatT_     = 0.0f;
    MyEngine::Vector2 panelBasePos_{};
    MyEngine::Vector2 promptBasePos_{};

    // 共用リソースへのポインタ（自前では delete しない）
    MyEngine::SpriteCommon*   spriteCommon_   = nullptr;
    MyEngine::Object3dCommon* object3dCommon_ = nullptr;
    MyEngine::Camera*         camera_         = nullptr;
    float           hpNdcZ_         = 0.08f;
};
