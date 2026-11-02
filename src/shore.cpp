#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include "backend.h"

namespace shore {

SHPropertyGroup& SHPropertyGroup::AddProperty(const std::string& name, const std::string& value) {
    properties_.push_back({name, value});
    return *this;
}

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

SHFileType::SHFileType(const std::string& id, const std::string& name, const std::vector<std::string>& extensions)
    : id_(id), name_(name), extensions_(extensions) {}

bool SHFileType::MatchesPath(const std::string& path) const {
    const size_t slash = path.find_last_of("/\\");
    const size_t dot = path.find_last_of('.');
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) return false;
    std::string extension = path.substr(dot);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    for (std::string candidate : extensions_) {
        if (candidate.empty()) continue;
        if (candidate[0] != '.') candidate.insert(candidate.begin(), '.');
        std::transform(candidate.begin(), candidate.end(), candidate.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (extension == candidate) return true;
    }
    return false;
}

namespace {
std::vector<SHFileType*>& FileTypes() {
    static std::vector<SHFileType*> types;
    return types;
}

template <typename T>
void RegisterUnique(std::vector<T*>& values, T* value) {
    if (value && std::find(values.begin(), values.end(), value) == values.end()) values.push_back(value);
}

void RenderObjects(const std::vector<std::unique_ptr<SHDocumentObject>>& objects,
                   SHCanvas& canvas, SHRenderMode mode) {
    for (const auto& object : objects) {
        object->OnRendering(canvas, mode);
        RenderObjects(object->Children(), canvas, mode);
    }
}
}  // namespace

void SHFileTypeRegistry::Register(SHFileType& type) {
    auto& types = FileTypes();
    for (SHFileType*& registered : types) {
        if (registered->Id() == type.Id()) {
            registered = &type;
            return;
        }
    }
    types.push_back(&type);
}

SHFileType* SHFileTypeRegistry::Find(const std::string& path) {
    for (SHFileType* type : FileTypes()) if (type->MatchesPath(path)) return type;
    return nullptr;
}

const std::vector<SHFileType*>& SHFileTypeRegistry::All() { return FileTypes(); }

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

bool SHDocument::Open(const std::string& path, const SHFileType& type) {
    if (!type.Read(*this, path)) return false;
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

bool SHDocument::Save(const std::string& path, const SHFileType& type) {
    const std::string& target = path.empty() ? path_ : path;
    if (target.empty() || !type.Write(*this, target)) return false;
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

SHDocumentObject* SHDocument::AddObject(std::unique_ptr<SHDocumentObject> object) {
    if (!object) return nullptr;
    objects_.push_back(std::move(object));
    return objects_.back().get();
}

void SHDocumentRenderer::Render(SHDocument& document, SHCanvas& canvas, SHRenderMode mode) const {
    document.OnRendering(canvas, mode);
    RenderObjects(document.Objects(), canvas, mode);
}

const std::vector<SHPropertyGroup>& SHPropertyPanel::Groups() const {
    static const std::vector<SHPropertyGroup> empty;
    return object_ ? object_->PropertyGroups() : empty;
}

SHDialog& SHDialog::AddControl(int id, SHControlType type, const std::string& label, const std::string& value) {
    controls_.push_back({id, type, label, value});
    return *this;
}

void SHFeatureRegistry::RegisterTool(SHTool* tool) { RegisterUnique(tools_, tool); }
void SHFeatureRegistry::RegisterDocument(SHDocument* document) { RegisterUnique(documents_, document); }
void SHFeatureRegistry::RegisterView(SHView* view) { RegisterUnique(views_, view); }
void SHFeatureRegistry::RegisterService(SHService* service) { RegisterUnique(services_, service); }
void SHFeatureRegistry::InstallTools(SHFrame& frame) const {
    for (SHTool* tool : tools_) if (tool) tool->Install(frame);
}

void SHTool::Install(SHFrame& frame) {
    frame.AddMenuItem(menu_, name_, commandId_, [this] { OnActivate(); });
    if (!icon_.empty()) frame.AddToolBarButton(commandId_, name_, icon_);
}

void SHFeature::AddFeature(SHFeature* feature) {
    if (!feature || feature == this) return;
    for (SHFeature* ancestor = this; ancestor; ancestor = ancestor->parent_)
        if (ancestor == feature) return;
    if (feature->parent_) {
        auto& siblings = feature->parent_->features_;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), feature), siblings.end());
    }
    feature->parent_ = this;
    RegisterUnique(features_, feature);
}

void SHFeature::AddConcept(SHConcept* concept) {
    if (!concept) return;
    if (concept->parent_) {
        auto& concepts = concept->parent_->concepts_;
        concepts.erase(std::remove(concepts.begin(), concepts.end(), concept), concepts.end());
    }
    concept->parent_ = this;
    RegisterUnique(concepts_, concept);
}

void SHFeature::Install(SHFrame& frame) const {
    InstallTools(frame);
    for (SHConcept* concept : concepts_) if (concept) concept->Install(frame);
    for (SHFeature* feature : features_) if (feature) feature->Install(frame);
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

static SHApp* g_app = nullptr;
SHApp* SHApp::Get() { return g_app; }
const std::string& SHApp::BackendProperty(const std::string& name) const {
    static const std::string empty;
    auto it = backendProperties_.find(name);
    return it == backendProperties_.end() ? empty : it->second;
}
void SHApp::RegisterFeature(SHFeature* feature) {
    if (!feature || std::find(features_.begin(), features_.end(), feature) != features_.end()) return;
    features_.push_back(feature);
    for (const auto& frame : frames_) feature->Install(*frame);
}
void SHApp::Quit() { backend::Quit(); }
const char* SHApp::BackendName() { return backend::Name(); }
SHFrame* SHApp::AddFrame(std::unique_ptr<SHFrame> f) {
    frames_.push_back(std::move(f));
    for (SHFeature* feature : features_) if (feature) feature->Install(*frames_.back());
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
