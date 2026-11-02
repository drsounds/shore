#pragma once
#include <vector>

namespace shore {

class SHDocument;
class SHFeature;
class SHFrame;
class SHService;
class SHTool;
class SHView;

class SHFeatureRegistry {
public:
    void RegisterTool(SHTool* tool);
    void RegisterDocument(SHDocument* document);
    void RegisterView(SHView* view);
    void RegisterService(SHService* service);
    const std::vector<SHTool*>& Tools() const { return tools_; }
    const std::vector<SHDocument*>& Documents() const { return documents_; }
    const std::vector<SHView*>& Views() const { return views_; }
    const std::vector<SHService*>& Services() const { return services_; }
    void InstallTools(SHFrame& frame) const;
private:
    std::vector<SHTool*> tools_;
    std::vector<SHDocument*> documents_;
    std::vector<SHView*> views_;
    std::vector<SHService*> services_;
};

}  // namespace shore
