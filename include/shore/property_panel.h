#pragma once
#include <vector>
#include "panel.h"
#include "property_group.h"

namespace shore {

class SHDocumentObject;

class SHPropertyPanel : public SHPanel {
public:
    SHPropertyPanel(const std::string& id, const std::string& title, SHDocumentObject* object = nullptr)
        : SHPanel(id, title, SHDockPosition::Right), object_(object) {}
    SHDocumentObject* Object() const { return object_; }
    void SetObject(SHDocumentObject* object) { object_ = object; }
    const std::vector<SHPropertyGroup>& Groups() const;
private:
    SHDocumentObject* object_;
};

}  // namespace shore
