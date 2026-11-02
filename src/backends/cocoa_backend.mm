// Cocoa backend (macOS, Clang/Objective-C++).
#import <Cocoa/Cocoa.h>
#include <map>
#include "../backend.h"


@interface SHCocoaView : NSView
@property(nonatomic) shore::SHFrame* shoreFrame;
@end

@interface SHCocoaTarget : NSObject
@property(nonatomic) shore::SHFrame* frame;
- (void)fire:(id)sender;
@end

@interface SHAppDelegate : NSObject <NSApplicationDelegate>
@property(nonatomic) shore::SHApp* app;
@property(nonatomic) BOOL ok;
@end

namespace shore {
namespace {
NSString* S(const std::string& s) { return [NSString stringWithUTF8String:s.c_str()]; }
NSColor* Col(SHColor c) { return [NSColor colorWithSRGBRed:c.r / 255.0 green:c.g / 255.0 blue:c.b / 255.0 alpha:1.0]; }

class CocoaCanvas : public SHCanvas {
public:
    explicit CocoaCanvas(NSRect r) : r_(r) {}
    int Width() const override { return (int)r_.size.width; }
    int Height() const override { return (int)r_.size.height; }
    // The view is flipped, so coordinates are top-left based like other backends.
    void FillRect(int x, int y, int w, int h, SHColor c) override {
        [Col(c) setFill];
        NSRectFill(NSMakeRect(x, y, w, h));
    }
    void DrawLine(int x1, int y1, int x2, int y2, SHColor c) override {
        [Col(c) setStroke];
        [NSBezierPath strokeLineFromPoint:NSMakePoint(x1, y1) toPoint:NSMakePoint(x2, y2)];
    }
    void DrawText(int x, int y, const std::string& t, SHColor c) override {
        [S(t) drawAtPoint:NSMakePoint(x, y) withAttributes:@{NSForegroundColorAttributeName : Col(c)}];
    }
private:
    NSRect r_;
};
}  // namespace
}  // namespace shore

@implementation SHCocoaView
- (BOOL)isFlipped { return YES; }
- (void)drawRect:(NSRect)dirty {
    [[NSColor whiteColor] setFill];
    NSRectFill(self.bounds);
    shore::CocoaCanvas c(self.bounds);
    self.shoreFrame->Paint(c);
}
- (void)mouseDown:(NSEvent*)e {
    NSPoint p = [self convertPoint:e.locationInWindow fromView:nil];
    if (self.shoreFrame->View()) self.shoreFrame->View()->OnMouseDown((int)p.x, (int)p.y);
}
@end

@implementation SHCocoaTarget
- (void)fire:(id)sender { self.frame->Dispatch((int)[sender tag]); }
@end

@implementation SHAppDelegate
- (void)applicationDidFinishLaunching:(NSNotification*)n {
    self.ok = self.app->OnInit();
    if (!self.ok) [NSApp terminate:nil];
    [NSApp activateIgnoringOtherApps:YES];
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)a { return YES; }
@end

namespace shore {
namespace {
class CocoaPeer : public SHFramePeer {
public:
    explicit CocoaPeer(SHFrame* f) {
        NSRect r = NSMakeRect(100, 100, f->Width(), f->Height());
        window_ = [[NSWindow alloc] initWithContentRect:r
                                              styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
        window_.releasedWhenClosed = NO;
        window_.title = S(f->Title());
        NSView* content = window_.contentView;
        status_ = [NSTextField labelWithString:@""];
        status_.frame = NSMakeRect(4, 0, r.size.width - 8, 18);
        status_.autoresizingMask = NSViewWidthSizable;
        [content addSubview:status_];
        view_ = [[SHCocoaView alloc] initWithFrame:NSMakeRect(0, 18, r.size.width, r.size.height - 18)];
        view_.shoreFrame = f;
        view_.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        [content addSubview:view_];
        target_ = [SHCocoaTarget new];
        target_.frame = f;
        if (!NSApp.mainMenu) NSApp.mainMenu = [NSMenu new];
        // Application menu slot so user menus appear after it.
        NSMenuItem* appItem = [NSMenuItem new];
        NSMenu* appMenu = [NSMenu new];
        [appMenu addItemWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
        appItem.submenu = appMenu;
        [NSApp.mainMenu addItem:appItem];
    }
    ~CocoaPeer() override { [window_ close]; }
    void SetTitle(const std::string& t) override { window_.title = S(t); }
    void SetStatusText(const std::string& s) override { status_.stringValue = S(s); }
    void AddMenuItem(const std::string& menu, const std::string& label, int id) override {
        NSMenu*& m = menus_[menu];
        if (!m) {
            m = [[NSMenu alloc] initWithTitle:S(menu)];
            NSMenuItem* top = [NSMenuItem new];
            top.submenu = m;
            [NSApp.mainMenu addItem:top];
        }
        NSMenuItem* it = [m addItemWithTitle:S(label) action:@selector(fire:) keyEquivalent:@""];
        it.target = target_;
        it.tag = id;
    }
    void SetToolBar(const SHToolBar& tb) override {
        // Toolbar rendered as a row of buttons above the view (keeps the API simple and identical across backends).
        for (NSView* v in [toolbar_ subviews]) [v removeFromSuperview];
        if (!toolbar_) {
            toolbar_ = [[NSView alloc] initWithFrame:NSMakeRect(0, window_.contentView.bounds.size.height - 28, window_.contentView.bounds.size.width, 28)];
            toolbar_.autoresizingMask = NSViewWidthSizable | NSViewMinYMargin;
            [window_.contentView addSubview:toolbar_];
            view_.frame = NSMakeRect(0, 18, view_.frame.size.width, view_.frame.size.height - 28);
        }
        CGFloat x = 4;
        for (const auto& it : tb.Items()) {
            if (it.separator) { x += 10; continue; }
            NSButton* b = [NSButton buttonWithTitle:S(it.label) target:target_ action:@selector(fire:)];
            b.tag = it.id;
            b.bezelStyle = NSBezelStyleTexturedRounded;
            [b sizeToFit];
            b.frame = NSMakeRect(x, 2, b.frame.size.width, 24);
            x += b.frame.size.width + 4;
            [toolbar_ addSubview:b];
        }
    }
    void Show(bool s) override { if (s) [window_ makeKeyAndOrderFront:nil]; else [window_ orderOut:nil]; }
    void Invalidate() override { view_.needsDisplay = YES; }
private:
    NSWindow* window_;
    SHCocoaView* view_;
    NSTextField* status_;
    NSView* toolbar_ = nil;
    SHCocoaTarget* target_;
    std::map<std::string, NSMenu*> menus_;
};
}  // namespace

namespace backend {
std::unique_ptr<SHFramePeer> CreateFramePeer(SHFrame* f) { return std::unique_ptr<SHFramePeer>(new CocoaPeer(f)); }
int Run(SHApp* app, int, char**) {
    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        SHAppDelegate* d = [SHAppDelegate new];
        d.app = app;
        NSApp.delegate = d;
        [NSApp run];
        return d.ok ? 0 : 1;
    }
}
void Quit() { [NSApp terminate:nil]; }
const char* Name() { return "Cocoa"; }
}  // namespace backend
}  // namespace shore
