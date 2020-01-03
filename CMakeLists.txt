cmake_minimum_required (VERSION 3.8)

project ("NCEEHelper")
add_compile_options("/std:c++17")
add_compile_definitions("UNICODE")
add_compile_definitions("_UNICODE")
add_compile_options("/utf-8")
find_path(RANG_INCLUDE_DIR rang.hpp)
include_directories(${RANG_INCLUDE_DIR})

ADD_SUBDIRECTORY(Src)

