#pragma once
#include <Vector4.h>
#include <cstdint>
#include <Vector3.h>
struct alignas(16) PointLight {
    Vector3 position;     // 12 bytes
    float intensity;      // 4 bytes (→ 对齐16)

    Vector4 color;        // 16 bytes

    int32_t pointLighting; // 4 bytes
    float padding[3];      // 补满16字节对齐

    // 总计：16 + 16 + 16 = 48 bytes
};