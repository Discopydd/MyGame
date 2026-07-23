#pragma once
#include "SpriteCommon.h"
#include "Sprite.h"
#include "WinApp.h"

#include <memory>
#include <vector>

/// <summary>
/// ゲームオーバー画面のUI、選択操作、ポータル粒子演出を管理するクラス。
/// </summary>
class GameOverManager {
public:
    enum class State {
        None,
        PortalOpen,   // 暗転＋ポータル崩壊演出
        SlideTitle,   // タイトルが上から滑り込む
        Wait,         // タイトルが停止し、入力を待つ
        Done
    };

    /// <summary>
    /// GameOverManagerのインスタンスを生成します。
    /// </summary>
    GameOverManager() = default;
    /// <summary>
    /// GameOverManagerが保持するリソースを破棄します。
    /// </summary>
    ~GameOverManager() = default;

    /// <summary>
    /// 動作に必要な参照とリソースを設定し、初期状態を構築します。
    /// </summary>
    /// <param name="spriteCommon">スプライト生成に使用する共通描画処理。</param>
    void Initialize(MyEngine::SpriteCommon* spriteCommon);
    /// <summary>
    /// 使用しているリソースを解放し、終了処理を行います。
    /// </summary>
    void Finalize();

    /// <summary>
    /// Start処理を実行します。
    /// </summary>
    void Start();          // GameOver 演出を開始
    /// <summary>
    /// 入力や経過時間に応じて、状態を1フレーム分更新します。
    /// </summary>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    void Update(float dt); // GameScene::Update 内で呼び出す
    /// <summary>
    /// 現在の状態を画面へ描画します。
    /// </summary>
    void Draw();           // GameScene::Draw 内で呼び出す

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
    /// <summary>
    /// Text Visibleを設定します。
    /// </summary>
    /// <param name="visible">表示する場合は true。</param>
    void SetTextVisible(bool visible) { showTexts_ = visible; }
    /// <summary>
    /// Draw Enabledを設定します。
    /// </summary>
    /// <param name="enabled">機能を有効にする場合は true。</param>
    void SetDrawEnabled(bool enabled) { drawEnabled_ = enabled; }

private:
    /// <summary>
    /// ゲームオーバー画面のポータル演出に使用する粒子の位置、速度、寿命を保持する構造体。
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
    State  state_ = State::None;
    float  t_     = 0.0f;

    std::unique_ptr<MyEngine::Sprite> backdropSprite_;
    std::unique_ptr<MyEngine::Sprite> portalRingSprite_;
    std::unique_ptr<MyEngine::Sprite> titleSprite_;
    std::unique_ptr<MyEngine::Sprite> promptSprite_;

    std::vector<PortalMote> motes_;
    float moteSpawnTimer_ = 0.0f;

    MyEngine::Vector2 titleSize_      = { 520.0f, 312.0f };
    MyEngine::Vector2 titleCenter_{};
    MyEngine::Vector2 titleStartPos_{};
    MyEngine::Vector2 titleEndPos_{};
    float   titleSlideTime_ = 0.65f;

    MyEngine::Vector2 ringCenter_{};
    MyEngine::Vector2 ringBaseSize_ = { 620.0f, 620.0f };
    float   ringRotation_ = 0.0f;
    float   ringPulseT_   = 0.0f;
    float   backdropAlpha_ = 0.0f;

    MyEngine::Vector2 promptBasePos_{};

    MyEngine::SpriteCommon* spriteCommon_ = nullptr;
    bool showTexts_ = true;
    bool drawEnabled_ = true;

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
    float EaseOutCubic_(float t) {
        t = Clamp01_(t);
        return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
    }
};
