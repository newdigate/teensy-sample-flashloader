//
// Created by Nicholas Newdigate on 16/12/2023.
//

#ifndef TEENSYS_SAMPLE_FLASHLOADER_AUDIOSAMPLE_H
#define TEENSYS_SAMPLE_FLASHLOADER_AUDIOSAMPLE_H

#include <Audio.h>
#include <vector>
#include <SD.h>
#include <Arduino.h>
class audio_chunk_heap;

struct audiosample {
    __int16_t *sampledata;
    __uint32_t samplesize;
    __uint16_t handle;
    audio_chunk_heap *heap; // just to check we don't free the sample from the wrong pool
};
#endif //TEENSYS_SAMPLE_FLASHLOADER_AUDIOSAMPLE_H
