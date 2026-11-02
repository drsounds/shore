#include <shore/document.h>
#include <shore/frame.h>
#include <shore/view.h>

namespace shore {

SHView::~SHView() { if (doc_) doc_->RemoveView(this); }
void SHView::SetDocument(SHDocument* d) {
    if (doc_) doc_->RemoveView(this);
    doc_ = d;
    if (doc_) doc_->AddView(this);
    Invalidate();
}
void SHView::Invalidate() { if (frame_) frame_->Invalidate(); }

}  // namespace shore
