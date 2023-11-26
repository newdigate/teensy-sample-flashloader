cmake_minimum_required(VERSION 3.5)
project(example_loadsample)

teensy_include_directories(../../src)
teensy_add_executable(loadsample loadsample.ino)
teensy_target_link_libraries(loadsample   sampleflashloader TeensyVariablePlayback Audio  SD SdFat  SPI SerialFlash  Wire  cores)