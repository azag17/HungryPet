#include <SD.h>
#include "ABCNoteParser.h"
#include "TuneManager.h"

// File information
File tuneFile;
File root;

// Buffered note storage/parsing
ABCNoteParser* abcParser;
unsigned int readNoteIndex = 0;
unsigned int writeNoteIndex = 0;
int tuneFreq[MAX_NOTE_BUFFER];
int tuneDur[MAX_NOTE_BUFFER];

// Timing
unsigned long previousMillis = 0;
unsigned long interval = 0;

TuneManager::TuneManager(char tuneFolderPath[], int buzzer = A0) {
  Serial.println(F("Initializing TuneManager"));
  previousMillis = millis();
  abcParser = new ABCNoteParser();
  
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the SD library functions will not work.
  pinMode(PIN_HARDWARE_SS, OUTPUT);
  
  if (!SD.begin(PIN_CS)) {
    Serial.println(F("SD Card Initialization Failed"));
  } else {
    Serial.println(F("SD Card Initialized Successfully"));
  }
  
  // Open the root folder so we can rely on 'openNextFile' for our playlist 'shuffle'
  root = SD.open(tuneFolderPath);
  PIN_PEZO = buzzer;
}

void TuneManager::addNotesToTune(int numOfNotesToAdd) {
  for (int i=0; i<numOfNotesToAdd; i++) {
    // Immediately abort loading more notes if we have run out of space in the buffer
    if ((writeNoteIndex+1)%MAX_NOTE_BUFFER == readNoteIndex) return;
    
    // If the file is still available, get the next freq and duration
    if (tuneFile.available()) {
      // Get our next note from the file
      int nextNoteFreq = 0;
      int nextNoteDur = 0;
      
      // get the next note using our parser
      abcParser->getNextNote(&tuneFile, &nextNoteFreq, &nextNoteDur);
      
      // Check to make sure that we have a note (in case it reached end of file)
      if (nextNoteDur != 0) {
        // Otherwise, add our decoded note to the ongoing tune
        tuneFreq[writeNoteIndex] = nextNoteFreq;
        tuneDur[writeNoteIndex] = nextNoteDur;
        
        // Increment the index of where we are writing our notes
        writeNoteIndex = (writeNoteIndex+1)%MAX_NOTE_BUFFER;
        
        // So long as we were able to get the next note, continue with the parsing process
        continue;
      } 
    } 
    // If we have reached the end of the song, return to avoid trying to load more notes
    return;
  }
}

// "Ensures" that the given file pointer will indeed point to a file, if one is available
void TuneManager::ensureFile() {
  if (!tuneFile || !tuneFile.available()) {
  	if(tuneFile && !tuneFile.available())
  		tuneFile.close();
    // Proceed to the next song if it exists
    tuneFile = root.openNextFile();
    if(!tuneFile || !tuneFile.available()) {
      root.rewindDirectory();
    }
    // We got a new tune, load up some notes from it
    if (tuneFile) {
      //Serial.print(F("Loaded next tune file: "));
      //Serial.println(tuneFile.name());
      // Reset our parser to it's defaults (don't want previous songs settings)
      abcParser->reset();
      // Fill up our buffer with the initial set of music
      this->addNotesToTune(MAX_NOTE_BUFFER);
    }
  }
}

void TuneManager::playTunes() {
  // Make sure we have a song to play
  this->ensureFile();
  
  // play the song by iterating over the notes at given intervals:
  unsigned long currentMillis = millis();
  if (readNoteIndex != writeNoteIndex && (currentMillis - previousMillis > interval) ) {
    previousMillis = currentMillis;
  
    // Before playing, if this and the next notes are the same frequency, we need to manually
    // add a small break between the notes so they don't blend together
    int tempDur = tuneDur[readNoteIndex];
    if (tuneFreq[(readNoteIndex+1)%MAX_NOTE_BUFFER] == tuneFreq[readNoteIndex]) {
      // Remove some milliseconds to the notes interval to create a short 'rest'
      tempDur -= 10;
    }
  
    // Play the note
    tone(PIN_PEZO, tuneFreq[readNoteIndex], tempDur);
    
    // Set how long to wait until next note 
    interval = tuneDur[readNoteIndex];
     
    // Remove the note we just played so it doesn't repeat

    readNoteIndex = (readNoteIndex+1)%MAX_NOTE_BUFFER;
  } else {
    // If we can't play a note yet, might as well buffer some of the upcoming notes
    //Serial.println("Not playing a note, so add to buffer");
    addNotesToTune(MIN_NOTE_BUFFER);
  } 
}
