#pragma once

#include <string>

namespace shore {

class SHPanel;
class SHToolBar;

class SHFramePeer {
public:
    virtual ~SHFramePeer() = default;
    virtual void SetTitle(const std::string& title) = 0;
    virtual void SetStatusText(const std::string& text) = 0;
    virtual void AddMenuItem(const std::string& menu, const std::string& label, int id) = 0;
    virtual void SetToolBar(const SHToolBar& toolbar) = 0;
    virtual void AddPanel(const SHPanel&) {}
    virtual void Show(bool show) = 0;
    virtual void Invalidate() = 0;
};

}  // namespace shore
