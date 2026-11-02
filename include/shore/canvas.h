#pragma once
#include <string>
#include "types.h"

namespace shore {

class SHCanvas {
public:
    virtual ~SHCanvas() = default;
    virtual int Width() const = 0;
    virtual int Height() const = 0;
    virtual void FillRect(int x, int y, int w, int h, SHColor c) = 0;
    virtual void DrawLine(int x1, int y1, int x2, int y2, SHColor c) = 0;
    virtual void DrawText(int x, int y, const std::string& text, SHColor c) = 0;
};

}  // namespace shore
