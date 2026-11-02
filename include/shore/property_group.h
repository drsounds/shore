#pragma once
#include <string>
#include <vector>
#include "types.h"

namespace shore {

class SHPropertyGroup {
public:
    explicit SHPropertyGroup(const std::string& name) : name_(name) {}
    SHPropertyGroup& AddProperty(const std::string& name, const std::string& value);
    const std::string& Name() const { return name_; }
    const std::vector<SHProperty>& Properties() const { return properties_; }
private:
    std::string name_;
    std::vector<SHProperty> properties_;
};

}  // namespace shore
