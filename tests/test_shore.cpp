#include <cstdio>
#include <cstdlib>
#include <shore/shore.h>
using namespace shore;

#define CHECK(x) do { if (!(x)) { std::printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #x); std::exit(1); } } while (0)

struct Doc : SHDocument {
    std::string data;
    bool Load(const std::string& p) override { data = p; return p != "bad"; }
};
struct View : SHView {
    int updates = 0, draws = 0;
    void OnDraw(SHCanvas&) override { ++draws; }
    void OnUpdate(SHView* s, void* h) override { ++updates; SHView::OnUpdate(s, h); }
};
struct Canvas : SHCanvas {
    int Width() const override { return 1; } int Height() const override { return 1; }
    void FillRect(int, int, int, int, SHColor) override {}
    void DrawLine(int, int, int, int, SHColor) override {}
    void DrawText(int, int, const std::string&, SHColor) override {}
};
struct App : SHApp {
    bool OnInit() override {
        Doc doc;
        SHFrame* f = AddFrame(std::unique_ptr<SHFrame>(new SHFrame("t")));
        std::unique_ptr<View> v(new View);
        View* vp = v.get();
        vp->SetDocument(&doc);
        f->SetView(std::move(v));
        CHECK(vp->Frame() == f && doc.Views().size() == 1);
        doc.UpdateAllViews();
        CHECK(vp->updates == 1);
        Canvas c; f->Paint(c); CHECK(vp->draws == 1);
        CHECK(doc.Open("/x/y.txt") && doc.Title() == "y.txt" && !doc.IsModified());
        CHECK(!doc.Open("bad"));
        int hits = 0;
        f->AddMenuItem("File", "Go", 7, [&] { ++hits; });
        CHECK(f->Dispatch(7) && hits == 1 && !f->Dispatch(99));
        std::unique_ptr<SHToolBar> tb(new SHToolBar);
        tb->AddButton(7, "Go").AddSeparator();
        f->SetToolBar(std::move(tb));
        CHECK(f->ToolBar()->Items().size() == 2);
        f->SetView(nullptr);  // view destroyed -> removed from doc
        CHECK(doc.Views().empty());
        return true;
    }
};
int main(int argc, char** argv) { App a; int rc = a.Run(argc, argv); std::puts(rc == 0 ? "OK" : "FAIL"); return rc; }
