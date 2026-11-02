#include <shore/document.h>
#include <shore/document_renderer.h>
#include <shore/document_object.h>
#include <shore/canvas.h>

namespace shore {
namespace {
void RenderObjects(const std::vector<std::unique_ptr<SHDocumentObject>>& objects,
                   SHCanvas& canvas, SHRenderMode mode) {
    for (const auto& object : objects) {
        object->OnRendering(canvas, mode);
        RenderObjects(object->Children(), canvas, mode);
    }
}
}  // namespace

void SHDocumentRenderer::Render(SHDocument& document, SHCanvas& canvas, SHRenderMode mode) const {
    document.OnRendering(canvas, mode);
    RenderObjects(document.Objects(), canvas, mode);
}

}  // namespace shore
