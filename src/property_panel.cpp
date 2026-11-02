#include <shore/document_object.h>
#include <shore/property_panel.h>

namespace shore {

const std::vector<SHPropertyGroup>& SHPropertyPanel::Groups() const {
    static const std::vector<SHPropertyGroup> empty;
    return object_ ? object_->PropertyGroups() : empty;
}

}  // namespace shore
