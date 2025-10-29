#include "SkinnedModel.h"

const AnimationClip* SkinnedModel::GetClipByName(const std::string& name) const {
    for (const auto& c : clips_) if (c.name == name) return &c;
    return nullptr;
}
