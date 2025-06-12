cmake_minimum_required(VERSION 3.25)

add_library(aurum_gain_plugin SHARED gain_plugin.cpp)
target_include_directories(aurum_gain_plugin PRIVATE ${CMAKE_SOURCE_DIR}/include)
set_target_properties(aurum_gain_plugin PROPERTIES OUTPUT_NAME "aurum_gain")
