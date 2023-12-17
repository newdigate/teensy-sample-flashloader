cmake_minimum_required(VERSION 3.5)
project(sampleflashloader)

set(SOURCES flashloader.cpp dualheapasyncflashloader.cpp)
set(HEADERS flashloader.h dualheapasyncflashloader.h audiosample.h audiochunkheap.h TeensyAudioFlashLoader.h)

teensy_add_library(sampleflashloader ${SOURCES})