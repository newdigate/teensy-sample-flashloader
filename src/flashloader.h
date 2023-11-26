/* Audio Library for Teensy
 * Copyright (c) 2021, Nic Newdigate
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef TEENSY_SAMPLE_FLASHLOADER_FLASHLOADER_H
#define TEENSY_SAMPLE_FLASHLOADER_FLASHLOADER_H

#include <Arduino.h>
#include <SD.h>

extern "C" uint8_t external_psram_size;

namespace newdigate {

    const uint32_t flashloader_default_sd_buffersize = 4 * 1024;

    struct audiosample {
        int16_t *sampledata;
        uint32_t samplesize;
    };

    class flashloader {
    public:

        flashloader() {
            _bytesavailable = external_psram_size * 1048576;
        }

        uint32_t _bytesavailable=0;
        audiosample * loadSample(const char *filename );
    };
};
#endif