cmake_minimum_required(VERSION 3.5)
project(sampleflashloader)

set(SOURCES flashloader.cpp)
set(HEADERS flashloader.h)

teensy_add_library(sampleflashloader ${SOURCES})