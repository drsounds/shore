#pragma once
#include <string>
#include <vector>
#include "feature_registry.h"

namespace shore {

class SHConcept;

class SHFeature : public SHFeatureRegistry {
public:
    SHFeature(const std::string& id, const std::string& name, SHFeature* parent = nullptr)
        : id_(id), name_(name), parent_(nullptr) { if (parent) parent->AddFeature(this); }
    const std::string& Id() const { return id_; }
    const std::string& Name() const { return name_; }
    SHFeature* Parent() const { return parent_; }
    void AddFeature(SHFeature* feature);
    void AddConcept(SHConcept* concept);
    void Install(SHFrame& frame) const;
    const std::vector<SHFeature*>& Features() const { return features_; }
    const std::vector<SHConcept*>& Concepts() const { return concepts_; }
private:
    std::string id_;
    std::string name_;
    SHFeature* parent_;
    std::vector<SHFeature*> features_;
    std::vector<SHConcept*> concepts_;
};

}  // namespace shore
