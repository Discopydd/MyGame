#pragma once
#include <string>
#include <vector>
#include "../math/Matrix4x4.h"

struct Bone {
    std::string name;
    int parent = -1;              // -1 表示 root
    Matrix4x4   bindLocal;        // 绑定姿势下的局部矩阵（T*R*S）
    Matrix4x4   inverseBind;      // 逆绑定矩阵
};

struct Skeleton {
    std::vector<Bone> bones;
    std::vector<int>  roots;      // 允许多根（通常1个）
};
