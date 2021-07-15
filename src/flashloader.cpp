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

audiosample * flashloader::loadSample(char *filename ) {
    Serial.printf("Reading %s\n", filename);
    File f = SD.open(filename, O_READ);
    if (f) {
        if (f.size() < _bytesavailable) {
            int16_t * sampleStart = (int16_t *)(memory_begin + _head);

            uint32_t total_read = 0;
            
            while (f.available()) {
                noInterrupts();
                size_t bytesRead = f.read(memory_begin + _head + total_read/4, flashloader_default_sd_buffersize);
                interrupts();
                if (bytesRead == -1)
                    break;
                total_read += bytesRead;
            }
            _head += total_read/4;
            _bytesavailable -= total_read;

            audiosample *sample = new audiosample();
            sample->sampledata = sampleStart;
            sample->samplesize = f.size();
                    
            Serial.printf("\tsample start %x\n", (uint32_t)sampleStart);
            Serial.printf("\tsample size %d\n", sample->samplesize);
            Serial.printf("\tavailable: %d\n", _bytesavailable);

            return sample; 
        }       
    }

    Serial.printf("not found %s\n", filename);
    return nullptr;
}

}