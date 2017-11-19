#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <Time.h>
#include <TimeLib.h>
#include <IRremote.h>
#include <RGBMood.h>
#include "pitches.h"

// initialize the library by associating LCD interface pin
// with the arduino pin number it is connected to:
const int rs = 12, en = 11, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd (rs, en, d4, d5, d6, d7);

// set up the bluetooth serial port:
const int rx = 3, tx = 2;
SoftwareSerial BTSerial(tx, rx);

// set up the infrared receiver:
const int recv = 10;
IRrecv irrecv(recv);
decode_results results;

// set up the rgb led mood:
const int red = 13, green = 1, blue = 0;
RGBMood mood(red, green, blue);

// set up the notes in the melody and its duration
// notes in the melody:
int melody[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};
int nNotes = 8;

// declare constants:
const int tsensor = 8;    // touch sensor on digital pin 8
const int buzzer = 9;     // buzzer on digital pin 9

// initialize to a default date and time:
int mo = 1, dy = 1, yr = 0, hr = 0, mi = 0, sc = 0;

// declare variables:
int val1, val2;
int TS_state = LOW;
int wait_mode = LOW;
int numFeedings = 0;
int LED_state = HIGH;
int LIMIT = 3;


void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // set the pin mode:
  pinMode(tsensor, INPUT);
  pinMode(buzzer, OUTPUT);

  // set up serial library at 9600 bps:
  Serial.begin(9600);

  // set up bluetooth serial library at 9600 bps:
  BTSerial.begin(9600);

  // enable IR receiver:
  irrecv.enableIRIn();

  // set up the LED mood settings
  mood.setMode(RGBMood::RAINBOW_HUE_MODE);
  mood.setFadingSteps(200);
  mood.setFadingSpeed(25);
  mood.setHoldingTime(0);
  mood.fadeHSB(0, 255, 255);

  // set up date via IR receiver input:
  setupDate();
  updateDateAndTime();
}

void loop() {
  if(LED_state == HIGH) {
    mood.tick();  // turn on LED
  }
  
  if(irrecv.decode(&results)) {
    char IRkey = translateIR();
    
    if(IRkey == 'P') {
      // turn LED on/off
      LED_state = !LED_state;
    }
    else if(IRkey == '-') {
      // turn off music
      noTone(buzzer);
    }

    // receive the next value:
    irrecv.resume();
  }
  
  if(wait_mode == HIGH) {
    // to receive signal from other Arduino
    while(BTSerial.available() > 0) {
      char BTkey = BTSerial.read();
      
      if(BTkey == 'T') {
        // get input of date and time from other Arduino:
        hr = BTSerial.read();
        mi = BTSerial.read();
        sc = BTSerial.read();
        
        // update the date and time:
        setTime(hr, mi, sc, dy, mo, yr);
      }
      else if(BTkey == 'F') {
        numFeedings++;
        wait_mode = LOW;
      }
    }
  }
  else {
    // to send signal to receiving Arduino
    val1 = digitalRead(tsensor);        // read input value and store into val1
    delay(10);
    val2 = digitalRead(tsensor);        // read input value again to check for bounces
  
    if(val1 == val2) {                  // make sure the 2 readings are consistent
      if(val1 != TS_state) {            // check if state has changed
        if(val1 == HIGH) {              // check if touch sensor was triggered
          
          if(numFeedings < LIMIT) {
            Serial.println("Feed Me");  // send message
            BTSerial.write("1");        // send a signal
            wait_mode = HIGH;
            
            // play a short melody:
            for(int i = 0; i < nNotes; i++) {
              int noteDuration = 1000 / noteDurations[i];
              tone(buzzer, melody[i], noteDuration);

              delay(noteDuration * 1.30);
              noTone(buzzer);
            }
          }
          
          // new day, restart count to activate touch sensor:
          if(hour() == 0 && minute() == 0 && second() == 0)
          {
            numFeedings = 0;
          }
        }
        
        TS_state = val1;
      }
    }
  }
  
  updateDateAndTime();
}

void setupDate() {
  int count = 0;
  int digit = -1;
  
  while(true) {
    char rec = 'Z';

    if(irrecv.decode(&results)) {
      rec = translateIR();
      digit = isDigit(rec);
      irrecv.resume();
      
      if(count == 0) {  // month
        if(rec == '0' || digit == 1) {
          mo = digit * 10;
          count++;
        }
      }
      else if(count == 1) {
        if((mo == 0 && digit > 0 && digit < 10) ||
           (mo == 10 && (rec == '0' || digit == 1 || digit == 2))) {
          mo += digit;
          count++;
        }
      }
      else if(count == 2) {  // day
        if(rec == '0' || (digit > 0 && digit < 4)) {
          dy = digit * 10;
          count++;
        }
      }
      else if(count == 3) {
        if((dy == 0 && digit > 0 && digit < 10) ||
           (dy == 30 && (rec == '0' || digit == 1)) ||
           (rec == '0' || (digit > 0 && digit < 10))) {
          dy += digit;
          count++;
        }
      }
      else if(count == 4) {  // year
        if(rec == '0' || (digit > 0 && digit < 10)) {
          yr = digit * 10;
          count++;
        }
      }
      else if(count == 5) {
        if(rec == '0' || (digit > 0 && digit < 10)) {
          yr += digit;
          break;
        }
      }
    }
  }
}

void updateDateAndTime() {
  lcd.setCursor(0, 0);
  
  if(month() < 10) {
    lcd.print("0");
  }
  lcd.print(month());
  lcd.print("/");
  
  if(day() < 10) {
    lcd.print("0");
  }
  lcd.print(day());
  lcd.print("/");

  lcd.print(year());
  lcd.print(" ");

  lcd.print("(");
  lcd.print(numFeedings);
  lcd.print(")");

  lcd.setCursor(0, 1);
  
  if(hourFormat12() < 10) {
    lcd.print("0");
  }
  lcd.print(hourFormat12());
  lcd.print(":");

  if(minute() < 10) {
    lcd.print("0");
  }  
  lcd.print(minute());
  lcd.print(":");

  if(second() < 10) {
    lcd.print("0");
  }
  lcd.print(second());
  lcd.print(" ");
  
  if(isPM()) {
    lcd.print("PM");
  }
  else {
    lcd.print("AM");
  }
}

char translateIR() { 
  // takes action based on IR code received
  switch(results.value) 
  {
    case 0xFF22DD: return 'B';  // PREV
    case 0xFF02FD: return 'F';  // NEXT
    case 0xFFC23D: return 'P';  // PLAY/PAUSE
    case 0xFFE01F: return '-';  // VOL-
    case 0xFFA857: return '+';  // VOL+
    case 0xFF6897: return '0'; 
    case 0xFF30CF: return '1'; 
    case 0xFF18E7: return '2'; 
    case 0xFF7A85: return '3'; 
    case 0xFF10EF: return '4'; 
    case 0xFF38C7: return '5'; 
    case 0xFF5AA5: return '6'; 
    case 0xFF42BD: return '7'; 
    case 0xFF4AB5: return '8'; 
    case 0xFF52AD: return '9'; 
    default:       return 'Z';  // Other
  } 
}
