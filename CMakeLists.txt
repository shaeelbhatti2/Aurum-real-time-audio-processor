cmake_minimum_required(VERSION 3.25)

project(aurum VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(aurum_core
    src/app/application.cpp
)

target_include_directories(aurum_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(aurum src/app/main.cpp)
target_link_libraries(aurum PRIVATE aurum_core)

install(TARGETS aurum RUNTIME DESTINATION bin)
