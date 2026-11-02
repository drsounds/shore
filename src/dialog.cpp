#include <shore/dialog.h>

namespace shore {

SHDialog& SHDialog::AddControl(int id, SHControlType type, const std::string& label, const std::string& value) {
    controls_.push_back({id, type, label, value});
    return *this;
}

}  // namespace shore
