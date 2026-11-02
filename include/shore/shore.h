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

struct SHColor { unsigned char r = 0, g = 0, b = 0; };

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
    bool Save(const std::string& path = std::string());

    void AddView(SHView* v);
    void RemoveView(SHView* v);
    const std::vector<SHView*>& Views() const { return views_; }
    void UpdateAllViews(SHView* sender = nullptr, void* hint = nullptr);

protected:
    // Override to read/write contents. Default: no-op success.
    virtual bool Load(const std::string&) { return true; }
    virtual bool Store(const std::string&) { return true; }

private:
    std::string title_ = "Untitled", path_;
    bool modified_ = false;
    std::vector<SHView*> views_;
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
    struct Item { int id; std::string label; bool separator; };
    SHToolBar& AddButton(int id, const std::string& label) { items_.push_back({id, label, false}); return *this; }
    SHToolBar& AddSeparator() { items_.push_back({0, "", true}); return *this; }
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
    SHToolBar* ToolBar() const { return toolbar_.get(); }
    void SetStatusText(const std::string& s);
    const std::string& StatusText() const { return status_; }
    // The frame does not own the view's document; it owns the view.
    void SetView(std::unique_ptr<SHView> v);
    SHView* View() const { return view_.get(); }

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
    std::unique_ptr<SHFramePeer> peer_;
};

// Application object: subclass and implement OnInit().
class SHApp {
public:
    virtual ~SHApp() = default;
    virtual bool OnInit() = 0;
    virtual void OnExit() {}
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
    int argc_ = 0;
    char** argv_ = nullptr;
};

// Entry point; defines main() (or WinMain-compatible main on MSVC/MFC).
#define SH_IMPLEMENT_APP(AppClass) \
    int main(int argc, char** argv) { AppClass app; return app.Run(argc, argv); }

}  // namespace shore
