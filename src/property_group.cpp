#include <shore/property_group.h>

namespace shore {

SHPropertyGroup& SHPropertyGroup::AddProperty(const std::string& name, const std::string& value) {
    properties_.push_back({name, value});
    return *this;
}

}  // namespace shore
