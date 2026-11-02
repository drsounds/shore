#pragma once
#include <string>
#include "feature_registry.h"

namespace shore {

class SHConcept : public SHFeatureRegistry {
public:
    SHConcept(const std::string& id, const std::string& name, SHFeature* parent = nullptr);
    const std::string& Id() const { return id_; }
    const std::string& Name() const { return name_; }
    SHFeature* Parent() const { return parent_; }
    void Install(SHFrame& frame) const { InstallTools(frame); }
private:
    friend class SHFeature;
    std::string id_;
    std::string name_;
    SHFeature* parent_;
};

}  // namespace shore
