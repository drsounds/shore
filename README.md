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
