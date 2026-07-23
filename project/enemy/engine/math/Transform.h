#pragma once
#include "../math/Vector3.h"

namespace MyEngine {

/// <summary>
/// オブジェクトの拡大率、回転量、平行移動量をまとめて保持する構造体。
/// </summary>
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

} // namespace MyEngine
