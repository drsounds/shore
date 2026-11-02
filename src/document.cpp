#include <algorithm>
#include <shore/document.h>
#include <shore/view.h>

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

}  // namespace shore
