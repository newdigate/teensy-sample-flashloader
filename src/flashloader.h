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
#include <vector>
#include <Audio.h>

extern "C" uint8_t external_psram_size;

namespace newdigate {

    const uint32_t flashloader_default_sd_buffer_size = 4 * 1024;

    class audio_chunk_heap;

    struct audiosample {
        int16_t *sampledata;
        uint32_t samplesize;
        uint16_t handle;
        audio_chunk_heap *heap; // just to check we don't free the sample from the wrong pool
    };

    class audio_chunk_heap {
    private:
        uint32_t _start; // where the heap starts
        uint32_t _size;  // size in bytes of the heap
        uint32_t _offset; // the offset, starting at zero, accumulates by sample size in bytes as you load samples
        uint32_t _remainder; // the remaining bytes left for allocation
        uint32_t _index; // the current sample index, zero based.
        std::vector<audiosample*> _samples;
        void free(audiosample* sample) {
            if (!sample) { Serial.println("freeing a heap that doesn't belong to you...."); return; }
            if (sample->heap != this) { Serial.println("freeing a heap that doesn't belong to you...."); }
            delete sample;
        }
    public:

        audio_chunk_heap(uint32_t start, uint32_t size) :
                _start(start),
                _size(size),
                _offset(0),
                _remainder(size),
                _index(0),
                _samples { }
        {
            reset();
        }

        audio_chunk_heap(const audio_chunk_heap&& x) = delete;

        audiosample* allocate(size_t size) {
            if (size < _remainder) {
                auto result = new audiosample();
                result->sampledata = (int16_t*)(_start + _offset);
                result->samplesize = size;
                result->handle = _index;
                result->heap = this;
                _index++;
                _offset += size;
                _remainder -= size;
                return result;
            }
            return nullptr; // failed
        }

        void reset() {
            // discard all samples, deallocate pointers
            for ( auto index : _samples) {
                if (index)
                    free(index);
            }
            _samples.clear();
            _offset = 0;
            _remainder = _size;
            // _index = 0; -- you could reset the sample index back to zero,
        }
    };

    class flashloader {
    public:
        const uint32_t heap_offset = 0x000A;
        const uint32_t extmem_start = 0x70000000;
        const uint32_t heap_start = extmem_start + heap_offset;

        flashloader(size_t read_buffer_size = 16*1024) :
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

        bool continueAsyncLoadPartial() {
            if (_currentFile && _currentReadSample) {

                AudioNoInterrupts();
                size_t bytesRead = _currentFile->read( _currentReadSample->sampledata +  _currentSampleOffset, _read_buffer_size);
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

        audiosample * loadSampleWav(const char *filename );
        audiosample * loadSample(const char *filename );
    };
};
#endif