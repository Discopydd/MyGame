#pragma once
#include "Object3dCommon.h"
#include "Object3d.h"
#include "Camera.h"
#include "WinApp.h"
#include "MyMath.h"
#include <vector>

// 需要用到 Player 的 GetHpRatio
#include <player/Player.h>

class HPBar3DManager {
public:
    HPBar3DManager() = default;
    ~HPBar3DManager() = default;

    // hpNdcZ: 用来控制血条离相机的深度（和你 GameScene 里的 hpNdcZ_ 同一个值）
    void Initialize(Object3dCommon* objCommon,
                    Camera* camera,
                    Player* player,
                    float hpNdcZ);

    void Finalize();

    // 每帧更新（计算可见段数 & 重新摆放位置）
    void Update(float dt);

    // 在 GameScene::Draw 的 3D 阶段调用
    void Draw3D();

    // 如果以后玩家指针变了，可以重新设置
    void SetPlayer(Player* player) { player_ = player; }

private:
    Object3dCommon* object3dCommon_ = nullptr;
    Camera*         camera_         = nullptr;
    Player*         player_         = nullptr;

    std::vector<Object3d*> strips_;   // 每一段血条

    int   segments_      = 5;        // 总段数
    int   visibleCount_  = 5;        // 当前可见段数
    float insetX_        = 40.0f;     // 屏幕左侧内边距
    float insetY_        = 50.0f;     // 屏幕上侧内边距
    float segPixelW_     = 45.0f;     // 每段宽度（屏幕像素）
    float gapPixel_      = 4.0f;      // 段间距（像素）
    float hpNdcZ_        = 0.08f;     // 深度

    // 把屏幕坐标转成世界坐标（GameScene.cpp 里已有同名函数）
    Vector3 ScreenToWorld_(float sx, float sy, float ndcZ);
};
