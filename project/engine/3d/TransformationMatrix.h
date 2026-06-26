#pragma once
#include "Matrix4x4.h"
namespace MyEngine {

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

} // namespace MyEngine
