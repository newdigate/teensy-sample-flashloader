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
newdigate::audiosample *samples[4];

std::vector<unsigned> chunk_sizes {512, 512, 512, 512, 1024, 1024, 1024, 1024, 1024, 1024};
newdigate::flashloader loader(  chunk_sizes, 16 * 1024);
char * fileNames[] =  {
        "BASS1.RAW", "BASS2.RAW", "BASS3.RAW", "BASS4.RAW", "BASS5.RAW", "BASS6.RAW", "BASS7.RAW", "BASS8.RAW"};
File file;
int numLoaded = 0;
bool isLoading = true;

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

bool asyncOpening = true;

void toggleHeap() {
    loader.toggleHeap();
    numLoaded = 0;
}

void loop() {
    //if (audio_sd_buffered_play_needs_loading) {
        // load from sd to audio buffers
    //} else
    if (numLoaded < 8)
    {
        if (asyncOpening) {
            file = SD.open(fileNames[numLoaded]);
            samples[numLoaded] = loader.beginAsyncLoad(file);
            asyncOpening = false;
        } else if (loader.continueAsyncLoadPartial()) {
            numLoaded++;
            file.close();
            asyncOpening = true;
        }
    }

    unsigned currentMillis = millis();
    if (currentMillis > lastSamplePlayed + 500) {
        if (!rraw_a1.isPlaying()) {
            rraw_a1.playRaw(kicksample->sampledata, kicksample->samplesize/2, 1);
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

void std::__throw_length_error(char const*) {

}