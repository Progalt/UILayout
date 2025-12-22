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
            add_custom_command(TARGET uilayout_test POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${LUAJIT_SO_PATH}
                    $<TARGET_FILE_DIR:uilayout_test>/lua51.dll
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
    target_link_libraries(uilayout_test LuaJIT::lua51)

    # On Unix systems, may need to link additional libraries
    if(UNIX AND NOT APPLE)
        target_link_libraries(uilayout_test dl m)
    endif()

else()
    message(FATAL_ERROR "LuaJIT not built. Please build LuaJIT first.")
endif()
