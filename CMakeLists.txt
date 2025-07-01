cmake_minimum_required(VERSION 3.15)
project(GraphicsEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use FetchContent for dependencies
include(FetchContent)

# Find or download GLFW
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    message(STATUS "GLFW not found, downloading...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif()

# Find or download GLEW (using git version that supports CMake)
find_package(GLEW QUIET)
if(NOT GLEW_FOUND)
    message(STATUS "GLEW not found, downloading...")
    FetchContent_Declare(
        glew
        GIT_REPOSITORY https://github.com/nigels-com/glew.git
        GIT_TAG glew-2.2.0
    )
    set(BUILD_UTILS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glew)
    
    # Create GLEW::GLEW alias for consistency
    if(TARGET glew)
        add_library(GLEW::GLEW ALIAS glew)
    elseif(TARGET libglew_static)
        add_library(GLEW::GLEW ALIAS libglew_static)
    endif()
endif()

# Download liblo for OSC communication
message(STATUS "Downloading liblo for OSC communication...")
FetchContent_Declare(
    liblo
    GIT_REPOSITORY https://github.com/radarsat1/liblo.git
    GIT_TAG 0.31
)
set(ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_TOOLS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(liblo)

# Download ImGui for GUI rendering
message(STATUS "Downloading ImGui...")
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
)
FetchContent_MakeAvailable(imgui)

# Download ImGui Node Editor for Node Editor Binary
message(STATUS "Downloading ImGui Node Editor...")
FetchContent_Declare(
    imgui-node-editor
    GIT_REPOSITORY https://github.com/thedmd/imgui-node-editor.git
    GIT_TAG master
)
FetchContent_MakeAvailable(imgui-node-editor)

# Download AngelScript for Code Interpreter Binary
message(STATUS "Downloading AngelScript...")
FetchContent_Declare(
    angelscript
    GIT_REPOSITORY https://github.com/codecat/angelscript-mirror.git
    GIT_TAG master
)
FetchContent_MakeAvailable(angelscript)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Create ImGui library
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# Graphics Engine Core Library (shared between binaries)
set(GRAPHICS_ENGINE_CORE_SOURCES
    src/core/NodeGraph.cpp
)

set(GRAPHICS_ENGINE_CORE_HEADERS
    src/core/NodeGraph.h
)
# OSC Communication Library (shared between binaries)
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

# Create shared libraries
add_library(GraphicsEngineCore ${GRAPHICS_ENGINE_CORE_SOURCES} ${GRAPHICS_ENGINE_CORE_HEADERS})
target_include_directories(GraphicsEngineCore PUBLIC src/)
target_link_libraries(GraphicsEngineCore
    PUBLIC
    glfw
    GLEW::GLEW
    OpenGL::GL
)

add_library(OSCCommunication ${OSC_COMMUNICATION_SOURCES} ${OSC_COMMUNICATION_HEADERS})
target_include_directories(OSCCommunication PUBLIC src/)
target_link_libraries(OSCCommunication
    PUBLIC
    lo
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
target_compile_definitions(graphics_engine PRIVATE LYGIA_PATH="${CMAKE_SOURCE_DIR}/external/lygia")
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
target_link_libraries(code_interpreter
    PRIVATE
    GraphicsEngineCore
    OSCCommunication
)

# ============================================================================
# Legacy Examples and Tests
# ============================================================================

# LYGIA Explorer example (standalone utility)
add_executable(LygiaExplorer examples/lygia_explorer.cpp)
target_compile_features(LygiaExplorer PRIVATE cxx_std_20)

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
