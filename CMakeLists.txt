cmake_minimum_required(VERSION 3.16)
project(shore VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Backend selection: auto picks MFC on Windows, Cocoa on macOS, wxWidgets elsewhere.
set(SHORE_BACKEND "auto" CACHE STRING "auto|mfc|wtl|cocoa|wx|null")
set_property(CACHE SHORE_BACKEND PROPERTY STRINGS auto mfc wtl cocoa wx null)
if(SHORE_BACKEND STREQUAL "auto")
  if(WIN32)
    set(_backend mfc)
  elseif(APPLE)
    set(_backend cocoa)
  else()
    set(_backend wx)
  endif()
else()
  set(_backend ${SHORE_BACKEND})
endif()
message(STATUS "Shore backend: ${_backend}")

option(SHORE_BUILD_EXAMPLES "Build examples" ON)
option(SHORE_BUILD_TESTS "Build tests" ON)

set(_core_sources
  src/app.cpp
  src/canvas.cpp
  src/concept.cpp
  src/dialog.cpp
  src/document.cpp
  src/document_object.cpp
  src/document_renderer.cpp
  src/feature.cpp
  src/feature_registry.cpp
  src/file_type.cpp
  src/file_type_registry.cpp
  src/frame.cpp
  src/property_group.cpp
  src/property_panel.cpp
  src/tool.cpp
  src/view.cpp
)
set(_sources ${_core_sources})
set(SHORE_NEEDS_WX FALSE)
if(_backend STREQUAL "null")
  list(APPEND _sources src/backends/null_backend.cpp)
elseif(_backend STREQUAL "wx")
  list(APPEND _sources src/backends/wx_backend.cpp)
  set(SHORE_NEEDS_WX TRUE)
elseif(_backend STREQUAL "mfc")
  list(APPEND _sources src/backends/mfc_backend.cpp)
elseif(_backend STREQUAL "wtl")
  if(NOT WIN32 OR NOT MSVC)
    message(FATAL_ERROR "The WTL backend requires a Windows MSVC-compatible toolchain")
  endif()
  list(APPEND _sources src/backends/wtl_backend.cpp)
elseif(_backend STREQUAL "cocoa")
  enable_language(OBJCXX)
  list(APPEND _sources src/backends/cocoa_backend.mm)
else()
  message(FATAL_ERROR "Unknown SHORE_BACKEND '${_backend}'")
endif()

add_library(shore ${_sources})
add_library(shore::shore ALIAS shore)
target_include_directories(shore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE src
)

if(_backend STREQUAL "wx")
  find_package(wxWidgets REQUIRED COMPONENTS core base)
  include(${wxWidgets_USE_FILE})
  target_link_libraries(shore PUBLIC ${wxWidgets_LIBRARIES})
elseif(_backend STREQUAL "mfc")
  target_compile_definitions(shore PUBLIC _AFXDLL UNICODE _UNICODE)
  set(CMAKE_MFC_FLAG 2)
  set_target_properties(shore PROPERTIES MFC_FLAG 2)
elseif(_backend STREQUAL "wtl")
  find_path(WTL_INCLUDE_DIR atlapp.h
    HINTS "${WTL_ROOT}" "$ENV{WTL_ROOT}"
    PATH_SUFFIXES Include include WTL
  )
  if(NOT WTL_INCLUDE_DIR)
    message(FATAL_ERROR "WTL headers were not found. Set WTL_ROOT to the WTL installation directory.")
  endif()
  target_include_directories(shore PRIVATE "${WTL_INCLUDE_DIR}")
  target_compile_definitions(shore PUBLIC UNICODE _UNICODE)
  target_link_libraries(shore PUBLIC comctl32)
elseif(_backend STREQUAL "cocoa")
  target_link_libraries(shore PUBLIC "-framework Cocoa")
endif()

if(SHORE_BUILD_EXAMPLES)
  add_executable(notepad WIN32 examples/notepad.cpp)
  target_link_libraries(notepad PRIVATE shore)
  if(_backend STREQUAL "mfc")
    set_target_properties(notepad PROPERTIES MFC_FLAG 2)
    # keep a console-style main() entry point
    target_link_options(notepad PRIVATE /ENTRY:mainCRTStartup)
  elseif(_backend STREQUAL "wtl")
    target_link_options(notepad PRIVATE /ENTRY:mainCRTStartup)
  endif()
endif()

if(SHORE_BUILD_TESTS)
  enable_testing()
  # Core logic is backend independent, so test it against the null backend.
  add_library(shore_null ${_core_sources} src/backends/null_backend.cpp)
  target_include_directories(shore_null PUBLIC include PRIVATE src)
  add_executable(shore_tests tests/test_shore.cpp)
  target_link_libraries(shore_tests PRIVATE shore_null)
  add_test(NAME shore_tests COMMAND shore_tests)
endif()

install(TARGETS shore EXPORT shoreTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT shoreTargets
  FILE shoreTargets.cmake
  NAMESPACE shore::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/shore
)

configure_package_config_file(
  cmake/shoreConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/shoreConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/shore
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/shoreConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/shoreConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/shoreConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/shore
)
