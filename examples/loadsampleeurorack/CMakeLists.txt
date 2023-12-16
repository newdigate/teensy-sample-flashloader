cmake_minimum_required(VERSION 3.5)
project(example_loadsampleeurorack)

teensy_include_directories(../../src)
teensy_add_executable(loadsampleeurorack loadsample.ino)
teensy_target_link_libraries(loadsampleeurorack  Audio arm_math sampleflashloader TeensyVariablePlayback   SD SdFat  SPI SerialFlash  Wire  cores)
target_link_libraries(loadsampleeurorack.elf m stdc++)
