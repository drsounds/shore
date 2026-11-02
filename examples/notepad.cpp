// Minimal example: a click counter document with a toolbar and menu.
#include SHORE_BACKEND_HEADER
#include <shore/shore.h>
using namespace shore;

class CounterDoc : public SHDocument {
public:
    int count = 0;
    void Increment() { ++count; SetModified(); UpdateAllViews(); }
    void Reset() { count = 0; SetModified(); UpdateAllViews(); }
};

class CounterView : public SHView {
public:
    void OnDraw(SHCanvas& c) override {
        c.FillRect(10, 10, c.Width() - 20, 40, {230, 240, 255});
        auto* d = static_cast<CounterDoc*>(Document());
        c.DrawText(20, 20, "Count: " + std::to_string(d->count), {0, 0, 0});
        c.DrawLine(10, 60, c.Width() - 10, 60, {128, 128, 128});
    }
    void OnMouseDown(int, int) override { static_cast<CounterDoc*>(Document())->Increment(); }
};

class NotepadApp : public SHApp {
public:
    NotepadApp() : SHApp(SHORE_BACKEND_FACTORY()) {}

    bool OnInit() override {
        doc_.reset(new CounterDoc);
        auto* frame = AddFrame(std::unique_ptr<SHFrame>(new SHFrame("Shore Counter")));
        std::unique_ptr<SHView> view(new CounterView);
        view->SetDocument(doc_.get());
        frame->SetView(std::move(view));
        frame->AddMenuItem("File", "Quit", 1, [] { SHApp::Quit(); });
        frame->AddMenuItem("Edit", "Increment", 2, [this] { doc_->Increment(); });
        frame->AddMenuItem("Edit", "Reset", 3, [this] { doc_->Reset(); });
        std::unique_ptr<SHToolBar> tb(new SHToolBar);
        tb->AddButton(2, "Increment").AddSeparator().AddButton(3, "Reset");
        frame->SetToolBar(std::move(tb));
        frame->SetStatusText(std::string("Backend: ") + SHApp::BackendName());
        frame->Show();
        return true;
    }
private:
    std::unique_ptr<CounterDoc> doc_;
};

SH_IMPLEMENT_APP(NotepadApp)
