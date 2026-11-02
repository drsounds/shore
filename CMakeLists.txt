cmake_minimum_required(VERSION 3.16)
project(shore VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(SHORE_BUILD_EXAMPLES "Build examples (requires an installed backend package)" OFF)
set(SHORE_BACKEND_TARGET "" CACHE STRING "Backend CMake target for the example")
set(SHORE_BACKEND_PACKAGE "" CACHE STRING "Backend package name for the example")
set(SHORE_BACKEND_HEADER "" CACHE STRING "Backend factory header for the example")
set(SHORE_BACKEND_FACTORY "" CACHE STRING "Qualified backend factory function for the example")
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
add_library(shore ${_core_sources})
add_library(shore::shore ALIAS shore)
target_include_directories(shore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(SHORE_BUILD_EXAMPLES)
  if(NOT SHORE_BACKEND_PACKAGE OR NOT SHORE_BACKEND_TARGET OR
     NOT SHORE_BACKEND_HEADER OR NOT SHORE_BACKEND_FACTORY)
    message(FATAL_ERROR "Set SHORE_BACKEND_PACKAGE, SHORE_BACKEND_TARGET, SHORE_BACKEND_HEADER, and SHORE_BACKEND_FACTORY to build the example")
  endif()
  find_package(${SHORE_BACKEND_PACKAGE} CONFIG REQUIRED)
  add_executable(notepad examples/notepad.cpp)
  target_link_libraries(notepad PRIVATE shore ${SHORE_BACKEND_TARGET})
  target_compile_definitions(notepad PRIVATE
    SHORE_BACKEND_HEADER=\"${SHORE_BACKEND_HEADER}\"
    SHORE_BACKEND_FACTORY=${SHORE_BACKEND_FACTORY}
  )
endif()

if(SHORE_BUILD_TESTS)
  enable_testing()
  add_executable(shore_tests tests/test_shore.cpp)
  target_sources(shore_tests PRIVATE tests/test_backend.cpp)
  target_link_libraries(shore_tests PRIVATE shore)
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
