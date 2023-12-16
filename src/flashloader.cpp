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
#include "flashloader.h"

namespace newdigate {

    audiosample * flashloader::loadSample(const char *filename ) {
        Serial.printf("Reading %s\n", filename);
        unsigned s = ((-_lastPointer) % 512)-4;
        Serial.printf("Align size: %x\n", s);
        auto* align = (unsigned*)extmem_malloc (s);

        File f = SD.open(filename, O_READ);
        if (f) {
            uint64_t size = f.size();
            uint mod = (-size) % 1024;
            size = size + mod;
            if (f.size() < _bytesavailable) {
                noInterrupts();
                uint32_t total_read = 0;
                auto *data = (uint32_t*)extmem_malloc( size);
                _lastPointer = (uint32_t)data;
                memset(data, 0, size + 4);

                int8_t *index = (int8_t*)data + 4;
                while (f.available()) {
                    size_t bytesRead = f.read(index, flashloader_default_sd_buffersize);
                    if (bytesRead == -1)
                        break;
                    total_read += bytesRead;
                    index += bytesRead;
                }
                memset(index, 0, mod);
                interrupts();
                _bytesavailable -= total_read;

                audiosample *sample = new audiosample();
                sample->sampledata = (int16_t*)data;
                sample->samplesize = f.size();

                Serial.printf("\tsample start %x\n", (uint32_t)data);
                Serial.printf("\tsample size %d\n", sample->samplesize);
                Serial.printf("\tavailable: %d\n", _bytesavailable);

                return sample;
            }
        }

        Serial.printf("not found %s\n", filename);
        return nullptr;
    }

    audiosample * flashloader::loadAudioPlayMemorySample(const char *filename ) {
        Serial.printf("Reading %s\n", filename);
        //unsigned s = ((-_lastPointer) % 512)-4;
        //Serial.printf("Align size: %x\n", s);
        //auto* align = (unsigned*)extmem_malloc (s);

        File f = SD.open(filename, O_READ);
        if (f) {
            uint64_t size = f.size();
            uint mod = size % 1024;
            size = size + mod;
            if (f.size() < _bytesavailable) {
                noInterrupts();
                uint32_t total_read = 0;
                auto *data = (uint32_t*)extmem_malloc( size + 4);
                memset(data, 0, size + 4);
                //data[0] = (01 << 24) | size; // format == 01 PCM
                data[0] = (0x81 << 24) | (size / 2); // format == 01 PCM

                int8_t *index = (int8_t*)data + 4;
                while (f.available()) {
                    size_t bytesRead = f.read(index, flashloader_default_sd_buffersize);
                    if (bytesRead == -1)
                        break;
                    total_read += bytesRead;
                    index += bytesRead;
                }
                memset(index, 0, mod);
                interrupts();
                _bytesavailable -= total_read;

                audiosample *sample = new audiosample();
                sample->sampledata = (int16_t*)data;
                sample->samplesize = f.size();
/*
                Serial.printf("\tsample start %x\n", (uint32_t)data);
                Serial.printf("\tsample size %d\n", sample->samplesize);
                Serial.printf("\tavailable: %d\n", _bytes_available);
*/
                return sample;
            }
        }

        Serial.printf("not found %s\n", filename);
        return nullptr;
    }

}