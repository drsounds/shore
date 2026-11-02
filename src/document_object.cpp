#include <shore/document_object.h>

namespace shore {

SHDocumentObject* SHDocumentObject::AddChild(std::unique_ptr<SHDocumentObject> child) {
    if (!child) return nullptr;
    child->parent_ = this;
    children_.push_back(std::move(child));
    return children_.back().get();
}

SHPropertyGroup& SHDocumentObject::AddPropertyGroup(const std::string& name) {
    propertyGroups_.push_back(SHPropertyGroup(name));
    return propertyGroups_.back();
}

}  // namespace shore
