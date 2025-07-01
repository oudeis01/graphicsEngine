cmake_minimum_required(VERSION 3.15)
project(GraphicsEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# FetchContent Cache Configuration
# ============================================================================
# Set a persistent cache directory within project to preserve downloads
# This keeps everything self-contained while avoiding re-downloads
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.cmake-cache" CACHE STRING "FetchContent cache directory")
message(STATUS "FetchContent cache directory: ${FETCHCONTENT_BASE_DIR}")

# Force all dependencies to be downloaded instead of using system packages
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG OFF)

# Build static libraries by default for self-contained binaries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

# Disable unnecessary features in dependencies
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# Use FetchContent for all dependencies
include(FetchContent)

# ============================================================================
# Download GLFW (always from source)
# ============================================================================
message(STATUS "Downloading GLFW...")
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
    GIT_SHALLOW TRUE
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ============================================================================
# Download GLEW (always from source) - use a pre-built repository
# ============================================================================
message(STATUS "Downloading GLEW...")
FetchContent_Declare(
    glew
    URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.zip
)
FetchContent_MakeAvailable(glew)

# Build GLEW from the downloaded release
add_library(libglew_static STATIC 
    ${glew_SOURCE_DIR}/src/glew.c
)
target_include_directories(libglew_static PUBLIC ${glew_SOURCE_DIR}/include)
target_compile_definitions(libglew_static PUBLIC GLEW_STATIC)
if(UNIX AND NOT APPLE)
    target_link_libraries(libglew_static PUBLIC dl)
endif()

# Use the static GLEW target
set(GLEW_TARGET libglew_static)

# ============================================================================
# Download OpenGL headers (modern OpenGL loader)
# ============================================================================
message(STATUS "Downloading OpenGL headers...")
FetchContent_Declare(
    opengl_headers
    GIT_REPOSITORY https://github.com/KhronosGroup/OpenGL-Registry.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(opengl_headers)

# Find system OpenGL library (this is unavoidable as it's a system driver)
find_package(OpenGL REQUIRED)

# ============================================================================
# Download ImGui for GUI rendering (always from source)
# ============================================================================
message(STATUS "Downloading ImGui...")
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

# ============================================================================
# Download ImGui Node Editor (always from source)
# ============================================================================
message(STATUS "Downloading ImGui Node Editor...")
FetchContent_Declare(
    imgui-node-editor
    GIT_REPOSITORY https://github.com/thedmd/imgui-node-editor.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui-node-editor)

# ============================================================================
# Download AngelScript (always from source)
# ============================================================================
message(STATUS "Downloading AngelScript...")
FetchContent_Declare(
    angelscript
    GIT_REPOSITORY https://github.com/codecat/angelscript-mirror.git
    GIT_TAG master
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR sdk/angelscript/projects/cmake
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(angelscript)

# ============================================================================
# liblo (OSC Communication) - MUST be dynamically linked per LGPL-2.1+
# ============================================================================
# Note: liblo MUST be dynamically linked to comply with LGPL-2.1+ license
# We use system liblo instead of building from source to ensure dynamic linking

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBLO REQUIRED liblo)

if(NOT LIBLO_FOUND)
    message(FATAL_ERROR "liblo not found. Please install liblo development package:
    Ubuntu/Debian: sudo apt-get install liblo-dev
    Fedora/RHEL: sudo dnf install liblo-devel
    Arch Linux: sudo pacman -S liblo
    macOS: brew install liblo
    Windows: Use vcpkg or install from source")
endif()

# Create interface library for liblo with dynamic linking
add_library(liblo_dynamic INTERFACE)
target_link_libraries(liblo_dynamic INTERFACE ${LIBLO_LIBRARIES})
target_include_directories(liblo_dynamic INTERFACE ${LIBLO_INCLUDE_DIRS})
target_compile_options(liblo_dynamic INTERFACE ${LIBLO_CFLAGS_OTHER})

# ============================================================================
# Create ImGui library with all required sources
# ============================================================================
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)

# ============================================================================
# OSC Communication Library (shared between binaries)
# ============================================================================
set(OSC_COMMUNICATION_SOURCES
    src/osc/OSCServer.cpp
    src/osc/OSCClient.cpp
    src/osc/OSCMessages.cpp
)

set(OSC_COMMUNICATION_HEADERS
    src/osc/OSCServer.h
    src/osc/OSCClient.h
    src/osc/OSCMessages.h
)

# ============================================================================
# Graphics Engine Core Library (shared between binaries)
# ============================================================================
set(GRAPHICS_ENGINE_CORE_SOURCES
    src/core/NodeGraph.cpp
)

set(GRAPHICS_ENGINE_CORE_HEADERS
    src/core/NodeGraph.h
)

# ============================================================================
# Create shared libraries with downloaded dependencies
# ============================================================================
add_library(GraphicsEngineCore STATIC ${GRAPHICS_ENGINE_CORE_SOURCES} ${GRAPHICS_ENGINE_CORE_HEADERS})
target_include_directories(GraphicsEngineCore PUBLIC 
    src/
    ${glfw_SOURCE_DIR}/include
    ${glew_SOURCE_DIR}/include
)
target_link_libraries(GraphicsEngineCore
    PUBLIC
    glfw
    ${GLEW_TARGET}
    OpenGL::GL
)

add_library(OSCCommunication STATIC ${OSC_COMMUNICATION_SOURCES} ${OSC_COMMUNICATION_HEADERS})
target_include_directories(OSCCommunication PUBLIC 
    src/
)
target_link_libraries(OSCCommunication
    PUBLIC
    liblo_dynamic  # Dynamic linking for LGPL compliance
)

# ============================================================================
# Binary 1: Graphics Engine (OSC Server with OpenGL Window)
# ============================================================================

set(GRAPHICS_ENGINE_BINARY_SOURCES
    src/graphics_engine/main.cpp
    src/graphics_engine/GraphicsEngine.cpp
    src/graphics_engine/RenderContext.cpp
    src/graphics_engine/ShaderManager.cpp
    src/graphics_engine/Pipeline.cpp
)

add_executable(graphics_engine ${GRAPHICS_ENGINE_BINARY_SOURCES})
target_compile_definitions(graphics_engine PRIVATE 
    LYGIA_PATH="${CMAKE_SOURCE_DIR}/external/lygia"
    GLEW_STATIC
)
target_include_directories(graphics_engine PRIVATE 
    ${glfw_SOURCE_DIR}/include
    ${glew_SOURCE_DIR}/include
    ${liblo_SOURCE_DIR}
)
target_link_libraries(graphics_engine
    PRIVATE
    GraphicsEngineCore
    OSCCommunication
)

# ============================================================================
# Binary 2: Node Editor (OSC Client)
# ============================================================================

set(NODE_EDITOR_BINARY_SOURCES
    src/node_editor/main.cpp
    src/node_editor/NodeEditor.cpp
    src/core/NodeGraph.cpp
)

add_executable(node_editor ${NODE_EDITOR_BINARY_SOURCES})
target_compile_definitions(node_editor PRIVATE GLEW_STATIC)
target_include_directories(node_editor PRIVATE 
    ${glfw_SOURCE_DIR}/include
    ${glew_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${liblo_SOURCE_DIR}
)
target_link_libraries(node_editor
    PRIVATE
    GraphicsEngineCore
    OSCCommunication
    imgui
    glfw
    OpenGL::GL
)

# ============================================================================
# Binary 3: Code Interpreter (OSC Client)
# ============================================================================

set(CODE_INTERPRETER_BINARY_SOURCES
    src/code_interpreter/main.cpp
    src/code_interpreter/CodeInterpreter.cpp
    src/core/NodeGraph.cpp
)

add_executable(code_interpreter ${CODE_INTERPRETER_BINARY_SOURCES})
target_include_directories(code_interpreter PRIVATE 
    ${angelscript_SOURCE_DIR}/angelscript/include
    ${liblo_SOURCE_DIR}
)
target_link_libraries(code_interpreter
    PRIVATE
    GraphicsEngineCore
    OSCCommunication
    angelscript
)

# ============================================================================
# Legacy Examples and Tests
# ============================================================================

# LYGIA Explorer example (standalone utility)
# TODO: Restore when examples/lygia_explorer.cpp is implemented
# add_executable(LygiaExplorer examples/lygia_explorer.cpp)
# target_compile_features(LygiaExplorer PRIVATE cxx_std_20)

# Note: Legacy GraphicsEngineTest and LygiaShaderDemo have been removed
# as they depend on the old GraphicsEngine and ShaderManager classes
# which are incompatible with the new OSC-based distributed architecture.

# ============================================================================
# Custom Run Targets
# ============================================================================

# Start Graphics Engine server
add_custom_target(run-engine
    COMMAND ${CMAKE_BINARY_DIR}/graphics_engine
    DEPENDS graphics_engine
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Starting Graphics Engine Server (OSC Server on port 57120)..."
)

# Start Node Editor client
add_custom_target(run-node-editor
    COMMAND ${CMAKE_BINARY_DIR}/node_editor
    DEPENDS node_editor
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Starting Node Editor (OSC Client)..."
)

# Start Code Interpreter client with example script
add_custom_target(run-code-interpreter
    COMMAND ${CMAKE_BINARY_DIR}/code_interpreter examples/scripts/example.as
    DEPENDS code_interpreter
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Starting Code Interpreter with example script..."
)

# Start all three binaries (for development testing)
add_custom_target(run-all
    COMMAND echo "Starting Graphics Engine Server..." && ${CMAKE_BINARY_DIR}/graphics_engine &
    COMMAND sleep 2
    COMMAND echo "Starting Node Editor..." && ${CMAKE_BINARY_DIR}/node_editor &
    COMMAND sleep 1
    COMMAND echo "Starting Code Interpreter..." && ${CMAKE_BINARY_DIR}/code_interpreter examples/scripts/example.as
    DEPENDS graphics_engine node_editor code_interpreter
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Starting all three binaries for testing..."
)

# Legacy compatibility targets (disabled - binaries don't exist in new architecture)
# add_custom_target(run
#     COMMAND ${CMAKE_BINARY_DIR}/GraphicsEngineTest
#     DEPENDS GraphicsEngineTest
#     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#     COMMENT "Running legacy Graphics Engine Test..."
# )

# add_custom_target(run-demo
#     COMMAND ${CMAKE_BINARY_DIR}/LygiaShaderDemo
#     DEPENDS LygiaShaderDemo
#     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#     COMMENT "Running LYGIA Shader Demo..."
# )

add_custom_target(run-explorer
    COMMAND ${CMAKE_BINARY_DIR}/LygiaExplorer
    DEPENDS LygiaExplorer
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running LYGIA Explorer..."
)

# ============================================================================
# Selective Clean Targets (preserve downloaded sources)
# ============================================================================

# Clean all build artifacts but preserve downloaded sources in _deps
add_custom_target(clean-build
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/Makefile
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/graphics_engine || true
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/node_editor || true
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/code_interpreter || true
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/lib || true
    COMMENT "Cleaning build artifacts while preserving downloaded sources..."
)

# Full clean including downloaded sources (use when dependencies need refresh)
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Full clean including downloaded sources (will re-download everything)..."
)

# Quick reconfigure (clean build cache but keep compiled objects when possible)
add_custom_target(reconfigure
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Reconfiguring CMake while preserving as much as possible..."
)

# ============================================================================
