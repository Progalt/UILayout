cmake_minimum_required(VERSION 3.12)

project(LatteLayout VERSION 1.0)

set(
	SOURCES "LatteLayout/layout.c" "LatteLayout/layout.h"
)

add_library(LatteLayout STATIC ${SOURCES})

target_include_directories(LatteLayout PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(LatteLayout PUBLIC c_std_11)