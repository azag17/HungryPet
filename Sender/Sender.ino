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
int mo = 1, dy = 1, yr = 0, hr = 12, mi = 0, sc = 0;

// declare variables:
int val1, val2;
int TS_state = LOW;
int wait_mode = LOW;
int numFeedings = 0;
int LIMIT = 3;


void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();

  // set the pin mode:
  pinMode(tsensor, INPUT);
  pinMode(buzzer, OUTPUT);

  // set up serial library at 9600 bps:
  // Serial.begin(9600);

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
}

void loop() {
  mood.tick();  // turn on LED

  // new day, restart count to activate touch sensor:
  if(hour() == 0 && minute() == 0 && second() == 0)
  {
    numFeedings = 0;
    hr = 0; mi = 0; sc = 0;
  }
  
  if(wait_mode == HIGH) {
    // to receive signal from other Arduino
    if(BTSerial.available() >= 4) {
      char BTkey = BTSerial.read();
      
      if(BTkey == 'F')
      {
        // get input of time from other Arduino:
        hr = BTSerial.read();
        mi = BTSerial.read();
        sc = BTSerial.read();
        
        // set the date and time:
        setTime(hr, mi, sc, dy, mo, yr);
        
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
            // Serial.println("Feed Me");  // send message
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
        }
        
        TS_state = val1;
      }
    }
  }
  updateDateAndTime();
}

void setupDate() {
  int cursorLocation = 0; 
  String Date = "  /  /    ";
  
  lcd.setCursor(3,0);
  lcd.print("Input Date");
  lcd.setCursor(3, 1);
  lcd.print(Date);
  lcd.setCursor(cursorLocation+3, 1);
  lcd.blink();
  
  while(true) {
    char rec = 'Z';
    if (irrecv.decode(&results)) {
      rec = translateIR();
      irrecv.resume(); // receive the next value
      if(rec == 'Z' || rec == '-' || rec == '+') { continue; }
    
      if(rec == 'P'){
        if(!isDigit(Date[0]) || !isDigit(Date[1]) || Date.substring(0,2).toInt() < 1 || Date.substring(0,2).toInt() > 12) {
          Date = "  /  /    ";
          lcd.setCursor(3, 1);
          lcd.print(Date);
          cursorLocation = 0;
          lcd.setCursor(cursorLocation+3, 1);
          continue;
        }

        int daysInMonth = 0;
        int curMonth = Date.substring(0,2).toInt();
        if(curMonth == 2) {
          int curYear = Date.substring(6,10).toInt();
          if(curYear%4 == 0 && (curYear%100 != 0 || curYear%400 == 0))
            daysInMonth = 29;
          else
            daysInMonth = 28; 
        }
        else if(curMonth < 8) {
          if(curMonth%2) //if odd
            daysInMonth = 31;
          else
            daysInMonth = 30;
          }
        else {
          if(curMonth%2) //if odd
            daysInMonth = 30;
          else
            daysInMonth = 31;
        }
        if(!isDigit(Date[3]) || !isDigit(Date[4]) || Date.substring(3,5).toInt() < 1 || Date.substring(3,5).toInt() > daysInMonth) {
          //Serial.println("Invalid Day");
          Date = "  /  /    ";
          lcd.setCursor(3, 1);
          lcd.print(Date);
          cursorLocation = 0;
          lcd.setCursor(cursorLocation+3, 1);
          continue;
        }
        
        if(!isDigit(Date[6]) || !isDigit(Date[7]) || !isDigit(Date[8]) || !isDigit(Date[9]) ) {
          //Serial.println("Invalid Year");
          Date = "  /  /    ";
          lcd.setCursor(3, 1);
          lcd.print(Date);
          cursorLocation = 0;
          lcd.setCursor(cursorLocation+3, 1);
          continue;
        }
        break;
      }
      
      else if (rec == 'B') {
        if(cursorLocation > 0) {
          cursorLocation--;
          if(cursorLocation == 2 || cursorLocation == 5) {
            cursorLocation--;
          }
          lcd.setCursor(cursorLocation+3, 1);
        }
      }

      else if (rec == 'F') {
        if(cursorLocation < 9) {
          cursorLocation++;
          if(cursorLocation == 2 || cursorLocation == 5) {
            cursorLocation++;
          }
          lcd.setCursor(cursorLocation+3, 1);
        }
      }

      else {
        Date[cursorLocation] = rec;
        if(cursorLocation < 9) {
          cursorLocation++;
          if(cursorLocation == 2 || cursorLocation == 5) {
            cursorLocation++;
          }
        }
        lcd.setCursor(3, 1);
        lcd.print(Date);
        lcd.setCursor(cursorLocation+3, 1);
      }
      delay(500);
    }
  }
  lcd.noBlink();
  lcd.clear();

  dy = Date.substring(3,5).toInt();
  mo = Date.substring(0,2).toInt();
  yr = Date.substring(6,10).toInt();
  
  setTime(hr, mi, sc, dy, mo, yr);
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
  
  if(hr > 0 && hr < 10) {
    lcd.print("0");
  }

  if(hr == 0)
  {
    lcd.print("12");
  }
  else if(hr > 12)
  {
    lcd.print(hr - 12);
  }
  else 
  {
    lcd.print(hr);
  }
  lcd.print(":");

  if(mi < 10) {
    lcd.print("0");
  }  
  lcd.print(mi);
  lcd.print(":");

  if(sc < 10) {
    lcd.print("0");
  }
  lcd.print(sc);
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
