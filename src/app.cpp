#include <algorithm>
#include <shore/app.h>
#include <shore/feature.h>
#include <shore/frame.h>
#include "backend.h"

namespace shore {

namespace {
SHApp* g_app = nullptr;
}

SHApp::~SHApp() = default;
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
