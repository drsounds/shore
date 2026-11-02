#pragma once
#include <string>
#include <vector>

namespace shore {

class SHToolBar {
public:
    struct Item { int id; std::string label; bool separator; std::string icon; };
    SHToolBar& AddButton(int id, const std::string& label, const std::string& icon = std::string()) {
        items_.push_back({id, label, false, icon}); return *this;
    }
    SHToolBar& AddSeparator() { items_.push_back({0, "", true, ""}); return *this; }
    const std::vector<Item>& Items() const { return items_; }
private:
    std::vector<Item> items_;
};

}  // namespace shore
