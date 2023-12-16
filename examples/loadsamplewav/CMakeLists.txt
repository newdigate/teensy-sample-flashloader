cmake_minimum_required(VERSION 3.5)
project(example_loadsample_wav)

teensy_include_directories(../../src)
teensy_add_executable(loadsamplewav loadsamplewav.ino)
teensy_target_link_libraries(loadsamplewav Audio  sampleflashloader TeensyVariablePlayback   SD SdFat  SPI SerialFlash  Wire  cores)