#include <shore/canvas.h>
#include <algorithm>
#include <cmath>

namespace shore {

void SHCanvas::DrawRect(int x, int y, int w, int h, SHColor c) {
    if (w <= 0 || h <= 0) return;
    FillRect(x, y, w, 1, c);
    if (h > 1) FillRect(x, y + h - 1, w, 1, c);
    if (h > 2) {
        FillRect(x, y + 1, 1, h - 2, c);
        if (w > 1) FillRect(x + w - 1, y + 1, 1, h - 2, c);
    }
}

void SHCanvas::DrawPolyline(const std::vector<SHPoint>& pts, SHColor c, bool closed) {
    if (pts.size() < 2) return;
    for (size_t i = 0; i + 1 < pts.size(); ++i) DrawLine(pts[i].x, pts[i].y, pts[i + 1].x, pts[i + 1].y, c);
    if (closed && pts.size() > 2) DrawLine(pts.back().x, pts.back().y, pts.front().x, pts.front().y, c);
}

void SHCanvas::DrawEllipse(int x, int y, int w, int h, SHColor c) {
    if (w <= 0 || h <= 0) return;
    const double cx = x + w / 2.0, cy = y + h / 2.0, rx = w / 2.0, ry = h / 2.0;
    const int steps = std::max(16, std::min(w + h, 256));
    std::vector<SHPoint> pts;
    for (int i = 0; i < steps; ++i) {
        double a = 6.283185307179586 * i / steps;
        SHPoint p;
        p.x = static_cast<int>(std::lround(cx + rx * std::cos(a)));
        p.y = static_cast<int>(std::lround(cy + ry * std::sin(a)));
        pts.push_back(p);
    }
    DrawPolyline(pts, c, true);
}

void SHCanvas::FillEllipse(int x, int y, int w, int h, SHColor c) {
    if (w <= 0 || h <= 0) return;
    const double cy = y + h / 2.0, rx = w / 2.0, ry = h / 2.0;
    for (int row = 0; row < h; ++row) {
        double dy = (y + row + 0.5 - cy) / ry;
        double k = 1.0 - dy * dy;
        if (k <= 0) continue;
        int half = static_cast<int>(std::lround(rx * std::sqrt(k)));
        int left = static_cast<int>(std::lround(x + rx)) - half;
        if (half > 0) FillRect(left, y + row, half * 2, 1, c);
    }
}

// Even-odd scanline fill, one pixel row at a time.
void SHCanvas::FillPolygon(const std::vector<SHPoint>& pts, SHColor c) {
    if (pts.size() < 3) return;
    int minY = pts[0].y, maxY = pts[0].y;
    for (const auto& p : pts) { minY = std::min(minY, p.y); maxY = std::max(maxY, p.y); }
    std::vector<double> xs;
    for (int y = minY; y < maxY; ++y) {
        xs.clear();
        const double sy = y + 0.5;
        for (size_t i = 0; i < pts.size(); ++i) {
            const SHPoint& a = pts[i];
            const SHPoint& b = pts[(i + 1) % pts.size()];
            if ((a.y <= sy) == (b.y <= sy)) continue;
            xs.push_back(a.x + (sy - a.y) * (b.x - a.x) / double(b.y - a.y));
        }
        std::sort(xs.begin(), xs.end());
        for (size_t i = 0; i + 1 < xs.size(); i += 2) {
            int x0 = static_cast<int>(std::lround(xs[i])), x1 = static_cast<int>(std::lround(xs[i + 1]));
            if (x1 > x0) FillRect(x0, y, x1 - x0, 1, c);
        }
    }
}

SHSize SHCanvas::MeasureText(const std::string& text) const {
    SHSize s;
    s.width = static_cast<int>(text.size()) * 8;
    s.height = 16;
    return s;
}

}  // namespace shore
