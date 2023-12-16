#include <Arduino.h>
#include <SD.h>
#include <Audio.h>
//#include <TeensyVariablePlayback.h>
#include "dualheapasyncflashloader.h"
#include "audiosample.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlayMemory      voice7;        //xy=167,520
AudioPlayMemory      voice6;        //xy=168,489
AudioPlayMemory      voice8;        //xy=168,551
AudioPlayMemory      voice5;        //xy=169,456
AudioPlayMemory      voice3;        //xy=172,356
AudioPlayMemory      voice4;        //xy=172,387
AudioPlayMemory      voice2;        //xy=177,317
AudioPlayMemory      voice1;        //xy=181,282
AudioMixer4              mxr_fr1_left;         //xy=360,295
AudioMixer4              mxr_fr1_right;         //xy=360,363
AudioMixer4              mxr_fr2_right;         //xy=361,534
AudioMixer4              mxr_fr2_left;         //xy=364,463
AudioMixer4              mxr_back_left;         //xy=587,394
AudioMixer4              mxr_back_right;         //xy=589,459
AudioOutputI2S           i2s1;           //xy=757,443
AudioConnection          patchCord1(voice7, 0, mxr_fr2_left, 2);
AudioConnection          patchCord2(voice7, 1, mxr_fr2_right, 2);
AudioConnection          patchCord3(voice6, 0, mxr_fr2_left, 1);
AudioConnection          patchCord4(voice6, 1, mxr_fr2_right, 1);
AudioConnection          patchCord5(voice8, 0, mxr_fr2_left, 3);
AudioConnection          patchCord6(voice8, 1, mxr_fr2_right, 3);
AudioConnection          patchCord7(voice5, 0, mxr_fr2_left, 0);
AudioConnection          patchCord8(voice5, 1, mxr_fr2_right, 0);
AudioConnection          patchCord9(voice3, 0, mxr_fr1_left, 2);
AudioConnection          patchCord10(voice3, 1, mxr_fr1_right, 2);
AudioConnection          patchCord11(voice4, 0, mxr_fr1_left, 3);
AudioConnection          patchCord12(voice4, 1, mxr_fr1_right, 3);
AudioConnection          patchCord13(voice2, 0, mxr_fr1_left, 1);
AudioConnection          patchCord14(voice2, 1, mxr_fr1_right, 1);
AudioConnection          patchCord15(voice1, 0, mxr_fr1_left, 0);
AudioConnection          patchCord16(voice1, 1, mxr_fr1_right, 0);
AudioConnection          patchCord17(mxr_fr1_left, 0, mxr_back_left, 0);
AudioConnection          patchCord18(mxr_fr1_right, 0, mxr_back_right, 0);
AudioConnection          patchCord19(mxr_fr2_right, 0, mxr_back_right, 1);
AudioConnection          patchCord20(mxr_fr2_left, 0, mxr_back_left, 1);
AudioConnection          patchCord21(mxr_back_left, 0, i2s1, 0);
AudioConnection          patchCord22(mxr_back_right, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=371,652
// GUItool: end automatically generated code

audiosample  *samples[2][4];
AudioPlayMemory         *voices[2][4] =  {
        { &voice1, &voice2, &voice3, &voice4 },
        { &voice5, &voice6, &voice7, &voice8 } };

newdigate::flashloader loader(1024);
const char * fileNames[2][4] =  {
        {"test.wav", "PercA138.wav", "loop5.raw", "loop7.raw" },
        {"test.wav", "PercA138.wav", "loop6.raw", "loop8.raw" } };

File asyncLoadFile;
int numLoaded = 0;
bool asyncOpening = true, asyncChanging = false, isPlaying = false;
uint32_t currentWriteHeap = 0, currentReadHeap = 1, patternStartTime = 0;

const double tempo = 120.0; //Bpm
const double durationOfOneBeatInMilliseconds = 1000.0 * (60.0 / tempo);
const uint32_t durationOf32BeatsInMilliseconds = durationOfOneBeatInMilliseconds * 32.0;

void setup() {
    AudioMemory(64);
    Serial.begin(9600);

    //while (!Serial) { delay(100); };
    delay(100);

    mxr_fr1_left.gain(0, 0.5);
    mxr_fr1_left.gain(1, 0.5);
    mxr_fr1_left.gain(2, 0.5);
    mxr_fr1_left.gain(3, 0.5);

    mxr_fr1_right.gain(0, 0.5);
    mxr_fr1_right.gain(1, 0.5);
    mxr_fr1_right.gain(2, 0.5);
    mxr_fr1_right.gain(3, 0.5);

    mxr_fr2_right.gain(0, 0.5);
    mxr_fr2_right.gain(1, 0.5);
    mxr_fr2_right.gain(2, 0.5);
    mxr_fr2_right.gain(3, 0.5);

    mxr_fr2_left.gain(0, 0.5);
    mxr_fr2_left.gain(1, 0.5);
    mxr_fr2_left.gain(2, 0.5);
    mxr_fr2_left.gain(3, 0.5);

    mxr_back_left.gain(0, 0.5);
    mxr_back_left.gain(1, 0.5);
    mxr_back_right.gain(0, 0.5);
    mxr_back_left.gain(1, 0.5);
    /*
    voice1.enableInterpolation(false);
    voice2.enableInterpolation(false);
    voice3.enableInterpolation(false);
    voice4.enableInterpolation(false);
    voice5.enableInterpolation(false);
    voice6.enableInterpolation(false);
    voice7.enableInterpolation(false);
    voice8.enableInterpolation(false);
    */

    sgtl5000_1.enable();
    sgtl5000_1.volume(0.6);

    Serial.print("Initializing SD card...");
    while (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println("initialization failed!");
        delay(1000);
    }
    Serial.println("initialization done.");
}

inline void heap_switch() {
    //Serial.printf("Heap switch init: currentWriteHeap=%d currentReadHeap=%d !\n", currentWriteHeap, currentReadHeap);
    currentWriteHeap ++;
    currentWriteHeap %= 2;
    currentReadHeap = 1 - currentWriteHeap;
    numLoaded = 0;
    Serial.printf("Heap switch complete: currentWriteHeap=%d currentReadHeap=%d !\n", currentWriteHeap, currentReadHeap);
}

void loop() {
    const uint32_t now = millis();
    //if (audio_sd_buffered_play_needs_loading) {
    // load from sd to audio buffers
    //} else
    if (numLoaded < 4) {
        if (asyncOpening) {
            const auto filename = fileNames[currentWriteHeap][numLoaded];
            asyncLoadFile = SD.open(filename);
            samples[currentWriteHeap][numLoaded] = loader.beginAsyncLoadWav(asyncLoadFile);
            if (!samples[currentWriteHeap][numLoaded]) {
                Serial.printf("abort!!! can not find '%s'...(heap:%d, index:%d)\n", filename, currentWriteHeap, numLoaded);
                delay(1000);
                return;
            }
            asyncOpening = false;
            Serial.printf("loading '%s'...      (heap:%d, index:%d, pointer:%x)\t\t", filename, currentWriteHeap, numLoaded, samples[currentWriteHeap][numLoaded]->sampledata);
        } else if (loader.continueAsyncLoadPartial()) {
            numLoaded++;
            asyncLoadFile.close();
            asyncOpening = true;
        }
    } else if (!isPlaying) {
        isPlaying = true;
        Serial.println("Audio loaded, playback enabled!");

        patternStartTime = now;
        //const uint32_t patternEndMillis = patternStartTime + durationOf32BeatsInMilliseconds;
        //Serial.printf("patternStartTime:  %d     patternEndTime: %d\n", patternStartTime, patternEndMillis);
        loader.toggle_beforeNewPatternVoicesStart();
        loader.toggle_afterNewPatternStarts();
        heap_switch();
    }

    if (!isPlaying) return;

    if (!asyncChanging) { // don't play any new samples after the pre-pattern switch event
        for (int i=0; i < 4; i++) {
            auto voice = voices[currentReadHeap][i];
            if (voice && !voice->isPlaying()) {
                audiosample *sample = samples[currentReadHeap][i];
                if (sample)
                    voice->play((const unsigned int*)sample->sampledata);
                /*
                voice->playRaw(sample->sampledata, sample->samplesize / 2, 1);
                voice->setLoopStart(0);
                voice->setLoopFinish(sample->samplesize/2);
                voice->setLoopType(loop_type::looptype_repeat);
                */
                Serial.printf("playing... (heap:%d, index:%d - size: %d)\t\t\t%x\n", currentReadHeap, i, sample->samplesize, sample->sampledata);
            }
        }
    }

    const uint32_t patternEndMillis = patternStartTime + durationOf32BeatsInMilliseconds;
    if (now >= patternEndMillis) {
        Serial.println("Pattern switch");
        asyncChanging = false;
        for (int i=0; i < 4; i++) {
            auto voice = voices[currentReadHeap][i];
            if (voice && voice->isPlaying()) {
                voice->stop();
            }
            //samples[currentReadHeap][i] = nullptr;
        }
        if (numLoaded < 4) {
            Serial.println("WARN: not all the samples managed to load in time!");
        }

        loader.toggle_afterNewPatternStarts();

        patternStartTime = now;
        heap_switch();
    } else if (now >= patternEndMillis - 500) {
        // 0.5 second before the next pattern starts to play, get ready
        // current samples can keep playing
        if (!asyncChanging) {
            Serial.println("pre-pattern switch event, pattern change coming soon!");
            loader.toggle_beforeNewPatternVoicesStart();
            asyncChanging = true;
        }
    }

    // my thinking re: delay(1) is to give the mcu a some time between partial loads, not sure thou...
    delay(1);
}


// this method below is needed for c++17 - it doesn't link otherwise, but for c++14 this is not needed:
void std::__throw_length_error(char const*) {
    // don't worry, be happy!
}
