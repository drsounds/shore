// MFC backend (Windows, MSVC). Build with CMake and /D_AFXDLL (see CMakeLists).
#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>
#include <map>
#include "../backend.h"

namespace shore {
namespace {

CString W(const std::string& s) {
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    CStringW w;
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.GetBuffer(n), n);
    w.ReleaseBuffer();
    return CString(w);
}
COLORREF Col(SHColor c) { return RGB(c.r, c.g, c.b); }

class MfcCanvas : public SHCanvas {
public:
    MfcCanvas(CDC& dc, CRect r) : dc_(dc), r_(r) {}
    int Width() const override { return r_.Width(); }
    int Height() const override { return r_.Height(); }
    void FillRect(int x, int y, int w, int h, SHColor c) override { dc_.FillSolidRect(x, y, w, h, Col(c)); }
    void DrawLine(int x1, int y1, int x2, int y2, SHColor c) override {
        CPen pen(PS_SOLID, 1, Col(c));
        CPen* old = dc_.SelectObject(&pen);
        dc_.MoveTo(x1, y1);
        dc_.LineTo(x2, y2);
        dc_.SelectObject(old);
    }
    void DrawText(int x, int y, const std::string& t, SHColor c) override {
        dc_.SetTextColor(Col(c));
        dc_.SetBkMode(TRANSPARENT);
        dc_.TextOut(x, y, W(t));
    }
private:
    CDC& dc_;
    CRect r_;
};

class MfcFrame : public CFrameWnd {
public:
    explicit MfcFrame(SHFrame* f) : frame_(f) {}
    CMenu menuBar;
    CToolBar toolBar;
    CStatusBar statusBar;
    std::map<std::string, CMenu*> menus;
protected:
    afx_msg int OnCreate(LPCREATESTRUCT cs) {
        if (CFrameWnd::OnCreate(cs) == -1) return -1;
        statusBar.Create(this);
        return 0;
    }
    afx_msg void OnPaint() {
        CPaintDC dc(this);
        CRect r;
        GetClientRect(&r);
        MfcCanvas c(dc, r);
        frame_->Paint(c);
    }
    afx_msg BOOL OnEraseBkgnd(CDC* dc) {
        CRect r;
        GetClientRect(&r);
        dc->FillSolidRect(r, RGB(255, 255, 255));
        return TRUE;
    }
    afx_msg void OnLButtonDown(UINT, CPoint p) { if (frame_->View()) frame_->View()->OnMouseDown(p.x, p.y); }
    afx_msg BOOL OnCommand(WPARAM wp, LPARAM lp) {
        UINT id = LOWORD(wp);
        if (id >= kBase && frame_->Dispatch(static_cast<int>(id - kBase))) return TRUE;
        return CFrameWnd::OnCommand(wp, lp);
    }
    afx_msg void OnClose() { CFrameWnd::OnClose(); }
    DECLARE_MESSAGE_MAP()
public:
    static const UINT kBase = 0x8000;
private:
    SHFrame* frame_;
};

BEGIN_MESSAGE_MAP(MfcFrame, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

class MfcPeer : public SHFramePeer {
public:
    explicit MfcPeer(SHFrame* f) : win_(new MfcFrame(f)) {
        win_->Create(nullptr, W(f->Title()), WS_OVERLAPPEDWINDOW, CRect(0, 0, f->Width(), f->Height()));
        win_->menuBar.CreateMenu();
        win_->SetMenu(&win_->menuBar);
    }
    ~MfcPeer() override {
        if (win_ && ::IsWindow(win_->m_hWnd)) win_->DestroyWindow();
        // CFrameWnd deletes itself in PostNcDestroy (auto-delete), so do not delete here.
    }
    void SetTitle(const std::string& t) override { win_->SetWindowText(W(t)); }
    void SetStatusText(const std::string& s) override { win_->statusBar.SetPaneText(0, W(s)); }
    void AddMenuItem(const std::string& menu, const std::string& label, int id) override {
        CMenu*& m = win_->menus[menu];
        if (!m) {
            m = new CMenu;
            m->CreatePopupMenu();
            win_->menuBar.AppendMenu(MF_POPUP, reinterpret_cast<UINT_PTR>(m->m_hMenu), W(menu));
        }
        m->AppendMenu(MF_STRING, MfcFrame::kBase + id, W(label));
        win_->DrawMenuBar();
    }
    void SetToolBar(const SHToolBar& tb) override {
        if (!::IsWindow(win_->toolBar.m_hWnd))
            win_->toolBar.CreateEx(win_, TBSTYLE_FLAT | TBSTYLE_LIST, WS_CHILD | WS_VISIBLE | CBRS_TOP);
        CToolBarCtrl& ctrl = win_->toolBar.GetToolBarCtrl();
        while (ctrl.GetButtonCount() > 0) ctrl.DeleteButton(0);
        for (const auto& it : tb.Items()) {
            CString text = W(it.label);
            text.AppendChar(0);  // double-NUL terminated string for AddStrings
            TBBUTTON b = {};
            if (it.separator) { b.fsStyle = BTNS_SEP; }
            else {
                b.iBitmap = I_IMAGENONE;
                b.idCommand = MfcFrame::kBase + it.id;
                b.fsState = TBSTATE_ENABLED;
                b.fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE | BTNS_SHOWTEXT;
                b.iString = ctrl.AddStrings(text);
            }
            ctrl.AddButtons(1, &b);
        }
        win_->RecalcLayout();
    }
    void Show(bool s) override {
        win_->ShowWindow(s ? SW_SHOW : SW_HIDE);
        if (s) win_->UpdateWindow();
    }
    void Invalidate() override { win_->Invalidate(); }
private:
    MfcFrame* win_;
};

class MfcApp : public CWinApp {
public:
    SHApp* app = nullptr;
    BOOL InitInstance() override {
        if (!app->OnInit()) return FALSE;
        return TRUE;
    }
};
}  // namespace

namespace backend {
std::unique_ptr<SHFramePeer> CreateFramePeer(SHFrame* f) { return std::unique_ptr<SHFramePeer>(new MfcPeer(f)); }
int Run(SHApp* app, int, char**) {
    MfcApp mfc;
    mfc.app = app;
    if (!AfxWinInit(::GetModuleHandle(nullptr), nullptr, ::GetCommandLine(), SW_SHOW)) return 1;
    if (!mfc.InitInstance()) return 1;
    return mfc.Run();
}
void Quit() { ::PostQuitMessage(0); }
const char* Name() { return "MFC"; }
}  // namespace backend
}  // namespace shore
