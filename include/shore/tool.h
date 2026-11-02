#pragma once
#include <string>

namespace shore {

class SHFrame;

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

}  // namespace shore
