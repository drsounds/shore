#include "test_backend.h"
#include <shore/app.h>

namespace {

class TestFramePeer : public shore::SHFramePeer {
public:
    void SetTitle(const std::string&) override {}
    void SetStatusText(const std::string&) override {}
    void AddMenuItem(const std::string&, const std::string&, int) override {}
    void SetToolBar(const shore::SHToolBar&) override {}
    void Show(bool) override {}
    void Invalidate() override {}
};

class TestBackend : public shore::SHBackend {
public:
    std::unique_ptr<shore::SHFramePeer> CreateFramePeer(shore::SHFrame*) override {
        return std::unique_ptr<shore::SHFramePeer>(new TestFramePeer);
    }
    int Run(shore::SHApp* app, int, char**) override {
        return app->OnInit() ? 0 : 1;
    }
    void Quit() override {}
    const char* Name() const override { return "test"; }
};

}  // namespace

std::unique_ptr<shore::SHBackend> MakeTestBackend() {
    return std::unique_ptr<shore::SHBackend>(new TestBackend);
}
