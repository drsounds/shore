#pragma once
#include <string>
#include <vector>
#include "types.h"

namespace shore {

// Drawing surface that backends implement.
//
// Backends must implement the primitives (size, FillRect, DrawLine, DrawText).
// Every other operation has a portable default built on those primitives, so
// a backend may override it with a native version but is never required to.
class SHCanvas {
public:
    virtual ~SHCanvas() = default;

    // Required primitives.
    virtual int Width() const = 0;
    virtual int Height() const = 0;
    virtual void FillRect(int x, int y, int w, int h, SHColor c) = 0;
    virtual void DrawLine(int x1, int y1, int x2, int y2, SHColor c) = 0;
    virtual void DrawText(int x, int y, const std::string& text, SHColor c) = 0;

    // Optional operations with default implementations.
    virtual void DrawRect(int x, int y, int w, int h, SHColor c);
    virtual void DrawEllipse(int x, int y, int w, int h, SHColor c);
    virtual void FillEllipse(int x, int y, int w, int h, SHColor c);
    virtual void DrawPolyline(const std::vector<SHPoint>& points, SHColor c, bool closed = false);
    virtual void FillPolygon(const std::vector<SHPoint>& points, SHColor c);
    // Approximate size of text when drawn with DrawText.
    virtual SHSize MeasureText(const std::string& text) const;

    // Convenience overloads.
    void FillRect(const SHRect& r, SHColor c) { FillRect(r.x, r.y, r.width, r.height, c); }
    void DrawRect(const SHRect& r, SHColor c) { DrawRect(r.x, r.y, r.width, r.height, c); }
    void DrawLine(SHPoint a, SHPoint b, SHColor c) { DrawLine(a.x, a.y, b.x, b.y, c); }
    void DrawText(SHPoint p, const std::string& text, SHColor c) { DrawText(p.x, p.y, text, c); }
};

}  // namespace shore
