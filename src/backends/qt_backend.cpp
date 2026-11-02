// Qt 6 Widgets backend.
#include <QAction>
#include <QApplication>
#include <QColor>
#include <QIcon>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QStatusBar>
#include <QToolBar>
#include <QWidget>
#include <map>
#include "../backend.h"

namespace shore {
namespace {

QString Q(const std::string& value) { return QString::fromUtf8(value.c_str()); }
QColor C(SHColor color) { return QColor(color.r, color.g, color.b); }

class QtCanvas : public SHCanvas {
public:
    explicit QtCanvas(QPainter& painter, const QSize& size) : painter_(painter), size_(size) {}
    int Width() const override { return size_.width(); }
    int Height() const override { return size_.height(); }
    void FillRect(int x, int y, int w, int h, SHColor color) override {
        painter_.fillRect(x, y, w, h, C(color));
    }
    void DrawLine(int x1, int y1, int x2, int y2, SHColor color) override {
        painter_.setPen(C(color));
        painter_.drawLine(x1, y1, x2, y2);
    }
    void DrawText(int x, int y, const std::string& text, SHColor color) override {
        painter_.setPen(C(color));
        painter_.drawText(x, y, Q(text));
    }
private:
    QPainter& painter_;
    QSize size_;
};

class QtCanvasWidget : public QWidget {
public:
    QtCanvasWidget(SHFrame* frame, QWidget* parent)
        : QWidget(parent), frame_(frame) {
        setAttribute(Qt::WA_OpaquePaintEvent);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.fillRect(rect(), Qt::white);
        QtCanvas canvas(painter, size());
        frame_->Paint(canvas);
    }
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton && frame_->View()) {
            const QPoint position = event->position().toPoint();
            frame_->View()->OnMouseDown(position.x(), position.y());
        }
    }
private:
    SHFrame* frame_;
};

class QtFrame : public QMainWindow {
public:
    explicit QtFrame(SHFrame* frame)
        : canvas_(new QtCanvasWidget(frame, this)) {
        setWindowTitle(Q(frame->Title()));
        resize(frame->Width(), frame->Height());
        setCentralWidget(canvas_);
        statusBar();
    }
    QWidget* canvas_;
};

class QtPeer : public SHFramePeer {
public:
    explicit QtPeer(SHFrame* frame) : frame_(frame), win_(new QtFrame(frame)) {}
    ~QtPeer() override { delete win_; }

    void SetTitle(const std::string& title) override { win_->setWindowTitle(Q(title)); }
    void SetStatusText(const std::string& text) override { win_->statusBar()->showMessage(Q(text)); }
    void AddMenuItem(const std::string& menu, const std::string& label, int id) override {
        QMenu*& menuPtr = menus_[menu];
        if (!menuPtr) menuPtr = win_->menuBar()->addMenu(Q(menu));
        QAction* action = menuPtr->addAction(Q(label));
        QObject::connect(action, &QAction::triggered, [this, id] { frame_->Dispatch(id); });
    }
    void SetToolBar(const SHToolBar& toolbar) override {
        if (!toolbar_) toolbar_ = win_->addToolBar(QString());
        toolbar_->clear();
        toolbar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        for (const auto& item : toolbar.Items()) {
            if (item.separator) {
                toolbar_->addSeparator();
                continue;
            }
            QAction* action = item.icon.empty()
                ? toolbar_->addAction(Q(item.label))
                : toolbar_->addAction(QIcon::fromTheme(Q(item.icon)), Q(item.label));
            QObject::connect(action, &QAction::triggered, [this, id = item.id] { frame_->Dispatch(id); });
        }
    }
    void Show(bool show) override { win_->setVisible(show); }
    void Invalidate() override { win_->canvas_->update(); }

private:
    SHFrame* frame_;
    QtFrame* win_;
    QToolBar* toolbar_ = nullptr;
    std::map<std::string, QMenu*> menus_;
};

std::unique_ptr<QApplication> g_qtApp;
}  // namespace

namespace backend {
std::unique_ptr<SHFramePeer> CreateFramePeer(SHFrame* frame) {
    return std::unique_ptr<SHFramePeer>(new QtPeer(frame));
}
int Run(SHApp* app, int argc, char** argv) {
    g_qtApp.reset(new QApplication(argc, argv));
    return app->OnInit() ? g_qtApp->exec() : 1;
}
void Quit() { if (g_qtApp) g_qtApp->quit(); }
const char* Name() { return "Qt 6"; }
}  // namespace backend
}  // namespace shore
