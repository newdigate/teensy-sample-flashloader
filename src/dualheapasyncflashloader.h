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

#ifndef TEENSY_SAMPLE_FLASHLOADER_DUALHEAPASYNCFLASHLOADER_H
#define TEENSY_SAMPLE_FLASHLOADER_DUALHEAPASYNCFLASHLOADER_H

#include <Arduino.h>
#include <SD.h>
#include <vector>
#include <Audio.h>
#include "audiosample.h"
#include "audiochunkheap.h"

extern "C" uint8_t external_psram_size;

namespace newdigate {

    class dualheapasyncflashloader {
        const uint32_t flashloader_default_sd_buffer_size = 4 * 1024;

    public:
        const uint32_t heap_offset = 0x000A;
        const uint32_t extmem_start = 0x70000000;
        const uint32_t heap_start = extmem_start + heap_offset;

        dualheapasyncflashloader(size_t read_buffer_size = 16*1024) :
                _read_buffer_size(read_buffer_size),
                heap_end(extmem_start + (external_psram_size * 1048576) - 1),
                total_heap_size(heap_end - heap_start),
                heap_start2(heap_start + total_heap_size / 2 + 1),
                heap1(heap_start, total_heap_size / 2),
                heap2(heap_start2, total_heap_size / 2),
                _readHeap(&heap2),
                _writeHeap(&heap1) {
            _bytes_available = external_psram_size * 1048576 - heap_offset;
        }
        size_t _read_buffer_size;
        uint32_t heap_end, total_heap_size, heap_start2;
        audio_chunk_heap heap1, heap2;
        audio_chunk_heap *_readHeap;
        audio_chunk_heap *_writeHeap;

        File *_currentFile = nullptr;
        audiosample *_currentReadSample = nullptr;
        uint32_t _currentSampleOffset = 0;
        uint32_t _bytes_available;

        // always load to the next heap.
        audiosample * beginAsyncLoad(File &file) {
            if (!file) return nullptr;
            _currentFile = &file;
            auto size = _currentFile->size();
            file.seek(0);
            audiosample *sample = _writeHeap->allocate(size);
            if (!sample) {
                Serial.println("WARN: sample was not allocated");
            }
            _currentReadSample = sample;
            return sample;
        }

        audiosample * beginAsyncLoadWav(File &file) {
            if (!file) {
                Serial.println("WARN: file is not open!");
                return nullptr;
            }
            _currentFile = &file;

            auto size = _currentFile->size();
            uint mod = size % 512;
            //size = size + (512 - mod);
            size = size +  (512 - mod);
            Serial.printf("size: %x\n", size);
            file.seek(0);

            audiosample *sample = _writeHeap->allocate(size + 4);
            if (!sample) {
                Serial.println("WARN: sample was not allocated");
                return nullptr;
            }
            memset( sample->sampledata, 0, size + 4);
            //((uint32_t*)(sample->sampledata))[0] = (0x81 << 24) | (size / 2); // format == 81 PCM
            ((uint32_t*)(sample->sampledata))[0] = (0x01 << 24) | (size / 2); // format == 01 PCM
            //((uint16_t*)(sample->sampledata))[0] = 0x81 << 8 | size/2 >> 16;
            //((uint16_t*)(sample->sampledata))[1] = size/2 & 0xFFFF;
            _currentSampleOffset += 4;
            _currentReadSample = sample;
            return sample;
        }


        audiosample *loadSample(const char *filename ) {
            Serial.printf("Reading %s\n", filename);

            File f = SD.open(filename, O_READ);
            if (f) {
                uint64_t size = f.size();
                uint mod = (-size) % 1024;
                size = size + mod;
                if (f.size() < _bytes_available) {
                    noInterrupts();
                    uint32_t total_read = 0;
#ifdef BUILD_FOR_LINUX
                    auto data = new int16_t[size/2];
#else
                    auto data = static_cast<int16_t *>(extmem_malloc(size));
#endif

                    memset(data, 0, size + 4);

                    auto *index = reinterpret_cast<uint8_t *>(data);
                    while (f.available()) {
                        size_t bytesRead = f.read(index, flashloader_default_sd_buffer_size);
                        if (bytesRead == -1)
                            break;
                        total_read += bytesRead;
                        index += bytesRead;
                    }
                    memset(index, 0, mod);
                    interrupts();
                    _bytes_available -= total_read;

                    auto *sample = new audiosample();
                    sample->sampledata = data;
                    sample->samplesize = f.size();
                    //_samples.push_back(sample);

                    Serial.printf("\tsample start %x\n", data);
                    Serial.printf("\tsample size %d\n", sample->samplesize);
                    Serial.printf("\tavailable: %d\n", _bytes_available);

                    return sample;
                }
            }

            Serial.printf("not found %s\n", filename);
            return nullptr;
        }

        bool continueAsyncLoadPartial() {
            if (_currentFile && _currentReadSample && _currentFile->available()) {
                auto *index = reinterpret_cast<uint8_t *>(_currentReadSample->sampledata);
                AudioNoInterrupts();
                size_t bytesRead = _currentFile->read( index + _currentSampleOffset, _read_buffer_size);
                AudioInterrupts();

                _currentSampleOffset += bytesRead;
                if (bytesRead == _read_buffer_size) {
                    return false; // return false because the sample is not completed loading
                } else {
                    Serial.printf("Complete! totalBytesRead: %d, pointer: %x\n", _currentSampleOffset,  _currentReadSample->sampledata);
                    _currentSampleOffset = 0;
                    return true;
                }
            };

            _currentSampleOffset = 0; //reset this value for the next sample!
            return true; // return true when complete
        }

        // Switching between buffers!
        // use toggle_beforeNewPatternVoicesStart and toggle_afterNewPatternStarts in pairs
        // 1. stop all Voices;
        // 2. call toggle_beforeNewPatternVoicesStart()
        // 3. point voices at new heap samples, start playing
        // 4. call toggle_afterNewPatternStarts()
        void toggle_beforeNewPatternVoicesStart() {
            // switch the read and write heap
            AudioNoInterrupts();
            audio_chunk_heap *temp = _readHeap;
            _readHeap = _writeHeap;
            _writeHeap = temp;
            AudioInterrupts();
        }

        void toggle_afterNewPatternStarts() const {
            _writeHeap->reset();
        }
    };
};
#endif