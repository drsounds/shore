#include <algorithm>
#include <cctype>
#include <shore/file_type.h>

namespace shore {

SHFileType::SHFileType(const std::string& id, const std::string& name,
                       const std::vector<std::string>& extensions)
    : id_(id), name_(name), extensions_(extensions) {}

bool SHFileType::MatchesPath(const std::string& path) const {
    const size_t slash = path.find_last_of("/\\");
    const size_t dot = path.find_last_of('.');
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) return false;
    std::string extension = path.substr(dot);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    for (std::string candidate : extensions_) {
        if (candidate.empty()) continue;
        if (candidate[0] != '.') candidate.insert(candidate.begin(), '.');
        std::transform(candidate.begin(), candidate.end(), candidate.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (extension == candidate) return true;
    }
    return false;
}

}  // namespace shore
