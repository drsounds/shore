#include <algorithm>
#include <shore/app.h>
#include <shore/feature.h>
#include <shore/frame.h>
#include <stdexcept>

namespace shore {

namespace {
SHApp* g_app = nullptr;
}

SHApp::~SHApp() = default;
SHApp::SHApp(std::unique_ptr<SHBackend> backend) : backend_(std::move(backend)) {
    if (!backend_) throw std::invalid_argument("SHApp requires a non-null backend");
}
void SHApp::SetBackend(std::unique_ptr<SHBackend> backend) {
    if (g_app == this) throw std::logic_error("Cannot change the backend while the app is running");
    if (!backend) throw std::invalid_argument("SHApp requires a non-null backend");
    if (!frames_.empty()) throw std::logic_error("Cannot change the backend after adding frames");
    backend_ = std::move(backend);
}
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
void SHApp::Quit() { if (g_app && g_app->backend_) g_app->backend_->Quit(); }
const char* SHApp::BackendName() {
    return g_app && g_app->backend_ ? g_app->backend_->Name() : "none";
}
std::unique_ptr<SHFramePeer> SHApp::CreateFramePeer(SHFrame* frame) {
    if (!g_app || !g_app->backend_)
        throw std::logic_error("A Shore backend must be selected before creating a frame");
    std::unique_ptr<SHFramePeer> peer = g_app->backend_->CreateFramePeer(frame);
    if (!peer) throw std::runtime_error("The selected Shore backend returned an empty frame peer");
    return peer;
}
SHFrame* SHApp::AddFrame(std::unique_ptr<SHFrame> f) {
    frames_.push_back(std::move(f));
    for (SHFeature* feature : features_) if (feature) feature->Install(*frames_.back());
    return frames_.back().get();
}
int SHApp::Run(int argc, char** argv) {
    if (!backend_) throw std::logic_error("Set a Shore backend before running the app");
    if (g_app) throw std::logic_error("A Shore app is already running");
    g_app = this;
    struct RunCleanup {
        SHApp* app;
        std::vector<std::unique_ptr<SHFrame>>& frames;
        ~RunCleanup() {
            frames.clear();
            if (g_app == app) g_app = nullptr;
        }
    } cleanup = {this, frames_};
    argc_ = argc;
    argv_ = argv;
    int rc = backend_->Run(this, argc, argv);
    OnExit();
    return rc;
}

}  // namespace shore
