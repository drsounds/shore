#include <shore/frame.h>
#include <shore/tool.h>

namespace shore {

void SHTool::Install(SHFrame& frame) {
    frame.AddMenuItem(menu_, name_, commandId_, [this] { OnActivate(); });
    if (!icon_.empty()) frame.AddToolBarButton(commandId_, name_, icon_);
}

}  // namespace shore
