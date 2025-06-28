cmake_minimum_required(VERSION 3.25)

project(aurum VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

add_library(aurum_core
    src/app/application.cpp
    src/engine/audio_engine.cpp
    src/dsp/effect_chain.cpp
    src/io/wav_file.cpp
    src/io/offline_renderer.cpp
    src/plugin/plugin_host.cpp
    src/preset/preset_manager.cpp
    src/gui/ring_buffer.cpp
    src/gui/aurum_gui.cpp
)

target_include_directories(aurum_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_library(PORTAUDIO_LIBRARY portaudio)
find_path(PORTAUDIO_INCLUDE_DIR portaudio.h)

if(PORTAUDIO_LIBRARY AND PORTAUDIO_INCLUDE_DIR)
    target_compile_definitions(aurum_core PUBLIC AURUM_HAS_PORTAUDIO=1)
    target_include_directories(aurum_core PRIVATE ${PORTAUDIO_INCLUDE_DIR})
    target_link_libraries(aurum_core PUBLIC ${PORTAUDIO_LIBRARY})
else()
    message(WARNING "PortAudio not found; building stub audio engine")
    target_compile_definitions(aurum_core PUBLIC AURUM_HAS_PORTAUDIO=0)
endif()

find_package(OpenGL REQUIRED)

FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.4.zip
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.90.8.zip
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    add_library(imgui_lib STATIC ${IMGUI_SOURCES})
    target_include_directories(imgui_lib PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui_lib PUBLIC glfw OpenGL::GL)
endif()

target_link_libraries(aurum_core PUBLIC imgui_lib glfw OpenGL::GL)

option(AURUM_BUILD_PLUGINS "Build bundled plugins" ON)
if(AURUM_BUILD_PLUGINS)
    add_subdirectory(plugins/gain)
    add_subdirectory(plugins/panner)
endif()

add_executable(aurum src/app/main.cpp)
target_link_libraries(aurum PRIVATE aurum_core)

if(APPLE)
    target_link_libraries(aurum_core PUBLIC "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

install(TARGETS aurum RUNTIME DESTINATION bin)
