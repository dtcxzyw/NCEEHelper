cmake_minimum_required (VERSION 3.8)
find_package(nlohmann_json REQUIRED)
find_package(cxxopts REQUIRED)
find_package(icu REQUIRED)
#find_path(UTFCPP_INCLUDE_DIR utf8.h)
#include_directories(${UTFCPP_INCLUDE_DIR})
add_link_options("/DELAYLOAD:icudt61.dll")
add_link_options("/DELAYLOAD:icuin61.dll")
add_link_options("/DELAYLOAD:icuio61.dll")
add_link_options("/DELAYLOAD:icuuc61.dll")
add_link_options("/DELAYLOAD:icutu61.dll")
aux_source_directory(. SOURCE)
add_executable (NCEEHelper ${SOURCE})
find_library(ICUDT icudt)
target_link_libraries(NCEEHelper ${ICUDT})
find_library(ICUUC icuuc)
target_link_libraries(NCEEHelper ${ICUUC})
find_library(ICUIO icuio)
target_link_libraries(NCEEHelper ${ICUIO})
find_library(ICUIN icuin)
target_link_libraries(NCEEHelper ${ICUIN})
find_library(ICUTU icutu)
target_link_libraries(NCEEHelper ${ICUTU})
target_link_libraries(NCEEHelper Bus)
install (TARGETS NCEEHelper DESTINATION .)
