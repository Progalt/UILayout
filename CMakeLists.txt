cmake_minimum_required(VERSION 3.12) 

project(UILayout) 

set(NANOVG_BUILD_EXAMPLES OFF)

add_subdirectory(external/nanovg)

add_subdirectory("LatteLayout/")


# Tests

include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
FetchContent_MakeAvailable(glfw)

add_executable(uilayout_test "example/test.c" "example/glad/glad.c" "example/glad/glad.h" "example/glad/khrplatform.h")
target_link_libraries(uilayout_test nanovg LatteLayout glfw)


