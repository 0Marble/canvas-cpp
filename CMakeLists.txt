cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17)

project(canvas)

file(GLOB SOURCES src/*)
add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC include)

enable_testing()

option(WITH_GLFW "Add default implementation for a GLFW+OpenGL canvas" OFF)
if (WITH_GLFW)
    add_subdirectory(deps/external/glfw)
    add_subdirectory(deps/external/glad)
    
    target_link_libraries(${PROJECT_NAME} PUBLIC glfw glad)

    add_executable(window_test tests/window_test.cpp)
    add_test(NAME canvas_window_test COMMAND window_test)
    target_link_libraries(window_test PRIVATE ${PROJECT_NAME})
endif()

add_executable(bmp_test tests/bmp_test.cpp)
add_test(NAME canvas_bmp_test COMMAND bmp_test)
target_link_libraries(bmp_test PRIVATE ${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PUBLIC -g)

