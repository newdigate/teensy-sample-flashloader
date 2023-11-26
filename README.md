# teensy sample flashloader
[![lib-teensy41](https://github.com/newdigate/teensy-sample-flashloader/actions/workflows/teensy41_lib.yml/badge.svg)](https://github.com/newdigate/teensy-sample-flashloader/actions/workflows/teensy41_lib.yml)
[![example-loadsample](https://github.com/newdigate/teensy-sample-flashloader/actions/workflows/teensy41_loadsample.yml/badge.svg)](https://github.com/newdigate/teensy-sample-flashloader/actions/workflows/teensy41_loadsample.yml)

load `.RAW` audio samples from micro-SD card to external `psram` or `flash` memory on teensy 4.1

**NB**: This library currently only works with `.RAW` audio files - `.WAV` files are not supported. 
* Use Audacity to convert your audio samples to `.RAW` format (`16 bits/sample`, `44100 samples/sec`, `signed integer samples`). 
* `playWav` method does **NOT** work. only playRaw currently works. 
* You can play a `.WAV` file using `playRaw(...)` but the 44 byte wav header will produce a tiny fragment of noise at the begging of the sample...

``` c++
#include <Arduino.h>
#include <SD.h>
#include <Audio.h>
#include <TeensyVariablePlayback.h>
#include "flashloader.h"

// GUItool: begin automatically generated code
AudioPlayArrayResmp      rraw_a1;        //xy=321,513
AudioOutputI2S           i2s1;           //xy=675,518
AudioConnection          patchCord1(rraw_a1, 0, i2s1, 0);
AudioConnection          patchCord2(rraw_a1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=521,588
// GUItool: end automatically generated code

unsigned long lastSamplePlayed = 0;
newdigate::audiosample *sample;

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

    newdigate::flashloader loader;
    sample = loader.loadSample("KICK.RAW");
}

const int numChannels = 1; // 1 for mono, 2 for stereo...

void loop() {
    unsigned currentMillis = millis();
    if (currentMillis > lastSamplePlayed + 500) {
        if (!rraw_a1.isPlaying()) {
            rraw_a1.playRaw(sample->sampledata, sample->samplesize/2, numChannels);
            lastSamplePlayed = currentMillis;

            Serial.print("Memory: ");
            Serial.print(AudioMemoryUsage());
            Serial.print(",");
            Serial.print(AudioMemoryUsageMax());
            Serial.println();
        }
    }
    delay(10);
}
```
