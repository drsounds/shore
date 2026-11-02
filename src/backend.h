// Internal backend interface; each platform implements these.
#pragma once
#include <shore/shore.h>

namespace shore {

// Native window behind an SHFrame.
class SHFramePeer {
public:
    virtual ~SHFramePeer() = default;
    virtual void SetTitle(const std::string& t) = 0;
    virtual void SetStatusText(const std::string& s) = 0;
    virtual void AddMenuItem(const std::string& menu, const std::string& label, int id) = 0;
    virtual void SetToolBar(const SHToolBar& tb) = 0;
    virtual void AddPanel(const SHPanel&) {}
    virtual void Show(bool show) = 0;
    virtual void Invalidate() = 0;
};

namespace backend {
std::unique_ptr<SHFramePeer> CreateFramePeer(SHFrame* frame);
// Initialise toolkit, run OnInit and the event loop; return exit code.
int Run(SHApp* app, int argc, char** argv);
void Quit();
const char* Name();
}  // namespace backend
}  // namespace shore
