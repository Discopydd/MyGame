#pragma once
#include "../math/Vector3.h"

namespace MyEngine {

/// <summary>
/// Transformで使用する関連データをまとめて保持する構造体です。
/// </summary>
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

} // namespace MyEngine
