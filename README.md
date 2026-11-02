# Shore

A small, abstract C++ Document/View framework. The public API (`SHApp`, `SHDocument`,
`SHView`, `SHFrame`, `SHToolBar`, `SHCanvas`) in `include/shore/shore.h` is backend independent;
the backend is chosen at build time:

| Platform | Backend  | Toolchain      |
|----------|----------|----------------|
| Windows  | MFC      | MSVC           |
| Linux    | wxWidgets| g++            |
| macOS    | Cocoa    | LLVM/Clang     |
| any      | null     | headless/tests |

```
cmake -B build -DSHORE_BACKEND=auto   # auto|mfc|wx|cocoa|null
cmake --build build
ctest --test-dir build
```

See `examples/notepad.cpp`. GitHub Actions (`.github/workflows/build.yml`) builds all three platforms.

## Optional UI and document architecture

The public API also includes dock-positioned `SHPanel`s, `SHFeature`/`SHConcept`
registries, lifecycle-aware `SHTool`s and `SHService`s, nested
`SHDocumentObject`s, rendering modes, file-type registration, object property
groups, and declarative `SHDialog` controls. These are opt-in: applications can
continue using only `SHApp`, `SHDocument`, `SHView`, and `SHFrame`.

Features can register tools and be installed on a frame (or registered with
`SHApp::RegisterFeature`) to create their menu commands. Toolbars accept an
optional backend-defined icon identifier. Panels are passed to the frame peer
with their requested dock position so backends can map them to native docking
systems. Backend-specific options can be supplied
through `SHApp::SetBackendProperty`, for example `wx.toolkit` or `mfc.ribbon`;
the backend reads the keys it supports from `BackendProperties()`.

Documents may own trees of `SHDocumentObject`s. `SHDocumentRenderer` traverses
those trees and dispatches `OnRendering` with an editing, preview, print, or
export mode. `SHPropertyPanel` exposes the property groups of its selected
object, including objects nested at any depth. `SHFileTypeRegistry` stores
application-defined serializers, which can be used with the typed `Open` and
`Save` overloads.
