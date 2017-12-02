#ifndef TUNEMANAGER_H
#define TUNEMANAGER_H

#include "Arduino.h"


// HARDWARE CONSTANTS
// pin of speaker/pezo
static int PIN_PEZO;
static const int PIN_CS = 10;
static const int PIN_HARDWARE_SS = 10;

// Buffer Constants
static const int MIN_NOTE_BUFFER = 1;
static const int MAX_NOTE_BUFFER = 5;

class TuneManager {
  public:
    TuneManager(char tuneFolderPath[], int buzzer = A0); 
    void playTunes();
  private:
    void addNotesToTune(int numOfNotesToAdd);
    void ensureFile();
};

#endif



