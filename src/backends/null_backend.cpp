// Headless backend: no windows; used for tests and servers without a GUI.
#include "../backend.h"

namespace shore {
namespace {
class NullPeer : public SHFramePeer {
public:
    void SetTitle(const std::string&) override {}
    void SetStatusText(const std::string&) override {}
    void AddMenuItem(const std::string&, const std::string&, int) override {}
    void SetToolBar(const SHToolBar&) override {}
    void Show(bool) override {}
    void Invalidate() override {}
};
}  // namespace

namespace backend {
std::unique_ptr<SHFramePeer> CreateFramePeer(SHFrame*) { return std::unique_ptr<SHFramePeer>(new NullPeer); }
int Run(SHApp* app, int, char**) { return app->OnInit() ? 0 : 1; }
void Quit() {}
const char* Name() { return "null"; }
}  // namespace backend
}  // namespace shore
