// wxWidgets backend (Linux/GTK, also usable on other wx platforms).
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <map>
#include "../backend.h"

namespace shore {
namespace {

wxColour C(SHColor c) { return wxColour(c.r, c.g, c.b); }

class WxCanvas : public SHCanvas {
public:
    explicit WxCanvas(wxDC& dc, wxSize s) : dc_(dc), size_(s) {}
    int Width() const override { return size_.x; }
    int Height() const override { return size_.y; }
    void FillRect(int x, int y, int w, int h, SHColor c) override {
        dc_.SetPen(*wxTRANSPARENT_PEN);
        dc_.SetBrush(wxBrush(C(c)));
        dc_.DrawRectangle(x, y, w, h);
    }
    void DrawLine(int x1, int y1, int x2, int y2, SHColor c) override {
        dc_.SetPen(wxPen(C(c)));
        dc_.DrawLine(x1, y1, x2, y2);
    }
    void DrawText(int x, int y, const std::string& t, SHColor c) override {
        dc_.SetTextForeground(C(c));
        dc_.DrawText(wxString::FromUTF8(t.c_str()), x, y);
    }
private:
    wxDC& dc_;
    wxSize size_;
};

class WxPeer;

class WxFrame : public wxFrame {
public:
    WxFrame(SHFrame* f) : wxFrame(nullptr, wxID_ANY, wxString::FromUTF8(f->Title().c_str()), wxDefaultPosition, wxSize(f->Width(), f->Height())), frame_(f) {
        CreateStatusBar();
        canvas_ = new wxPanel(this);
        canvas_->SetBackgroundStyle(wxBG_STYLE_PAINT);
        canvas_->Bind(wxEVT_PAINT, [this](wxPaintEvent&) {
            wxAutoBufferedPaintDC dc(canvas_);
            dc.SetBackground(*wxWHITE_BRUSH);
            dc.Clear();
            WxCanvas c(dc, canvas_->GetClientSize());
            frame_->Paint(c);
        });
        canvas_->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& e) {
            if (frame_->View()) frame_->View()->OnMouseDown(e.GetX(), e.GetY());
        });
        Bind(wxEVT_MENU, [this](wxCommandEvent& e) { frame_->Dispatch(e.GetId() - wxID_HIGHEST); });
        Bind(wxEVT_TOOL, [this](wxCommandEvent& e) { frame_->Dispatch(e.GetId() - wxID_HIGHEST); });
    }
    wxMenuBar* Bar() {
        if (!GetMenuBar()) SetMenuBar(new wxMenuBar);
        return GetMenuBar();
    }
    wxPanel* canvas_;
private:
    SHFrame* frame_;
};

class WxPeer : public SHFramePeer {
public:
    explicit WxPeer(SHFrame* f) : win_(new WxFrame(f)) {}
    ~WxPeer() override { if (win_) win_->Destroy(); }
    void SetTitle(const std::string& t) override { win_->SetTitle(wxString::FromUTF8(t.c_str())); }
    void SetStatusText(const std::string& s) override { win_->SetStatusText(wxString::FromUTF8(s.c_str())); }
    void AddMenuItem(const std::string& menu, const std::string& label, int id) override {
        wxString m = wxString::FromUTF8(menu.c_str());
        wxMenu*& menuPtr = menus_[menu];
        if (!menuPtr) {
            menuPtr = new wxMenu;
            win_->Bar()->Append(menuPtr, m);
        }
        menuPtr->Append(wxID_HIGHEST + id, wxString::FromUTF8(label.c_str()));
    }
    void SetToolBar(const SHToolBar& tb) override {
        wxToolBar* bar = win_->GetToolBar() ? win_->GetToolBar() : win_->CreateToolBar(wxTB_TEXT | wxTB_NOICONS);
        bar->ClearTools();
        for (const auto& it : tb.Items()) {
            if (it.separator) bar->AddSeparator();
            else bar->AddTool(wxID_HIGHEST + it.id, wxString::FromUTF8(it.label.c_str()), wxNullBitmap);
        }
        bar->Realize();
    }
    void Show(bool s) override { win_->Show(s); }
    void Invalidate() override { win_->canvas_->Refresh(); }
private:
    WxFrame* win_;
    std::map<std::string, wxMenu*> menus_;
};

class WxApp : public wxApp {
public:
    SHApp* app = nullptr;
    bool OnInit() override { return app->OnInit(); }
};

WxApp* g_wx = nullptr;
}  // namespace

namespace backend {
std::unique_ptr<SHFramePeer> CreateFramePeer(SHFrame* f) { return std::unique_ptr<SHFramePeer>(new WxPeer(f)); }
int Run(SHApp* app, int argc, char** argv) {
    g_wx = new WxApp;
    g_wx->app = app;
    wxApp::SetInstance(g_wx);
    if (!wxEntryStart(argc, argv)) return 1;
    int rc = 1;
    if (wxTheApp->CallOnInit()) rc = wxTheApp->OnRun();
    wxTheApp->OnExit();
    wxEntryCleanup();
    return rc;
}
void Quit() { if (wxTheApp) wxTheApp->ExitMainLoop(); }
const char* Name() { return "wxWidgets"; }
}  // namespace backend
}  // namespace shore
