cmake_minimum_required(VERSION 3.5)
project(example_loadsample)


if (NOT DEFINED TEENSY_VERSION)
    set(CMAKE_CXX_STANDARD 17)

    find_package(teensy_x86_stubs)
    include_directories(${teensy_x86_stubs_INCLUDE_DIR})

    find_package(teensy_audio_x86_stubs)
    include_directories(${teensy_audio_x86_stubs_INCLUDE_DIR})

    find_package(teensy_x86_sd_stubs)
    include_directories(${teensy_x86_sd_stubs_INCLUDE_DIR})

    find_package(teensy_variable_playback)
    include_directories(${teensy_variable_playback_INCLUDE_DIR})

    include_directories(../../src)
    add_executable(loadsample loadsample.cpp)
else()
    teensy_include_directories(../../src)
    teensy_add_executable(loadsample loadsample.cpp)
    teensy_target_link_libraries(loadsample Audio  sampleflashloader TeensyVariablePlayback   SD SdFat  SPI SerialFlash  Wire  cores)
endif ()