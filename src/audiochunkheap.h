//
// Created by Nicholas Newdigate on 16/12/2023.
//

#ifndef TEENSYS_SAMPLE_FLASHLOADER_AUDIOCHUNKHEAP_H
#define TEENSYS_SAMPLE_FLASHLOADER_AUDIOCHUNKHEAP_H

#include <Audio.h>
#include <vector>
#include <SD.h>
#include <Arduino.h>
#include "audiosample.h"

namespace newdigate {

    class audio_chunk_heap {
    private:
        uint32_t _start; // where the heap starts
        uint32_t _size;  // size in bytes of the heap
        uint32_t _offset; // the offset, starting at zero, accumulates by sample size in bytes as you load samples
        uint32_t _remainder; // the remaining bytes left for allocation
        uint32_t _index; // the current sample index, zero based.
        std::vector<audiosample *> _samples;

        void free(audiosample *sample) {
            if (!sample) {
                Serial.println("freeing a heap that doesn't belong to you....");
                return;
            }
            delete sample;
        }

    public:

        audio_chunk_heap(__uint32_t start, __uint32_t size) :
                _start(start),
                _size(size),
                _offset(0),
                _remainder(size),
                _index(0),
                _samples{} {
            reset();
        }

        audio_chunk_heap(const audio_chunk_heap &&x) = delete;

        audiosample *allocate(unsigned int size) {
            if (size < _remainder) {
                auto result = new audiosample();
                result->sampledata = (__int16_t *) (_start + _offset);
                result->samplesize = size;
                result->handle = _index;
                _index++;
                _offset += size;
                _remainder -= size;
                return result;
            }
            return nullptr; // failed
        }

        void reset() {
            // discard all samples, deallocate pointers
            for (auto index: _samples) {
                if (index)
                    free(index);
            }
            _samples.clear();
            _offset = 0;
            _remainder = _size;
            // _index = 0; -- you could reset the sample index back to zero,
        }
    };

}
#endif //TEENSYS_SAMPLE_FLASHLOADER_AUDIOCHUNKHEAP_H
