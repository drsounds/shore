# Shore

A small, abstract C++ Document/View framework. The public API (`SHApp`, `SHDocument`,
`SHView`, `SHFrame`, `SHToolBar`, `SHCanvas`) in `include/shore/shore.h` is backend independent;
the backend is supplied by a separate peer package:

| Platform | Backend  | Toolchain      |
|----------|----------|----------------|
| Windows  | MFC or WTL | MSVC         |
| Linux    | wxWidgets| g++            |
| any      | Qt 6     | Qt 6 Widgets   |
| macOS    | Cocoa    | LLVM/Clang     |
| any      | null     | headless/tests |

Build and install the core independently with CMake, then link the desired
`shore-backend-*` package. An app passes that package's `CreateBackend()` result
to `SHApp` (for example, in its constructor). Backend dependencies are no longer
required to build or use the core API.

To build the bundled example, set `SHORE_BACKEND_PACKAGE`,
`SHORE_BACKEND_TARGET`, `SHORE_BACKEND_HEADER`, and `SHORE_BACKEND_FACTORY` to
the selected backend's package name, CMake target, factory header, and factory
function.

See `examples/notepad.cpp` for the framework API. Each backend peer repository
contains its own build, package, and platform-specific workflow.

## Using with vcpkg

The `ports/shore` directory is an overlay port. Add this repository's `ports`
directory as an overlay port and add `shore` to your vcpkg manifest dependencies.
Each peer backend repository has a separate vcpkg port and depends on this core
package plus its platform toolkit.

Link the exported CMake target:

```cmake
find_package(shore CONFIG REQUIRED)
target_link_libraries(your_app PRIVATE shore::shore)
```

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
