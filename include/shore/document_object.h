#pragma once
#include <memory>
#include <vector>
#include "canvas.h"
#include "property_group.h"

namespace shore {

class SHDocumentObject {
public:
    virtual ~SHDocumentObject() = default;
    const SHRect& Bounds() const { return bounds_; }
    void SetBounds(const SHRect& bounds) { bounds_ = bounds; }
    SHDocumentObject* Parent() const { return parent_; }
    SHDocumentObject* AddChild(std::unique_ptr<SHDocumentObject> child);
    const std::vector<std::unique_ptr<SHDocumentObject>>& Children() const { return children_; }
    SHPropertyGroup& AddPropertyGroup(const std::string& name);
    const std::vector<SHPropertyGroup>& PropertyGroups() const { return propertyGroups_; }
    virtual void OnRendering(SHCanvas& canvas, SHRenderMode mode) { (void)canvas; (void)mode; }
private:
    SHRect bounds_;
    SHDocumentObject* parent_ = nullptr;
    std::vector<std::unique_ptr<SHDocumentObject>> children_;
    std::vector<SHPropertyGroup> propertyGroups_;
};

}  // namespace shore
