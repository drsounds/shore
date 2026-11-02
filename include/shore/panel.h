#pragma once
#include <string>
#include "types.h"

namespace shore {

class SHPanel {
public:
    SHPanel(const std::string& id, const std::string& title, SHDockPosition position = SHDockPosition::Left)
        : id_(id), title_(title), position_(position) {}
    virtual ~SHPanel() = default;
    const std::string& Id() const { return id_; }
    const std::string& Title() const { return title_; }
    SHDockPosition Position() const { return position_; }
private:
    std::string id_;
    std::string title_;
    SHDockPosition position_;
};

}  // namespace shore
