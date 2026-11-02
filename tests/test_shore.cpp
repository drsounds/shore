#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <shore/shore.h>
#include "test_backend.h"
using namespace shore;

#define CHECK(x) do { if (!(x)) { std::printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #x); std::exit(1); } } while (0)

struct Doc : SHDocument {
    std::string data;
    bool Load(const std::string& p) override { data = p; return p != "bad"; }
};
struct RenderedObject : SHDocumentObject {
    explicit RenderedObject(std::vector<SHRenderMode>& modes) : modes_(modes) {}
    void OnRendering(SHCanvas&, SHRenderMode mode) override { modes_.push_back(mode); }
    std::vector<SHRenderMode>& modes_;
};
struct FileType : SHFileType {
    FileType() : SHFileType("test", "Test", {"txt"}) {}
    bool Read(SHDocument&, const std::string&) const override { ++reads; return true; }
    bool Write(const SHDocument&, const std::string&) const override { ++writes; return true; }
    mutable int reads = 0, writes = 0;
};
struct Tool : SHTool {
    Tool() : SHTool("select", "Select", 12, "select-icon", "Edit") {}
    void OnActivate() override { ++activations; }
    int activations = 0;
};
struct Service : SHService {
    Service() : SHService("history") {}
};
struct View : SHView {
    int updates = 0, draws = 0;
    void OnDraw(SHCanvas&) override { ++draws; }
    void OnUpdate(SHView* s, void* h) override { ++updates; SHView::OnUpdate(s, h); }
};
struct Canvas : SHCanvas {
    int Width() const override { return 1; } int Height() const override { return 1; }
    int fills = 0, lines = 0;
    void FillRect(int, int, int, int, SHColor) override { ++fills; }
    void DrawLine(int, int, int, int, SHColor) override { ++lines; }
    void DrawText(int, int, const std::string&, SHColor) override {}
};
struct App : SHApp {
    App() : SHApp(MakeTestBackend()) {}

    bool OnInit() override {
        CHECK(std::string(SHApp::BackendName()) == "test");
        SetBackendProperty("wx.toolkit", "aui");
        CHECK(BackendProperty("wx.toolkit") == "aui");
        Doc doc;
        static FileType fileType;
        SHFileTypeRegistry::Register(fileType);
        CHECK(SHFileTypeRegistry::Find("notes.TXT") == &fileType);
        CHECK(doc.Open("/tmp/notes.txt", fileType) && fileType.reads == 1);
        CHECK(doc.Save(std::string(), fileType) && fileType.writes == 1);

        std::vector<SHRenderMode> rendered;
        std::unique_ptr<SHDocumentObject> rootObject(new RenderedObject(rendered));
        SHDocumentObject* rootObjectPtr = rootObject.get();
        rootObjectPtr->AddPropertyGroup("Appearance").AddProperty("Color", "blue");
        SHDocumentObject* childObject = rootObjectPtr->AddChild(std::unique_ptr<SHDocumentObject>(new RenderedObject(rendered)));
        childObject->AddChild(std::unique_ptr<SHDocumentObject>(new RenderedObject(rendered)));
        doc.AddObject(std::move(rootObject));
        SHPropertyPanel properties("properties", "Properties", childObject);
        CHECK(properties.Groups().empty());
        childObject->AddPropertyGroup("Position").AddProperty("X", "10");
        CHECK(properties.Groups().size() == 1 && properties.Groups()[0].Properties()[0].value == "10");

        Canvas c;
        c.Width();
        c.DrawRect(0, 0, 4, 4, SHColor());
        c.FillEllipse(0, 0, 6, 6, SHColor());
        c.FillPolygon({{0, 0}, {5, 0}, {0, 5}}, SHColor());
        CHECK(c.fills > 0 && c.lines == 0);
        c.DrawEllipse(0, 0, 6, 6, SHColor());
        CHECK(c.lines > 0 && c.MeasureText("ab").width == 16);
        SHDocumentRenderer renderer;
        renderer.Render(doc, c, SHRenderMode::Preview);
        CHECK(rendered.size() == 3 && rendered[0] == SHRenderMode::Preview);

        Tool tool;
        Service service;
        SHFeature root("root", "Root");
        SHFeature childFeature("child", "Child", &root);
        SHConcept concept("drawing", "Drawing", &root);
        childFeature.RegisterTool(&tool);
        concept.RegisterService(&service);
        root.RegisterDocument(&doc);
        CHECK(childFeature.Parent() == &root && root.Features().size() == 1);
        CHECK(concept.Parent() == &root && root.Concepts().size() == 1);
        CHECK(root.Documents().size() == 1 && concept.Services().size() == 1);

        SHDialog dialog("Settings");
        dialog.AddControl(1, SHControlType::TextField, "Name", "Example");
        CHECK(dialog.Controls().size() == 1 && dialog.Controls()[0].value == "Example");

        SHFrame* f = AddFrame(std::unique_ptr<SHFrame>(new SHFrame("t")));
        RegisterFeature(&root);
        std::unique_ptr<View> v(new View);
        View* vp = v.get();
        vp->SetDocument(&doc);
        f->SetView(std::move(v));
        CHECK(vp->Frame() == f && doc.Views().size() == 1);
        doc.UpdateAllViews();
        CHECK(vp->updates == 1);
        f->Paint(c); CHECK(vp->draws == 1);
        CHECK(doc.Open("/x/y.txt") && doc.Title() == "y.txt" && !doc.IsModified());
        CHECK(!doc.Open("bad"));
        int hits = 0;
        f->AddMenuItem("File", "Go", 7, [&] { ++hits; });
        CHECK(f->Dispatch(7) && hits == 1 && !f->Dispatch(99));
        CHECK(f->Dispatch(12) && tool.activations == 1);
        CHECK(f->ToolBar() && f->ToolBar()->Items().size() == 1 &&
              f->ToolBar()->Items()[0].icon == "select-icon");
        SHPanel* panel = f->AddPanel(std::unique_ptr<SHPanel>(new SHPanel("outline", "Outline", SHDockPosition::Left)));
        CHECK(panel && panel->Position() == SHDockPosition::Left && f->Panels().size() == 1);
        std::unique_ptr<SHToolBar> tb(new SHToolBar);
        tb->AddButton(7, "Go", "go-icon").AddSeparator();
        f->SetToolBar(std::move(tb));
        CHECK(f->ToolBar()->Items().size() == 2 && f->ToolBar()->Items()[0].icon == "go-icon");
        f->SetView(nullptr);  // view destroyed -> removed from doc
        CHECK(doc.Views().empty());
        return true;
    }
};

struct AppWithoutBackend : SHApp {
    bool OnInit() override { return true; }
};

int main(int argc, char** argv) {
    AppWithoutBackend missingBackend;
    bool rejectedMissingBackend = false;
    try {
        missingBackend.Run(argc, argv);
    } catch (const std::logic_error&) {
        rejectedMissingBackend = true;
    }
    CHECK(rejectedMissingBackend && SHApp::Get() == nullptr);
    App app;
    int rc = app.Run(argc, argv);
    CHECK(SHApp::Get() == nullptr);
    std::puts(rc == 0 ? "OK" : "FAIL");
    return rc;
}
