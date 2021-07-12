cmake_minimum_required(VERSION 3.5)
project(teensys_sample_flashloader)

import_arduino_library(cores ${COREPATH} avr debug util)
import_arduino_library(SPI ${DEPSPATH}/SPI)
import_arduino_library(SdFat ${DEPSPATH}/SdFat/src common DigitalIO ExFatLib FatLib FsLib iostream SdCard SpiDriver)
import_arduino_library(SD ${DEPSPATH}/SD/src)

add_subdirectory(src)
add_subdirectory(examples/loadsample)