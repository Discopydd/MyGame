#pragma once
#include "../math/Vector2.h"
#include "../math/Vector3.h"

struct SkinnedVertex {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;

    // 每顶点最多4个权重
    uint8_t joint[4]{ 0,0,0,0 };
    float   weight[4]{ 1.0f,0.0f,0.0f,0.0f };
};
