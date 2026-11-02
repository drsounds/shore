// WTL backend (Windows, MSVC).
#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <commctrl.h>
#include <windowsx.h>
#include <map>
#include "../backend.h"

WTL::CAppModule _Module;

namespace shore {
namespace {
using namespace WTL;

std::wstring W(const std::string& value) {
    int length = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
    if (length <= 0) return std::wstring();
    std::wstring result(static_cast<std::size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, &result[0], length);
    result.resize(static_cast<std::size_t>(length - 1));
    return result;
}

class WtlCanvas : public SHCanvas {
public:
    WtlCanvas(HDC dc, const RECT& rect) : dc_(dc), rect_(rect) {}
    int Width() const override { return rect_.right - rect_.left; }
    int Height() const override { return rect_.bottom - rect_.top; }
    void FillRect(int x, int y, int w, int h, SHColor c) override {
        RECT r = {x, y, x + w, y + h};
        HBRUSH brush = CreateSolidBrush(RGB(c.r, c.g, c.b));
        ::FillRect(dc_, &r, brush);
        DeleteObject(brush);
    }
    void DrawLine(int x1, int y1, int x2, int y2, SHColor c) override {
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(c.r, c.g, c.b));
        HGDIOBJ old = SelectObject(dc_, pen);
        MoveToEx(dc_, x1, y1, nullptr);
        LineTo(dc_, x2, y2);
        SelectObject(dc_, old);
        DeleteObject(pen);
    }
    void DrawText(int x, int y, const std::string& text, SHColor c) override {
        std::wstring wide = W(text);
        SetTextColor(dc_, RGB(c.r, c.g, c.b));
        SetBkMode(dc_, TRANSPARENT);
        TextOutW(dc_, x, y, wide.c_str(), static_cast<int>(wide.size()));
    }
private:
    HDC dc_;
    RECT rect_;
};

class WtlFrame;

class WtlCanvasWindow : public CWindowImpl<WtlCanvasWindow> {
public:
    DECLARE_WND_CLASS(_T("ShoreWtlCanvas"))
    explicit WtlCanvasWindow(WtlFrame* frame) : frame_(frame) {}

    BEGIN_MSG_MAP(WtlCanvasWindow)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
    END_MSG_MAP()

    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&) { return 1; }
    LRESULT OnLButtonDown(UINT, WPARAM, LPARAM lParam, BOOL&);
private:
    WtlFrame* frame_;
};

class WtlFrame : public CFrameWindowImpl<WtlFrame> {
public:
    DECLARE_FRAME_WND_CLASS(_T("ShoreWtlFrame"), 0)
    explicit WtlFrame(SHFrame* frame) : frame_(frame), canvas_(this) {}
    ~WtlFrame() { if (menuBar_) ::DestroyMenu(menuBar_); }

    BEGIN_MSG_MAP(WtlFrame)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_COMMAND, OnCommand)
        CHAIN_MSG_MAP(CFrameWindowImpl<WtlFrame>)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
        menuBar_ = ::CreateMenu();
        if (!menuBar_ || !::SetMenu(m_hWnd, menuBar_)) return -1;

        RECT rect = {0, 0, 0, 0};
        if (!toolbar_.Create(m_hWnd, rect, nullptr,
                WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | CCS_TOP | TBSTYLE_FLAT | TBSTYLE_LIST,
                0, 1001))
            return -1;
        toolbar_.SendMessage(TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
        if (!statusBar_.Create(m_hWnd, rect, nullptr,
                WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_SIZEGRIP, 0, 1002))
            return -1;
        int parts[] = {-1};
        statusBar_.SetParts(1, parts);
        if (!canvas_.Create(m_hWnd, rect, nullptr,
                WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 1003))
            return -1;
        Layout();
        return 0;
    }
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&) { Layout(); return 0; }
    LRESULT OnCommand(UINT, WPARAM wParam, LPARAM, BOOL& handled) {
        UINT id = LOWORD(wParam);
        if (id >= kCommandBase && frame_->Dispatch(static_cast<int>(id - kCommandBase))) {
            handled = TRUE;
            return 0;
        }
        handled = FALSE;
        return 0;
    }

    void SetStatusText(const std::string& text) {
        std::wstring wide = W(text);
        ::SendMessageW(statusBar_.m_hWnd, SB_SETTEXTW, 0, reinterpret_cast<LPARAM>(wide.c_str()));
    }
    void AddMenuItem(const std::string& menu, const std::string& label, int id) {
        HMENU& popup = menus_[menu];
        if (!popup) {
            popup = ::CreatePopupMenu();
            if (!popup) return;
            std::wstring menuText = W(menu);
            ::AppendMenuW(menuBar_, MF_POPUP, reinterpret_cast<UINT_PTR>(popup), menuText.c_str());
        }
        std::wstring itemText = W(label);
        ::AppendMenuW(popup, MF_STRING, kCommandBase + id, itemText.c_str());
        DrawMenuBar();
    }
    void SetToolBar(const SHToolBar& toolbar) {
        while (toolbar_.GetButtonCount() > 0) toolbar_.DeleteButton(0);
        for (const auto& item : toolbar.Items()) {
            TBBUTTON button = {};
            if (item.separator) {
                button.fsStyle = BTNS_SEP;
            } else {
                std::wstring label = W(item.label);
                button.iBitmap = I_IMAGENONE;
                button.idCommand = kCommandBase + item.id;
                button.fsState = TBSTATE_ENABLED;
                button.fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE | BTNS_SHOWTEXT;
                button.iString = static_cast<INT_PTR>(
                    ::SendMessageW(toolbar_.m_hWnd, TB_ADDSTRINGW, 0, reinterpret_cast<LPARAM>(label.c_str())));
            }
            toolbar_.AddButtons(1, &button);
        }
        toolbar_.AutoSize();
        Layout();
    }
    void InvalidateCanvas() { canvas_.Invalidate(); }
    void Layout() {
        if (!::IsWindow(m_hWnd) || !::IsWindow(toolbar_.m_hWnd) ||
            !::IsWindow(statusBar_.m_hWnd) || !::IsWindow(canvas_.m_hWnd))
            return;
        RECT client;
        GetClientRect(&client);
        const int toolbarHeight = 30;
        const int statusHeight = 22;
        int width = client.right - client.left;
        int height = client.bottom - client.top;
        toolbar_.SetWindowPos(nullptr, 0, 0, width, toolbarHeight, SWP_NOZORDER | SWP_NOACTIVATE);
        statusBar_.SetWindowPos(nullptr, 0, height - statusHeight, width, statusHeight,
                                SWP_NOZORDER | SWP_NOACTIVATE);
        canvas_.SetWindowPos(nullptr, 0, toolbarHeight, width, height - toolbarHeight - statusHeight,
                             SWP_NOZORDER | SWP_NOACTIVATE);
    }
    static const UINT kCommandBase = 0x8000;
private:
    SHFrame* frame_;
    HMENU menuBar_ = nullptr;
    CToolBarCtrl toolbar_;
    CStatusBarCtrl statusBar_;
    WtlCanvasWindow canvas_;
    std::map<std::string, HMENU> menus_;
};

LRESULT WtlCanvasWindow::OnPaint(UINT, WPARAM, LPARAM, BOOL&) {
    PAINTSTRUCT paint;
    HDC dc = BeginPaint(&paint);
    RECT rect;
    GetClientRect(&rect);
    HBRUSH background = CreateSolidBrush(RGB(255, 255, 255));
    ::FillRect(dc, &rect, background);
    DeleteObject(background);
    WtlCanvas canvas(dc, rect);
    frame_->Paint(canvas);
    EndPaint(&paint);
    return 0;
}

LRESULT WtlCanvasWindow::OnLButtonDown(UINT, WPARAM, LPARAM lParam, BOOL&) {
    if (frame_->View()) frame_->View()->OnMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    return 0;
}

class WtlPeer : public SHFramePeer {
public:
    explicit WtlPeer(SHFrame* frame) : win_(new WtlFrame(frame)) {
        RECT rect = {0, 0, frame->Width(), frame->Height()};
        win_->Create(nullptr, rect, W(frame->Title()).c_str(), WS_OVERLAPPEDWINDOW);
    }
    ~WtlPeer() override {
        if (::IsWindow(win_->m_hWnd)) win_->DestroyWindow();
        delete win_;
    }
    void SetTitle(const std::string& title) override { ::SetWindowTextW(win_->m_hWnd, W(title).c_str()); }
    void SetStatusText(const std::string& text) override { win_->SetStatusText(text); }
    void AddMenuItem(const std::string& menu, const std::string& label, int id) override {
        win_->AddMenuItem(menu, label, id);
    }
    void SetToolBar(const SHToolBar& toolbar) override { win_->SetToolBar(toolbar); }
    void Show(bool show) override { win_->ShowWindow(show ? SW_SHOW : SW_HIDE); }
    void Invalidate() override { win_->InvalidateCanvas(); }
private:
    WtlFrame* win_;
};
}  // namespace

namespace backend {
std::unique_ptr<SHFramePeer> CreateFramePeer(SHFrame* frame) {
    return std::unique_ptr<SHFramePeer>(new WtlPeer(frame));
}
int Run(SHApp* app, int, char**) {
    INITCOMMONCONTROLSEX controls = {sizeof(controls), ICC_BAR_CLASSES};
    InitCommonControlsEx(&controls);
    if (FAILED(_Module.Init(nullptr, GetModuleHandle(nullptr)))) return 1;
    WTL::CMessageLoop loop;
    _Module.AddMessageLoop(&loop);
    int result = app->OnInit() ? loop.Run() : 1;
    _Module.RemoveMessageLoop();
    _Module.Term();
    return result;
}
void Quit() { PostQuitMessage(0); }
const char* Name() { return "WTL"; }
}  // namespace backend
}  // namespace shore
