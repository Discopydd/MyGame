#pragma once
#include "../math/Vector4.h"
#include <cstdint>
#include "../math/Matrix4x4.h"
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
	float shininess;
};