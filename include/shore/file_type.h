#pragma once
#include <string>
#include <vector>

namespace shore {

class SHDocument;

class SHFileType {
public:
    SHFileType(const std::string& id, const std::string& name, const std::vector<std::string>& extensions);
    virtual ~SHFileType() = default;
    const std::string& Id() const { return id_; }
    const std::string& Name() const { return name_; }
    const std::vector<std::string>& Extensions() const { return extensions_; }
    bool MatchesPath(const std::string& path) const;
    virtual bool Read(SHDocument& document, const std::string& path) const = 0;
    virtual bool Write(const SHDocument& document, const std::string& path) const = 0;
private:
    std::string id_;
    std::string name_;
    std::vector<std::string> extensions_;
};

}  // namespace shore
