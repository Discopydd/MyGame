#pragma once
#include <string>
#include <vector>
#include "Skeleton.h"
#include "Animation.h"
#include "SkinnedVertex.h"

class SkinnedModel {
public:
    // 仅容器：后续 Step B 实现 Load/Find，Step C/D 接 GPU
    const Skeleton&       GetSkeleton() const { return skeleton_; }
    const AnimationClip*  GetClipByName(const std::string& name) const;
    const std::vector<SkinnedVertex>& GetVertices() const { return vertices_; }
    const std::vector<uint32_t>&      GetIndices()  const { return indices_;  }

    // 暂时开放可写接口，Step B中由导入器填充
    Skeleton&       MutableSkeleton() { return skeleton_; }
    std::vector<SkinnedVertex>& MutableVertices() { return vertices_; }
    std::vector<uint32_t>&      MutableIndices()  { return indices_;  }
    std::vector<AnimationClip>& MutableClips()    { return clips_;    }

private:
    Skeleton skeleton_;
    std::vector<SkinnedVertex> vertices_;
    std::vector<uint32_t>      indices_;
    std::vector<AnimationClip> clips_;
};
