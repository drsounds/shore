#pragma once
#include <string>

namespace shore {

class SHService {
public:
    explicit SHService(const std::string& id) : id_(id) {}
    virtual ~SHService() = default;
    const std::string& Id() const { return id_; }
    virtual bool OnStart() { return true; }
    virtual void OnStop() {}
private:
    std::string id_;
};

}  // namespace shore
