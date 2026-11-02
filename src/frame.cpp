#include <shore/frame.h>
#include <shore/canvas.h>
#include <shore/panel.h>
#include <shore/toolbar.h>
#include <shore/view.h>
#include <shore/app.h>

namespace shore {

SHFrame::SHFrame(const std::string& title, int w, int h)
    : title_(title), width_(w), height_(h), peer_(SHApp::CreateFramePeer(this)) {}
SHFrame::~SHFrame() { view_.reset(); peer_.reset(); }

void SHFrame::SetTitle(const std::string& t) { title_ = t; peer_->SetTitle(t); }
void SHFrame::AddMenuItem(const std::string& menu, const std::string& label, int id, Handler h) {
    handlers_[id] = std::move(h);
    menus_.push_back({menu, label, id});
    peer_->AddMenuItem(menu, label, id);
}
void SHFrame::SetToolBar(std::unique_ptr<SHToolBar> tb) {
    toolbar_ = std::move(tb);
    if (toolbar_) peer_->SetToolBar(*toolbar_);
}
void SHFrame::AddToolBarButton(int id, const std::string& label, const std::string& icon) {
    if (!toolbar_) toolbar_.reset(new SHToolBar);
    toolbar_->AddButton(id, label, icon);
    peer_->SetToolBar(*toolbar_);
}
void SHFrame::SetStatusText(const std::string& s) { status_ = s; peer_->SetStatusText(s); }
void SHFrame::SetView(std::unique_ptr<SHView> v) {
    if (view_) view_->frame_ = nullptr;
    view_ = std::move(v);
    if (view_) view_->frame_ = this;
    Invalidate();
}
SHPanel* SHFrame::AddPanel(std::unique_ptr<SHPanel> panel) {
    if (!panel) return nullptr;
    panels_.push_back(std::move(panel));
    peer_->AddPanel(*panels_.back());
    return panels_.back().get();
}
void SHFrame::Show(bool show) { peer_->Show(show); }
void SHFrame::Invalidate() { peer_->Invalidate(); }
bool SHFrame::Dispatch(int id) {
    auto it = handlers_.find(id);
    if (it == handlers_.end() || !it->second) return false;
    it->second();
    return true;
}
void SHFrame::Paint(SHCanvas& c) { if (view_) view_->OnDraw(c); }

}  // namespace shore
