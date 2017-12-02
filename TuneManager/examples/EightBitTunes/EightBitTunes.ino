#include "TuneManager.h"

// RADIO SETTINGS
boolean radioOn = true;
int buzzer_pin = 9;

TuneManager songManager("/tunes/", buzzer_pin);
//TuneManager songManager("/tunes/"); Defaults to pin A0

void setup() {
  Serial.begin(9600); // set up Serial library at 9600 bps
}

void loop() {
  manageTunes();
}

void manageTunes() {
  if (radioOn) {
    // only consider playing music if the radio is 'on'
    songManager.playTunes();
  }
}



