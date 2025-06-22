cmake_minimum_required(VERSION 3.15)
project(GraphicsEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Try to use system GLEW first, fallback to custom build
find_package(GLEW QUIET)
if(NOT GLEW_FOUND)
    # GLEW 설치 경로 (스크립트 빌드 위치)
    set(GLEW_ROOT_DIR "${CMAKE_SOURCE_DIR}/build/glew-install")
    
    # Check if custom GLEW exists
    if(EXISTS "${GLEW_ROOT_DIR}/lib/libGLEW.a")
        # GLEW include 경로와 라이브러리 경로 추가
        include_directories("${GLEW_ROOT_DIR}/include")
        set(GLEW_LIBRARIES "${GLEW_ROOT_DIR}/lib/libGLEW.a")
    else()
        message(FATAL_ERROR "GLEW not found. Please run './scripts/build_glew.sh' first or install system GLEW")
    endif()
endif()

# 다른 필요한 라이브러리(예: OpenGL, GLFW)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

include_directories(${GLFW_INCLUDE_DIRS})
link_directories(${GLFW_LIBRARY_DIRS})

# 소스 파일
set(SOURCES
    src/main.cpp
    src/Shader.cpp
    src/PipelineParser.cpp
)

add_executable(GraphicsEngine ${SOURCES})

# Copy shaders directory to build directory
add_custom_command(TARGET GraphicsEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        ${CMAKE_CURRENT_BINARY_DIR}/shaders
    COMMENT "Copying shaders to build directory"
)

# Copy pipeline.txt to build directory
add_custom_command(TARGET GraphicsEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/pipeline.txt
        ${CMAKE_CURRENT_BINARY_DIR}/pipeline.txt
    COMMENT "Copying pipeline.txt to build directory"
)

# 링커에 GLEW, OpenGL, GLFW 연결 - 순서가 중요함
target_link_libraries(GraphicsEngine
    ${GLEW_LIBRARIES}
    OpenGL::GL
    OpenGL::GLU
    ${GLFW_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

# 필요시 pthread (예: GLFW나 GLEW가 요구하면)
find_package(Threads REQUIRED)
target_link_libraries(GraphicsEngine Threads::Threads)
