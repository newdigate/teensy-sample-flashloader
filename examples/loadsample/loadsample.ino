#include <Arduino.h>
#include <SD.h>
#include <Audio.h>
#include <TeensyVariablePlayback.h>
#include "flashloader.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlayArrayResmp      rraw_a7;        //xy=167,520
AudioPlayArrayResmp      rraw_a6;        //xy=168,489
AudioPlayArrayResmp      rraw_a8;        //xy=168,551
AudioPlayArrayResmp      rraw_a5;        //xy=169,456
AudioPlayArrayResmp      rraw_a3;        //xy=172,356
AudioPlayArrayResmp      rraw_a4;        //xy=172,387
AudioPlayArrayResmp      rraw_a2;        //xy=177,317
AudioPlayArrayResmp      rraw_a1;        //xy=181,282
AudioMixer4              mxr_fr1_left;         //xy=360,295
AudioMixer4              mxr_fr1_right;         //xy=360,363
AudioMixer4              mxr_fr2_right;         //xy=361,534
AudioMixer4              mxr_fr2_left;         //xy=364,463
AudioMixer4              mxr_back_left;         //xy=587,394
AudioMixer4              mxr_back_right;         //xy=589,459
AudioOutputI2S           i2s1;           //xy=757,443
AudioConnection          patchCord1(rraw_a7, 0, mxr_fr2_left, 2);
AudioConnection          patchCord2(rraw_a7, 1, mxr_fr2_right, 2);
AudioConnection          patchCord3(rraw_a6, 0, mxr_fr2_left, 1);
AudioConnection          patchCord4(rraw_a6, 1, mxr_fr2_right, 1);
AudioConnection          patchCord5(rraw_a8, 0, mxr_fr2_left, 3);
AudioConnection          patchCord6(rraw_a8, 1, mxr_fr2_right, 3);
AudioConnection          patchCord7(rraw_a5, 0, mxr_fr2_left, 0);
AudioConnection          patchCord8(rraw_a5, 1, mxr_fr2_right, 0);
AudioConnection          patchCord9(rraw_a3, 0, mxr_fr1_left, 2);
AudioConnection          patchCord10(rraw_a3, 1, mxr_fr1_right, 2);
AudioConnection          patchCord11(rraw_a4, 0, mxr_fr1_left, 3);
AudioConnection          patchCord12(rraw_a4, 1, mxr_fr1_right, 3);
AudioConnection          patchCord13(rraw_a2, 0, mxr_fr1_left, 1);
AudioConnection          patchCord14(rraw_a2, 1, mxr_fr1_right, 1);
AudioConnection          patchCord15(rraw_a1, 0, mxr_fr1_left, 0);
AudioConnection          patchCord16(rraw_a1, 1, mxr_fr1_right, 0);
AudioConnection          patchCord17(mxr_fr1_left, 0, mxr_back_left, 0);
AudioConnection          patchCord18(mxr_fr1_right, 0, mxr_back_right, 0);
AudioConnection          patchCord19(mxr_fr2_right, 0, mxr_back_right, 1);
AudioConnection          patchCord20(mxr_fr2_left, 0, mxr_back_left, 1);
AudioConnection          patchCord21(mxr_back_left, 0, i2s1, 0);
AudioConnection          patchCord22(mxr_back_right, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=371,652
// GUItool: end automatically generated code


newdigate::audiosample  *samples[2][4];
AudioPlayArrayResmp      *voices[2][4] =  {
        { &rraw_a1, &rraw_a2, &rraw_a3, &rraw_a4 },
        { &rraw_a5, &rraw_a6, &rraw_a7, &rraw_a8 } };

newdigate::flashloader loader( 16 * 1024);
char * fileNames[2][4] =  {
        {"BASS1.RAW", "BASS2.RAW", "BASS3.RAW", "BASS4.RAW" },
        {"BASS5.RAW", "BASS6.RAW", "BASS7.RAW", "BASS8.RAW" } };

File file;
int numLoaded = 0;
bool asyncOpening = true, asyncChanging = false, isPlaying = false;
uint32_t currentWriteHeap = 0, currentReadHeap = 1;
uint32_t patternStartTime;
const double tempo = 120.0; //Bpm
const double durationOfOneBeatInMilliseconds = 1000.0 * (60.0 / tempo);
const uint32_t durationOf32BeatsInMilliseconds = durationOfOneBeatInMilliseconds * 32.0;

void setup() {
    Serial.begin(9600);
    AudioMemory(20);
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.5f, 0.5f);

    rraw_a1.enableInterpolation(true);

    Serial.print("Initializing SD card...");
    while (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println("initialization failed!");
        delay(1000);
    }
    Serial.println("initialization done.");

}

inline void heap_switch() {
    currentWriteHeap ++;
    currentWriteHeap %= 2;
    currentReadHeap = 1 - currentWriteHeap;
    numLoaded = 0;
}
void loop() {
    //if (audio_sd_buffered_play_needs_loading) {
        // load from sd to audio buffers
    //} else
    if (numLoaded < 4) {
        if (asyncOpening) {
            const auto filename = fileNames[currentWriteHeap][numLoaded];
            file = SD.open(filename);
            samples[currentWriteHeap][numLoaded] = loader.beginAsyncLoad(file);
            if (!samples[currentWriteHeap][numLoaded]) {
                Serial.printf("abort!!! can not find '%s'...(heap:%d, index:%d)\n", filename, currentWriteHeap, numLoaded);
                delay(1000);
                return;
            }
            asyncOpening = false;
            Serial.printf("loading '%s'...      (heap:%d, index:%d)\n", filename, currentWriteHeap, numLoaded);
        } else if (loader.continueAsyncLoadPartial()) {
            Serial.printf("loading complete...  (heap:%d, index:%d)\n", currentWriteHeap, numLoaded);
            numLoaded++;
            file.close();
            asyncOpening = true;
        }
    } else if (!isPlaying) {
        isPlaying = true;
        Serial.println("Audio loaded, playback enabled!");
        loader.toggle_beforeNewPatternVoicesStart();
        loader.toggle_afterNewPatternStarts();
        patternStartTime = millis();
        heap_switch();
    }

    if (!isPlaying) return;

    for (int i=0; i < 4; i++) {
        auto voice = voices[currentReadHeap][i];
        if (!voice->isPlaying()) {
            newdigate::audiosample *sample = samples[currentReadHeap][i];
            voice->playRaw(sample->sampledata, sample->samplesize / 2, 1);
            Serial.printf("playing... (heap:%d, index:%d)\n", currentWriteHeap, i);
        }
    }

    const uint32_t now = millis();
    const uint32_t patternEndMillis = patternStartTime + durationOf32BeatsInMilliseconds;
    // 0.5 second before the next pattern starts to play, get ready
    if (now >= patternEndMillis - 500) {
        // current samples can keep playing
        if (!asyncChanging) {
            Serial.println("Pattern change coming soon!");
            loader.toggle_beforeNewPatternVoicesStart();
            asyncChanging = true;
        }
    } else if (now >= patternEndMillis) {
        asyncChanging = false;
        for (int i=0; i < 4; i++) {
            auto voice = voices[currentReadHeap][i];
            if (!voice->isPlaying()) {
                voice->stop();
            }
        }
        Serial.println("Audio loaded, playback enabled!");
        loader.toggle_afterNewPatternStarts();
        patternStartTime = now;
        heap_switch();
    }
}

void std::__throw_length_error(char const*) {

}