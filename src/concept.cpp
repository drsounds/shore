#include <shore/concept.h>
#include <shore/feature.h>

namespace shore {

SHConcept::SHConcept(const std::string& id, const std::string& name, SHFeature* parent)
    : id_(id), name_(name), parent_(nullptr) {
    if (parent) parent->AddConcept(this);
}

}  // namespace shore
