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

    class audio_chunk_heap;

    struct audiosample {
        int16_t *sampledata;
        uint32_t samplesize;
        uint16_t handle;
        audio_chunk_heap *heap; // just to check we don't free the sample from the wrong pool
    };

    class audio_chunk_heap {
    private:
        uint32_t start;
        std::vector<bool> chunks_available;
        std::vector<uint32_t> chunks_start;
        std::vector<unsigned> &chunk_sizes; // in multiples of 1024 bytes, arranged from smallest to biggest!
    public:

        audio_chunk_heap(
                std::vector<unsigned> &sizes,
                uint32_t start) :
            start(start),
            chunks_available(),
            chunks_start(),
            chunk_sizes(sizes)
        {
            auto offset = start;
            for(auto&& index : chunk_sizes) {
                chunks_available.push_back(true);
                chunks_start.push_back( offset );
                offset += index;
            }
            reset();
        }

        audio_chunk_heap(const audio_chunk_heap&& x) = delete;

        audiosample* allocate(size_t size) {
            uint32_t required_chunk_size = ceil(size / 1024);
            for(uint32_t i=0; i < chunks_available.size(); i++) {
                if (chunks_available[i]) {
                    if (chunk_sizes[i] > required_chunk_size) {
                        chunks_available[i] = false; // reserve this chunk
                        auto result = new audiosample();
                        result->sampledata = (int16_t*)chunks_start[i];
                        result->samplesize = size;
                        result->handle = i;
                        result->heap = this;
                        return result;
                    }
                }
            }
            return nullptr; // failed
        }

        void free(audiosample* sample) {
            if (!sample) return;
            if (sample->heap != this) { Serial.println("freeing a heap that doesn't belong to you...."); }
            if (sample->handle < chunks_available.size()) {
                chunks_available[sample->handle] = true;
            }
        }

        void reset() {
            for(uint32_t i=0; i < chunks_available.size(); i++) {
                chunks_available[i] = true;
            }
        }

    };


    class flashloader {
    public:
        const uint32_t heap_offset = 0x000A;
        const uint32_t extmem_start = 0x70000000;
        const uint32_t heap_start = extmem_start + heap_offset;

        flashloader(size_t read_buffer_size) :
                _read_buffer_size(read_buffer_size),
                chunk_sizes {512, 512, 512, 512, 1024, 1024, 1024, 1024, 1024, 1024},
                heap_end(extmem_start + (external_psram_size * 1048576) - 1),
                heap_size(heap_end - heap_start),
                heap_start2(heap_start + heap_size/2 + 1),
                heap1(chunk_sizes, heap_start),
                heap2(chunk_sizes, heap_start2),
                _readHeap(&heap1),
                _writeHeap(&heap2) {
            _bytes_available = external_psram_size * 1048576 - heap_offset;
        }
        size_t _read_buffer_size;
        std::vector<unsigned> chunk_sizes; // in multiples of 1024 bytes!
        uint32_t heap_end, heap_size, heap_start2;
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
            _currentReadSample = sample;
            return sample;
        }

        bool continueAsyncLoadPartial() {
            if (!_currentFile && _currentReadSample) {
                size_t bytesRead = _currentFile->read( _currentReadSample->sampledata +  _currentSampleOffset , _read_buffer_size);
                _currentSampleOffset += bytesRead;
                if (bytesRead == _read_buffer_size)
                    return false; // return false because the sample is not completed loading
            };

            _currentSampleOffset = 0; //reset this value for the next sample!
            return true; // return true when complete
        }

        void free(audiosample* sample) {
            delete sample;
        }

        // when finished with a set of audio samples,
        // i.e once all samples have completed and once the new pattern starts playing,
        // before the new set of samples are loaded, de-allocate the old samples
        void beforeToggleHeap() {
            _readHeap->reset();
        }

        void toggleHeap() {
            audio_chunk_heap *temp = _readHeap;
            _readHeap = _writeHeap;
            _writeHeap = temp;
        }

        audiosample * loadSample(const char *filename );
    };
};
#endif