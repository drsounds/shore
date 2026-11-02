#include <shore/file_type_registry.h>

namespace shore {
namespace {
std::vector<SHFileType*>& FileTypes() {
    static std::vector<SHFileType*> types;
    return types;
}
}  // namespace

void SHFileTypeRegistry::Register(SHFileType& type) {
    auto& types = FileTypes();
    for (SHFileType*& registered : types) {
        if (registered->Id() == type.Id()) {
            registered = &type;
            return;
        }
    }
    types.push_back(&type);
}

SHFileType* SHFileTypeRegistry::Find(const std::string& path) {
    for (SHFileType* type : FileTypes()) if (type->MatchesPath(path)) return type;
    return nullptr;
}

const std::vector<SHFileType*>& SHFileTypeRegistry::All() { return FileTypes(); }

}  // namespace shore
