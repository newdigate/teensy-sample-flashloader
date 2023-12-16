//
// Created by Nicholas Newdigate on 16/12/2023.
//

#ifndef TEENSYS_SAMPLE_FLASHLOADER_AUDIOSAMPLE_H
#define TEENSYS_SAMPLE_FLASHLOADER_AUDIOSAMPLE_H

#include <Audio.h>
#include <vector>
#include <SD.h>
#include <Arduino.h>

namespace newdigate {
    struct audiosample {
        int16_t *sampledata;
        uint32_t samplesize;
        uint16_t handle;
    };
}
#endif //TEENSYS_SAMPLE_FLASHLOADER_AUDIOSAMPLE_H
