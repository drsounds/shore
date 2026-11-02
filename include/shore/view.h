#pragma once

namespace shore {

class SHDocument;
class SHFrame;
class SHCanvas;

class SHView {
public:
    virtual ~SHView();
    SHDocument* Document() const { return doc_; }
    void SetDocument(SHDocument* d);
    SHFrame* Frame() const { return frame_; }
    void Invalidate();

    virtual void OnDraw(SHCanvas& canvas) = 0;
    virtual void OnUpdate(SHView* sender, void* hint) { (void)sender; (void)hint; Invalidate(); }
    virtual void OnMouseDown(int x, int y) { (void)x; (void)y; }

private:
    friend class SHFrame;
    SHDocument* doc_ = nullptr;
    SHFrame* frame_ = nullptr;
};

}  // namespace shore
