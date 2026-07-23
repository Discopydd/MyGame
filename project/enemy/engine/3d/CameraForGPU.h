#pragma once
#include <Vector3.h>
namespace MyEngine {

/// <summary>
/// GPUへ転送するカメラのワールド座標を保持する構造体。
/// </summary>
struct CameraForGPU {
    Vector3 worldPosition;
    float pad;
};

} // namespace MyEngine
