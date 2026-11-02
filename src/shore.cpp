#include <algorithm>
#include <fstream>
#include <sstream>
#include "backend.h"

namespace shore {

void SHDocument::SetTitle(const std::string& t) { title_ = t; UpdateAllViews(); }
bool SHDocument::OnOpenDocument(const std::string& p) { return Load(p); }
bool SHDocument::OnSaveDocument(const std::string& p) { return Store(p); }

bool SHDocument::Open(const std::string& path) {
    if (!OnOpenDocument(path)) return false;
    path_ = path;
    modified_ = false;
    size_t s = path.find_last_of("/\\");
    title_ = s == std::string::npos ? path : path.substr(s + 1);
    UpdateAllViews();
    return true;
}

bool SHDocument::Save(const std::string& path) {
    const std::string& target = path.empty() ? path_ : path;
    if (target.empty() || !OnSaveDocument(target)) return false;
    path_ = target;
    modified_ = false;
    return true;
}

void SHDocument::AddView(SHView* v) {
    if (std::find(views_.begin(), views_.end(), v) == views_.end()) views_.push_back(v);
}
void SHDocument::RemoveView(SHView* v) { views_.erase(std::remove(views_.begin(), views_.end(), v), views_.end()); }
void SHDocument::UpdateAllViews(SHView* sender, void* hint) {
    auto copy = views_;
    for (SHView* v : copy) if (v != sender) v->OnUpdate(sender, hint);
}

SHView::~SHView() { if (doc_) doc_->RemoveView(this); }
void SHView::SetDocument(SHDocument* d) {
    if (doc_) doc_->RemoveView(this);
    doc_ = d;
    if (doc_) doc_->AddView(this);
    Invalidate();
}
void SHView::Invalidate() { if (frame_) frame_->Invalidate(); }

SHFrame::SHFrame(const std::string& title, int w, int h)
    : title_(title), width_(w), height_(h), peer_(backend::CreateFramePeer(this)) {}
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
void SHFrame::SetStatusText(const std::string& s) { status_ = s; peer_->SetStatusText(s); }
void SHFrame::SetView(std::unique_ptr<SHView> v) {
    if (view_) view_->frame_ = nullptr;
    view_ = std::move(v);
    if (view_) view_->frame_ = this;
    Invalidate();
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

static SHApp* g_app = nullptr;
SHApp* SHApp::Get() { return g_app; }
void SHApp::Quit() { backend::Quit(); }
const char* SHApp::BackendName() { return backend::Name(); }
SHFrame* SHApp::AddFrame(std::unique_ptr<SHFrame> f) {
    frames_.push_back(std::move(f));
    return frames_.back().get();
}
int SHApp::Run(int argc, char** argv) {
    g_app = this;
    argc_ = argc;
    argv_ = argv;
    int rc = backend::Run(this, argc, argv);
    OnExit();
    frames_.clear();
    g_app = nullptr;
    return rc;
}

}  // namespace shore
