cmake_minimum_required(VERSION 3.21)
project(main LANGUAGES CXX)
include(FetchContent)

# FetchContent for SFML
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

# FetchContent for RtMidi
FetchContent_Declare(RtMidi
    GIT_REPOSITORY https://github.com/thestk/rtmidi.git
    GIT_TAG master) # Replace "master" with a specific version if needed
FetchContent_MakeAvailable(RtMidi)

# Ensure RtMidi is a target
set(RTMIDI_TARGET_NAME "rtmidi")
if(TARGET ${RTMIDI_TARGET_NAME})
    message(STATUS "RtMidi is correctly set up.")
else()
    message(FATAL_ERROR "Failed to configure RtMidi.")
endif()

# Main executable
add_executable(main main.cpp)

# Link SFML libraries
target_link_libraries(main PRIVATE 
    sfml-system 
    sfml-window 
    sfml-graphics 
)

# Link RtMidi library
target_link_libraries(main PRIVATE rtmidi)

# Set C++ standard
target_compile_features(main PRIVATE cxx_std_17)

# Post-build for shared library copying on Windows
if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:main> $<TARGET_FILE_DIR:main> COMMAND_EXPAND_LISTS)
endif()

# Install target
install(TARGETS main)
