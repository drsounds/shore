#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace shore {

class SHFeature;
class SHFrame;

class SHApp {
public:
    virtual ~SHApp();
    virtual bool OnInit() = 0;
    virtual void OnExit() {}
    void RegisterFeature(SHFeature* feature);
    void SetBackendProperty(const std::string& name, const std::string& value) { backendProperties_[name] = value; }
    const std::string& BackendProperty(const std::string& name) const;
    const std::map<std::string, std::string>& BackendProperties() const { return backendProperties_; }
    int Run(int argc, char** argv);
    static void Quit();
    static SHApp* Get();
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

#define SH_IMPLEMENT_APP(AppClass) \
    int main(int argc, char** argv) { AppClass app; return app.Run(argc, argv); }

}  // namespace shore
