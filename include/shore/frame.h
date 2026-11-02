#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "types.h"

namespace shore {

class SHCanvas;
class SHPanel;
class SHView;
class SHToolBar;

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

    void AddMenuItem(const std::string& menu, const std::string& label, int id, Handler h);
    void AddCommand(int id, Handler h) { handlers_[id] = std::move(h); }
    void SetToolBar(std::unique_ptr<SHToolBar> tb);
    void AddToolBarButton(int id, const std::string& label, const std::string& icon = std::string());
    SHToolBar* ToolBar() const { return toolbar_.get(); }
    void SetStatusText(const std::string& s);
    const std::string& StatusText() const { return status_; }
    void SetView(std::unique_ptr<SHView> v);
    SHView* View() const { return view_.get(); }
    SHPanel* AddPanel(std::unique_ptr<SHPanel> panel);
    const std::vector<std::unique_ptr<SHPanel>>& Panels() const { return panels_; }

    void Show(bool show = true);
    void Invalidate();
    bool Dispatch(int id);
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

}  // namespace shore
