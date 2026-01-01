cmake_minimum_required(VERSION 3.12) 

project(LatteUI) 

set(NANOVG_BUILD_EXAMPLES OFF)


add_subdirectory(external/nanovg)

add_subdirectory("LatteLayout/")


# Tests

include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.28
)
FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(sol2)

set(LATTE_SOURCES
    "LatteRuntime/main.cpp" 
    "LatteRuntime/OS/Window.h" 
    "LatteRuntime/OS/EventLoop.h"
    "LatteRuntime/Utils/Singleton.h" 
    "LatteRuntime/OS/WindowManager.h" 
    "LatteRuntime/OS/WindowManager.cpp" 
    "LatteRuntime/OS/EventLoop.cpp" 
    "LatteRuntime/OS/Window.cpp"
    "LatteRuntime/glad/glad.c" 
    "LatteRuntime/glad/glad.h" 
    "LatteRuntime/glad/khrplatform.h"
    "LatteRuntime/Components/Component.h" 
    "LatteRuntime/Components/Component.cpp" 
    "LatteRuntime/Rendering/NodeRenderer.h" 
    "LatteRuntime/Rendering/NodeRenderer.cpp" 
    "LatteRuntime/Rendering/Color.h" 
    "LatteRuntime/Utils/Log.h" 
    "LatteRuntime/Utils/Log.cpp" 
    "LatteRuntime/Components/ComponentEvents.h" 
    "LatteRuntime/OS/Event.h" 
    "LatteRuntime/Components/ComponentEvents.cpp"
    "LatteRuntime/Router/Router.h" 
    "LatteRuntime/Router/Router.cpp" 
    "LatteRuntime/Components/ComponentLibrary.h" 
    "LatteRuntime/Components/ComponentLibrary.cpp" 
    "LatteRuntime/Components/Focus.h" 
    "LatteRuntime/Components/Focus.cpp" 
    "LatteRuntime/Rendering/FontMetrics.h" 
    "LatteRuntime/Rendering/FontMetrics.cpp"
 "LatteRuntime/OS/Clipboard.h" "LatteRuntime/OS/Clipboard.cpp" "LatteRuntime/Utils/LuaHelpers.h")

# --------- PLATFORM LOGIC
# CMake provides ANDROID automatically
if(ANDROID)
    message(STATUS "Configuring for Android platform")

    add_library(LatteRuntime SHARED ${LATTE_SOURCES})

    target_link_libraries(LatteRuntime PRIVATE log)

else()
    message(STATUS "Configuring for native desktop platform")

    add_executable(LatteRuntime ${LATTE_SOURCES})

    set_property(TARGET LatteRuntime PROPERTY CXX_STANDARD 20)
endif()

# Link core dependencies for both
target_link_libraries(LatteRuntime
    PRIVATE
    nanovg
    LatteLayout
    sol2::sol2
    SDL3::SDL3
)

# ------- LUJIT HANDLING (android or native)
set(LUAJIT_BASE_PATH "${CMAKE_SOURCE_DIR}/luajit/src")
if(WIN32)
    set(LUAJIT_LIB_PATH "${LUAJIT_BASE_PATH}/lua51.lib")
    set(LUAJIT_SO_PATH "${LUAJIT_BASE_PATH}/lua51.dll")
elseif(ANDROID)
    # Place your manually built libluajit.so for Android here:
    set(LUAJIT_LIB_PATH "${LUAJIT_BASE_PATH}/libluajit.so")
    set(LUAJIT_SO_PATH "${LUAJIT_BASE_PATH}/libluajit.so")
else()
    set(LUAJIT_LIB_PATH "${LUAJIT_BASE_PATH}/libluajit.a")
    set(LUAJIT_SO_PATH "${LUAJIT_BASE_PATH}/libluajit.so")
endif()

# Setup import target if LuaJIT is available
if(EXISTS ${LUAJIT_SO_PATH})
    message(STATUS "Found LuaJIT: ${LUAJIT_SO_PATH}")
    if(ANDROID OR UNIX)
        add_library(LuaJIT::lua51 SHARED IMPORTED)
        set_target_properties(LuaJIT::lua51 PROPERTIES
            IMPORTED_LOCATION ${LUAJIT_SO_PATH}
            INTERFACE_INCLUDE_DIRECTORIES ${LUAJIT_BASE_PATH}
        )
    elseif(WIN32)
        add_library(LuaJIT::lua51 SHARED IMPORTED)
        set_target_properties(LuaJIT::lua51 PROPERTIES
            IMPORTED_LOCATION ${LUAJIT_SO_PATH}
            IMPORTED_IMPLIB ${LUAJIT_LIB_PATH}
            INTERFACE_INCLUDE_DIRECTORIES ${LUAJIT_BASE_PATH}
        )
    endif()
    target_link_libraries(LatteRuntime PRIVATE LuaJIT::lua51)
else()
    message(WARNING "LuaJIT not built/found. Lua integration will not be available!")
endif()

# Additional system libraries for non-apple unix
if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_link_libraries(LatteRuntime PRIVATE dl m)
endif()

# --------- Post-build Copies

# Only do these on native (running "apps" on android host does not make sense),
# alternatively, you can use CPack or android packaging for these assets.
if(NOT ANDROID)
    add_custom_command(TARGET LatteRuntime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/Tests
            $<TARGET_FILE_DIR:LatteRuntime>/Tests
    )
    add_custom_command(TARGET LatteRuntime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/luaSrc
            $<TARGET_FILE_DIR:LatteRuntime>/luaSrc
    )
endif()