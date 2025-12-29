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
 "LatteRuntime/Router/Router.h" "LatteRuntime/Router/Router.cpp" "LatteRuntime/Components/ComponentLibrary.h" "LatteRuntime/Components/ComponentLibrary.cpp")

add_executable(LatteRuntime ${LATTE_SOURCES})
target_link_libraries(LatteRuntime nanovg LatteLayout sol2::sol2 SDL3::SDL3)
set_property(TARGET LatteRuntime PROPERTY CXX_STANDARD 20)

set(LUAJIT_BASE_PATH "${CMAKE_SOURCE_DIR}/luajit/src")

# Set platform-specific library paths
if(WIN32)
    set(LUAJIT_LIB_PATH "${LUAJIT_BASE_PATH}/lua51.lib")      
    set(LUAJIT_SO_PATH "${LUAJIT_BASE_PATH}/lua51.dll")      
else() 
    set(LUAJIT_LIB_PATH "${LUAJIT_BASE_PATH}/libluajit.a")   
    set(LUAJIT_SO_PATH "${LUAJIT_BASE_PATH}/libluajit.so")   
endif()

if(WIN32)
    set(LUAJIT_CHECK_FILE ${LUAJIT_SO_PATH}) 
else()
    set(LUAJIT_CHECK_FILE ${LUAJIT_LIB_PATH})
endif()

if(EXISTS ${LUAJIT_CHECK_FILE})
    message(STATUS "Found LuaJIT: ${LUAJIT_CHECK_FILE}")

    if(WIN32)
        # Windows: Use shared library with import library
        if(EXISTS ${LUAJIT_LIB_PATH})
            add_library(LuaJIT::lua51 SHARED IMPORTED)
            set_target_properties(LuaJIT::lua51 PROPERTIES
                IMPORTED_LOCATION ${LUAJIT_SO_PATH}
                IMPORTED_IMPLIB ${LUAJIT_LIB_PATH}
                INTERFACE_INCLUDE_DIRECTORIES ${LUAJIT_BASE_PATH}
            )

            # Copy DLL to target directory at build time
            add_custom_command(TARGET LatteRuntime POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${LUAJIT_SO_PATH}
                    $<TARGET_FILE_DIR:LatteRuntime>/lua51.dll
            )
        else()
            message(FATAL_ERROR "LuaJIT DLL found but import library not found: ${LUAJIT_LIB_PATH}")
        endif()
    else()
        # Unix: Use static library (or shared if preferred)
        if(EXISTS ${LUAJIT_SO_PATH})
            # Prefer shared library if available
            add_library(LuaJIT::lua51 SHARED IMPORTED)
            set_target_properties(LuaJIT::lua51 PROPERTIES
                IMPORTED_LOCATION ${LUAJIT_SO_PATH}
                INTERFACE_INCLUDE_DIRECTORIES ${LUAJIT_BASE_PATH}
            )
        else()
            # Fall back to static library
            add_library(LuaJIT::lua51 STATIC IMPORTED)
            set_target_properties(LuaJIT::lua51 PROPERTIES
                IMPORTED_LOCATION ${LUAJIT_LIB_PATH}
                INTERFACE_INCLUDE_DIRECTORIES ${LUAJIT_BASE_PATH}
            )
        endif()
    endif()

    # Link against LuaJIT
    target_link_libraries(LatteRuntime LuaJIT::lua51)

    # On Unix systems, may need to link additional libraries
    if(UNIX AND NOT APPLE)
        target_link_libraries(LatteRuntime dl m)
    endif()

else()
    message(FATAL_ERROR "LuaJIT not built. Please build LuaJIT first.")
endif()


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
