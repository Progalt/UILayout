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

set(LATTEUI_SOURCES
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
    "LatteRuntime/OS/Clipboard.h" 
    "LatteRuntime/OS/Clipboard.cpp" 
    "LatteRuntime/Utils/LuaHelpers.h" 
    "LatteRuntime/Components/Core.h" 
    "LatteRuntime/Components/Core.cpp"
 "LatteRuntime/latteui.h" "LatteRuntime/latteui.cpp")

# Build LatteUI library
if(ANDROID)
    message(STATUS "Configuring for Android platform")

    add_library(latteui SHARED ${LATTEUI_SOURCES})

    target_link_libraries(latteui PRIVATE log)

else()
    message(STATUS "Configuring for native desktop platform")

    add_library(latteui SHARED ${LATTEUI_SOURCES})
    target_include_directories(latteui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/LatteRuntime")

    if (WIN32)
        target_compile_definitions(latteui PRIVATE LATTEUI_EXPORTS)
    endif()

    set_property(TARGET latteui PROPERTY CXX_STANDARD 20)
endif()

# Link core dependencies for both
target_link_libraries(latteui
    PRIVATE
    nanovg
    LatteLayout
    sol2::sol2
    SDL3::SDL3
)

# Latte 
set(LATTECLI_SOURCES
    "LatteCLI/main.cpp"
)

add_executable(latte ${LATTECLI_SOURCES})

target_link_libraries(latte PRIVATE latteui sol2::sol2)

set_property(TARGET latte PROPERTY CXX_STANDARD 20)

# ------- LUJIT HANDLING 
set(LUAJIT_BASE_PATH "${CMAKE_SOURCE_DIR}/external/luajit/src")
if(WIN32)
    set(LUAJIT_LIB_PATH "${LUAJIT_BASE_PATH}/lua51.lib")
    set(LUAJIT_SO_PATH "${LUAJIT_BASE_PATH}/lua51.dll")
elseif(ANDROID)
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
    target_link_libraries(latteui PRIVATE LuaJIT::lua51)
    target_link_libraries(latte PRIVATE LuaJIT::lua51)
else()
    message(WARNING "LuaJIT not built/found. Lua integration will not be available!")
endif()

# Additional system libraries for non-apple unix
if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_link_libraries(latteui PRIVATE dl m)
endif()

# Post-build Copies
if(NOT ANDROID)
    add_custom_command(TARGET latteui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/Tests
            $<TARGET_FILE_DIR:latteui>/Tests
    )
    add_custom_command(TARGET latteui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/luaSrc
            $<TARGET_FILE_DIR:latteui>/luaSrc
    )
     add_custom_command(TARGET latteui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/Templates
            $<TARGET_FILE_DIR:latteui>/Templates
    )
endif()