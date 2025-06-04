cmake_minimum_required(VERSION 3.25)

project(aurum VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(aurum_core
    src/app/application.cpp
    src/engine/audio_engine.cpp
    src/dsp/effect_chain.cpp
    src/io/wav_file.cpp
    src/io/offline_renderer.cpp
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

add_executable(aurum src/app/main.cpp)
target_link_libraries(aurum PRIVATE aurum_core)

install(TARGETS aurum RUNTIME DESTINATION bin)
