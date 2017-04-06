cmake_minimum_required(VERSION 3.5)
project(first-opengl-prj)

find_package(glfw3 3.1.2 REQUIRED)
find_package(OpenGL REQUIRED)

set(SRC_FILES src/main.cpp)

add_executable(first-opengl ${SRC_FILES})

target_compile_options(first-opengl PUBLIC -std=c++1y)

target_include_directories(first-opengl PUBLIC /usr/include/libdrm ${OPENGL_INCLUDE_DIR})

target_link_libraries (first-opengl glfw ${OPENGL_gl_LIBRARY})
