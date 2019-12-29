cmake_minimum_required (VERSION 3.8)

project ("NCEEHelper")
add_compile_options("/std:c++17")
find_path(RANG_INCLUDE_DIR rang.hpp)
include_directories(${RANG_INCLUDE_DIR})

ADD_SUBDIRECTORY(Src)

