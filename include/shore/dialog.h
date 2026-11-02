#pragma once
#include <string>
#include <vector>
#include "types.h"

namespace shore {

class SHDialog {
public:
    explicit SHDialog(const std::string& title) : title_(title) {}
    const std::string& Title() const { return title_; }
    SHDialog& AddControl(int id, SHControlType type, const std::string& label, const std::string& value = std::string());
    const std::vector<SHControl>& Controls() const { return controls_; }
private:
    std::string title_;
    std::vector<SHControl> controls_;
};

}  // namespace shore
