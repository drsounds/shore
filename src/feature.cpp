#include <algorithm>
#include <shore/concept.h>
#include <shore/feature.h>

namespace shore {
namespace {
template <typename T>
void RegisterUnique(std::vector<T*>& values, T* value) {
    if (value && std::find(values.begin(), values.end(), value) == values.end()) values.push_back(value);
}
}  // namespace

void SHFeature::AddFeature(SHFeature* feature) {
    if (!feature || feature == this) return;
    for (SHFeature* ancestor = this; ancestor; ancestor = ancestor->parent_)
        if (ancestor == feature) return;
    if (feature->parent_) {
        auto& siblings = feature->parent_->features_;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), feature), siblings.end());
    }
    feature->parent_ = this;
    RegisterUnique(features_, feature);
}

void SHFeature::AddConcept(SHConcept* concept) {
    if (!concept) return;
    if (concept->parent_) {
        auto& concepts = concept->parent_->concepts_;
        concepts.erase(std::remove(concepts.begin(), concepts.end(), concept), concepts.end());
    }
    concept->parent_ = this;
    RegisterUnique(concepts_, concept);
}

void SHFeature::Install(SHFrame& frame) const {
    InstallTools(frame);
    for (SHConcept* concept : concepts_) if (concept) concept->Install(frame);
    for (SHFeature* feature : features_) if (feature) feature->Install(frame);
}

}  // namespace shore
