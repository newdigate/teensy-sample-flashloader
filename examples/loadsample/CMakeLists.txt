cmake_minimum_required(VERSION 3.5)
project(example_loadsample)

import_arduino_library(SerialFlash ${DEPSPATH}/SerialFlash util)
import_arduino_library(Wire ${DEPSPATH}/Wire utility)
import_arduino_library(arm_math ${DEPSPATH}/arm_math/src)
import_arduino_library(Audio ${DEPSPATH}/Audio utility)
import_arduino_library(TeensyVariablePlayback ${DEPSPATH}/teensy-variable-playback/src)


teensy_include_directories(../../src)
teensy_add_executable(loadsample loadsample.ino)
teensy_target_link_libraries(loadsample   sampleflashloader TeensyVariablePlayback Audio  SD SdFat  SPI SerialFlash  Wire  cores)