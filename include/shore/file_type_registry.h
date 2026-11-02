#pragma once
#include <vector>
#include "file_type.h"

namespace shore {

class SHFileTypeRegistry {
public:
    static void Register(SHFileType& type);
    static SHFileType* Find(const std::string& path);
    static const std::vector<SHFileType*>& All();
};

}  // namespace shore
