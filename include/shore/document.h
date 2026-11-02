#pragma once
#include <memory>
#include <string>
#include <vector>
#include "document_object.h"
#include "file_type.h"

namespace shore {

class SHView;

class SHDocument {
public:
    virtual ~SHDocument() = default;
    const std::string& Title() const { return title_; }
    void SetTitle(const std::string& t);
    const std::string& Path() const { return path_; }
    bool IsModified() const { return modified_; }
    void SetModified(bool m = true) { modified_ = m; }

    virtual bool OnNewDocument() { return true; }
    virtual bool OnOpenDocument(const std::string& path);
    virtual bool OnSaveDocument(const std::string& path);
    bool Open(const std::string& path);
    bool Open(const std::string& path, const SHFileType& type);
    bool Save(const std::string& path = std::string());
    bool Save(const std::string& path, const SHFileType& type);

    void AddView(SHView* v);
    void RemoveView(SHView* v);
    const std::vector<SHView*>& Views() const { return views_; }
    void UpdateAllViews(SHView* sender = nullptr, void* hint = nullptr);
    SHDocumentObject* AddObject(std::unique_ptr<SHDocumentObject> object);
    const std::vector<std::unique_ptr<SHDocumentObject>>& Objects() const { return objects_; }
    virtual void OnRendering(SHCanvas& canvas, SHRenderMode mode) { (void)canvas; (void)mode; }

protected:
    virtual bool Load(const std::string&) { return true; }
    virtual bool Store(const std::string&) { return true; }

private:
    std::string title_ = "Untitled", path_;
    bool modified_ = false;
    std::vector<SHView*> views_;
    std::vector<std::unique_ptr<SHDocumentObject>> objects_;
};

}  // namespace shore
