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

# Find OpenGL
find_package(OpenGL REQUIRED)

# Graphics Engine Core Sources
set(GRAPHICS_ENGINE_SOURCES
    src/graphics/GraphicsEngine.cpp
    src/graphics/ShaderManager.cpp
    src/graphics/Pipeline.cpp
    src/graphics/PipelineGraph.cpp
    src/graphics/PipelineModule.cpp
    src/graphics/GeneratorModules.cpp
    src/graphics/OperatorModules.cpp
    src/graphics/RenderContext.cpp
)

set(GRAPHICS_ENGINE_HEADERS
    src/graphics/GraphicsEngine.h
    src/graphics/ShaderManager.h
    src/graphics/Pipeline.h
    src/graphics/PipelineGraph.h
    src/graphics/PipelineModule.h
    src/graphics/GeneratorModules.h
    src/graphics/OperatorModules.h
    src/graphics/RenderContext.h
)

# Create Graphics Engine Library
add_library(GraphicsEngineLib ${GRAPHICS_ENGINE_SOURCES} ${GRAPHICS_ENGINE_HEADERS})
target_include_directories(GraphicsEngineLib PUBLIC src/)
target_link_libraries(GraphicsEngineLib
    PUBLIC
    glfw
    GLEW::GLEW
    OpenGL::GL
)

# Test executable
add_executable(GraphicsEngineTest tests/graphics_engine_test.cpp)
target_link_libraries(GraphicsEngineTest
    PRIVATE
    GraphicsEngineLib
)

# LYGIA Explorer example (standalone utility)
add_executable(LygiaExplorer examples/lygia_explorer.cpp)
target_compile_features(LygiaExplorer PRIVATE cxx_std_20)

# LYGIA Shader Demo example
add_executable(LygiaShaderDemo examples/lygia_shader_demo.cpp)
target_link_libraries(LygiaShaderDemo
    PRIVATE
    GraphicsEngineLib
)

# Add custom target for running the main test application
add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/GraphicsEngineTest
    DEPENDS GraphicsEngineTest
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running Graphics Engine Test..."
)

# Add custom target for running LYGIA demos
add_custom_target(run-demo
    COMMAND ${CMAKE_BINARY_DIR}/LygiaShaderDemo
    DEPENDS LygiaShaderDemo
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running LYGIA Shader Demo..."
)

add_custom_target(run-explorer
    COMMAND ${CMAKE_BINARY_DIR}/LygiaExplorer
    DEPENDS LygiaExplorer
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running LYGIA Explorer..."
)
