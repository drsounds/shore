// Shore: a small, backend-agnostic Document/View framework.
// Backends: wxWidgets (Linux), MFC (Windows), Cocoa (macOS), Null (headless/tests).
#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace shore {

class SHApp;
class SHDocument;
class SHView;
class SHFrame;
class SHFramePeer;
class SHFeature;
class SHConcept;
class SHFeatureRegistry;
class SHTool;
class SHService;
class SHDocumentObject;
class SHFileType;
class SHPanel;

struct SHColor { unsigned char r = 0, g = 0, b = 0; };
struct SHRect { int x = 0, y = 0, width = 0, height = 0; };

enum class SHRenderMode { Editing, Preview, Print, Export };
enum class SHDockPosition { Left, Right, Top, Bottom };
enum class SHControlType { Label, Button, TextField, CheckBox, Choice };

// Abstract drawing surface handed to SHView::OnDraw.
class SHCanvas {
public:
    virtual ~SHCanvas() = default;
    virtual int Width() const = 0;
    virtual int Height() const = 0;
    virtual void FillRect(int x, int y, int w, int h, SHColor c) = 0;
    virtual void DrawLine(int x1, int y1, int x2, int y2, SHColor c) = 0;
    virtual void DrawText(int x, int y, const std::string& text, SHColor c) = 0;
};

struct SHProperty {
    std::string name;
    std::string value;
};

class SHPropertyGroup {
public:
    explicit SHPropertyGroup(const std::string& name) : name_(name) {}
    SHPropertyGroup& AddProperty(const std::string& name, const std::string& value);
    const std::string& Name() const { return name_; }
    const std::vector<SHProperty>& Properties() const { return properties_; }
private:
    std::string name_;
    std::vector<SHProperty> properties_;
};

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

class SHFileType {
public:
    SHFileType(const std::string& id, const std::string& name, const std::vector<std::string>& extensions);
    virtual ~SHFileType() = default;
    const std::string& Id() const { return id_; }
    const std::string& Name() const { return name_; }
    const std::vector<std::string>& Extensions() const { return extensions_; }
    bool MatchesPath(const std::string& path) const;
    virtual bool Read(SHDocument& document, const std::string& path) const = 0;
    virtual bool Write(const SHDocument& document, const std::string& path) const = 0;
private:
    std::string id_;
    std::string name_;
    std::vector<std::string> extensions_;
};

class SHFileTypeRegistry {
public:
    static void Register(SHFileType& type);
    static SHFileType* Find(const std::string& path);
    static const std::vector<SHFileType*>& All();
};

// Holds the data; notifies attached views when it changes.
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
    // Override to read/write contents. Default: no-op success.
    virtual bool Load(const std::string&) { return true; }
    virtual bool Store(const std::string&) { return true; }

private:
    std::string title_ = "Untitled", path_;
    bool modified_ = false;
    std::vector<SHView*> views_;
    std::vector<std::unique_ptr<SHDocumentObject>> objects_;
};

class SHDocumentRenderer {
public:
    void Render(SHDocument& document, SHCanvas& canvas, SHRenderMode mode) const;
};

class SHPanel {
public:
    SHPanel(const std::string& id, const std::string& title, SHDockPosition position = SHDockPosition::Left)
        : id_(id), title_(title), position_(position) {}
    virtual ~SHPanel() = default;
    const std::string& Id() const { return id_; }
    const std::string& Title() const { return title_; }
    SHDockPosition Position() const { return position_; }
private:
    std::string id_;
    std::string title_;
    SHDockPosition position_;
};

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

struct SHControl {
    int id;
    SHControlType type;
    std::string label;
    std::string value;
};

class SHDialog {
public:
    explicit SHDialog(const std::string& title) : title_(title) {}
    const std::string& Title() const { return title_; }
    SHDialog& AddControl(int id, SHControlType type, const std::string& label, const std::string& value = std::string());
    const std::vector<SHControl>& Controls() const { return controls_; }
private:
    std::string title_;
    std::vector<SHControl> controls_;
};

class SHTool {
public:
    SHTool(const std::string& id, const std::string& name, int commandId = 0,
           const std::string& icon = std::string(), const std::string& menu = "Tools")
        : id_(id), name_(name), commandId_(commandId), icon_(icon), menu_(menu) {}
    virtual ~SHTool() = default;
    const std::string& Id() const { return id_; }
    const std::string& Name() const { return name_; }
    int CommandId() const { return commandId_; }
    const std::string& Icon() const { return icon_; }
    const std::string& Menu() const { return menu_; }
    void Install(SHFrame& frame);
    virtual void OnActivate() {}
    virtual void OnFocus() {}
    virtual void OnBlur() {}
    virtual void OnMove(int x, int y) { (void)x; (void)y; }
    virtual void OnResize(int width, int height) { (void)width; (void)height; }
private:
    std::string id_;
    std::string name_;
    int commandId_;
    std::string icon_;
    std::string menu_;
};

class SHService {
public:
    explicit SHService(const std::string& id) : id_(id) {}
    virtual ~SHService() = default;
    const std::string& Id() const { return id_; }
    virtual bool OnStart() { return true; }
    virtual void OnStop() {}
private:
    std::string id_;
};

class SHFeatureRegistry {
public:
    void RegisterTool(SHTool* tool);
    void RegisterDocument(SHDocument* document);
    void RegisterView(SHView* view);
    void RegisterService(SHService* service);
    const std::vector<SHTool*>& Tools() const { return tools_; }
    const std::vector<SHDocument*>& Documents() const { return documents_; }
    const std::vector<SHView*>& Views() const { return views_; }
    const std::vector<SHService*>& Services() const { return services_; }
    void InstallTools(SHFrame& frame) const;
private:
    std::vector<SHTool*> tools_;
    std::vector<SHDocument*> documents_;
    std::vector<SHView*> views_;
    std::vector<SHService*> services_;
};

class SHFeature : public SHFeatureRegistry {
public:
    SHFeature(const std::string& id, const std::string& name, SHFeature* parent = nullptr)
        : id_(id), name_(name), parent_(nullptr) { if (parent) parent->AddFeature(this); }
    const std::string& Id() const { return id_; }
    const std::string& Name() const { return name_; }
    SHFeature* Parent() const { return parent_; }
    void AddFeature(SHFeature* feature);
    void AddConcept(SHConcept* concept);
    void Install(SHFrame& frame) const;
    const std::vector<SHFeature*>& Features() const { return features_; }
    const std::vector<SHConcept*>& Concepts() const { return concepts_; }
private:
    std::string id_;
    std::string name_;
    SHFeature* parent_;
    std::vector<SHFeature*> features_;
    std::vector<SHConcept*> concepts_;
};

class SHConcept : public SHFeatureRegistry {
public:
    SHConcept(const std::string& id, const std::string& name, SHFeature* parent = nullptr)
        : id_(id), name_(name), parent_(nullptr) { if (parent) parent->AddConcept(this); }
    const std::string& Id() const { return id_; }
    const std::string& Name() const { return name_; }
    SHFeature* Parent() const { return parent_; }
    void Install(SHFrame& frame) const { InstallTools(frame); }
private:
    friend class SHFeature;
    std::string id_;
    std::string name_;
    SHFeature* parent_;
};

// Presents a document; draws itself on a canvas.
class SHView {
public:
    virtual ~SHView();
    SHDocument* Document() const { return doc_; }
    void SetDocument(SHDocument* d);
    SHFrame* Frame() const { return frame_; }
    void Invalidate();

    virtual void OnDraw(SHCanvas& canvas) = 0;
    virtual void OnUpdate(SHView* sender, void* hint) { (void)sender; (void)hint; Invalidate(); }
    virtual void OnMouseDown(int x, int y) { (void)x; (void)y; }

private:
    friend class SHFrame;
    SHDocument* doc_ = nullptr;
    SHFrame* frame_ = nullptr;
};

// Ordered list of buttons; each button triggers a command id on the frame.
class SHToolBar {
public:
    struct Item { int id; std::string label; bool separator; std::string icon; };
    SHToolBar& AddButton(int id, const std::string& label, const std::string& icon = std::string()) {
        items_.push_back({id, label, false, icon}); return *this;
    }
    SHToolBar& AddSeparator() { items_.push_back({0, "", true, ""}); return *this; }
    const std::vector<Item>& Items() const { return items_; }
private:
    std::vector<Item> items_;
};

// Top-level window with menu bar, toolbar, status bar and one view.
class SHFrame {
public:
    using Handler = std::function<void()>;
    explicit SHFrame(const std::string& title, int width = 640, int height = 480);
    ~SHFrame();
    SHFrame(const SHFrame&) = delete;
    SHFrame& operator=(const SHFrame&) = delete;

    void SetTitle(const std::string& t);
    const std::string& Title() const { return title_; }
    int Width() const { return width_; }
    int Height() const { return height_; }

    // Registers a command and adds a menu item for it.
    void AddMenuItem(const std::string& menu, const std::string& label, int id, Handler h);
    // Registers a command without a menu item (e.g. toolbar only).
    void AddCommand(int id, Handler h) { handlers_[id] = std::move(h); }
    void SetToolBar(std::unique_ptr<SHToolBar> tb);
    void AddToolBarButton(int id, const std::string& label, const std::string& icon = std::string());
    SHToolBar* ToolBar() const { return toolbar_.get(); }
    void SetStatusText(const std::string& s);
    const std::string& StatusText() const { return status_; }
    // The frame does not own the view's document; it owns the view.
    void SetView(std::unique_ptr<SHView> v);
    SHView* View() const { return view_.get(); }
    SHPanel* AddPanel(std::unique_ptr<SHPanel> panel);
    const std::vector<std::unique_ptr<SHPanel>>& Panels() const { return panels_; }

    void Show(bool show = true);
    void Invalidate();
    // Runs the handler for a command id; returns false if unknown.
    bool Dispatch(int id);
    // Paints the view onto a canvas (called by backends).
    void Paint(SHCanvas& c);
    SHFramePeer* Peer() const { return peer_.get(); }

    struct MenuEntry { std::string menu, label; int id; };
    const std::vector<MenuEntry>& Menus() const { return menus_; }

private:
    std::string title_, status_;
    int width_, height_;
    std::map<int, Handler> handlers_;
    std::vector<MenuEntry> menus_;
    std::unique_ptr<SHToolBar> toolbar_;
    std::unique_ptr<SHView> view_;
    std::vector<std::unique_ptr<SHPanel>> panels_;
    std::unique_ptr<SHFramePeer> peer_;
};

// Application object: subclass and implement OnInit().
class SHApp {
public:
    virtual ~SHApp() = default;
    virtual bool OnInit() = 0;
    virtual void OnExit() {}
    void RegisterFeature(SHFeature* feature);
    void SetBackendProperty(const std::string& name, const std::string& value) { backendProperties_[name] = value; }
    const std::string& BackendProperty(const std::string& name) const;
    const std::map<std::string, std::string>& BackendProperties() const { return backendProperties_; }
    int Run(int argc, char** argv);
    static void Quit();
    static SHApp* Get();
    // Takes ownership of a frame so it lives for the app's duration.
    SHFrame* AddFrame(std::unique_ptr<SHFrame> f);
    int ArgC() const { return argc_; }
    char** ArgV() const { return argv_; }
    static const char* BackendName();
private:
    std::vector<std::unique_ptr<SHFrame>> frames_;
    std::vector<SHFeature*> features_;
    std::map<std::string, std::string> backendProperties_;
    int argc_ = 0;
    char** argv_ = nullptr;
};

// Entry point; defines main() (or WinMain-compatible main on MSVC/MFC).
#define SH_IMPLEMENT_APP(AppClass) \
    int main(int argc, char** argv) { AppClass app; return app.Run(argc, argv); }

}  // namespace shore
