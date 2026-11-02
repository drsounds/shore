#include <algorithm>
#include <shore/feature_registry.h>
#include <shore/frame.h>
#include <shore/tool.h>

namespace shore {
namespace {
template <typename T>
void RegisterUnique(std::vector<T*>& values, T* value) {
    if (value && std::find(values.begin(), values.end(), value) == values.end()) values.push_back(value);
}
}  // namespace

void SHFeatureRegistry::RegisterTool(SHTool* tool) { RegisterUnique(tools_, tool); }
void SHFeatureRegistry::RegisterDocument(SHDocument* document) { RegisterUnique(documents_, document); }
void SHFeatureRegistry::RegisterView(SHView* view) { RegisterUnique(views_, view); }
void SHFeatureRegistry::RegisterService(SHService* service) { RegisterUnique(services_, service); }
void SHFeatureRegistry::InstallTools(SHFrame& frame) const {
    for (SHTool* tool : tools_) if (tool) tool->Install(frame);
}

}  // namespace shore
