#pragma once
#include "types.h"

namespace shore {

class SHDocument;
class SHCanvas;

class SHDocumentRenderer {
public:
    void Render(SHDocument& document, SHCanvas& canvas, SHRenderMode mode) const;
};

}  // namespace shore
