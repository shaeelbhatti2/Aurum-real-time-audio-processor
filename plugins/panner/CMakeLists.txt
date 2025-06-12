cmake_minimum_required(VERSION 3.25)

add_library(aurum_panner_plugin SHARED panner_plugin.cpp)
target_include_directories(aurum_panner_plugin PRIVATE ${CMAKE_SOURCE_DIR}/include)
set_target_properties(aurum_panner_plugin PROPERTIES OUTPUT_NAME "aurum_panner")
