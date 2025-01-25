cmake_minimum_required(VERSION 3.21)
project(main LANGUAGES CXX)
include(FetchContent)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(RtMidi
    GIT_REPOSITORY https://github.com/thestk/rtmidi.git
    GIT_TAG master)
FetchContent_MakeAvailable(RtMidi)

set(RTMIDI_TARGET_NAME "rtmidi")
if(TARGET ${RTMIDI_TARGET_NAME})
    message(STATUS "RtMidi is correctly set up.")
else()
    message(FATAL_ERROR "Failed to configure RtMidi.")
endif()

add_executable(TriMIDI ./src/main.cpp)

target_link_libraries(TriMIDI PRIVATE 
    sfml-system 
    sfml-window 
    sfml-graphics 
)

target_link_libraries(TriMIDI PRIVATE rtmidi)

target_compile_features(TriMIDI PRIVATE cxx_std_17)

if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET TriMIDI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:TriMIDI> $<TARGET_FILE_DIR:TriMIDI> COMMAND_EXPAND_LISTS)
endif()

install(TARGETS TriMIDI)
