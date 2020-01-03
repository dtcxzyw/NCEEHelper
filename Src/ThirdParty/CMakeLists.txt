cmake_minimum_required (VERSION 3.8)
add_compile_definitions("BUS_MSVC_DELAYLOAD")
add_library(Bus "Bus/BusImpl.cpp")
