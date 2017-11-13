#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <Time.h>
#include <TimeLib.h>
#include <IRremote.h>
#include <RGBMood.h>


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

// declare constants:
const int tsensor = 8;    // touch sensor on digital pin 8
const int buzzer = 9;     // buzzer on digital pin 9

// initialize to a default date and time:
int mo = 1, dy = 1, yr = 0, hr = 0, mi = 0, sc = 0;

// declare variables:
int val1, val2;
int TS_State = LOW;
int receive_mode = LOW;
char fed_signal;
int numFeedings = 0;
int buttonPressed;
char bp1;


void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // set the pin mode:
  pinMode(tsensor, INPUT);
  pinMode(buzzer, OUTPUT);

  // set up serial library at 9600 bps:
  Serial.begin(9600);

  // set up bluetooth serial library at 38400 bps:
  BTSerial.begin(38400);

  irrecv.enableIRIn();
}


void loop() {
  if(receive_mode == HIGH) {
    // to receive signal from other Arduino
    while(BTSerial.available() > 0) {
      fed_signal = Serial.read();

      if(fed_signal == '1') {
        numFeedings++;
        displayFeedSchedule();
      }
    }
  }
  else {
    // to send signal to receiving Arduino
    val1 = digitalRead(tsensor);   // read input value and store into val1
    delay(10);
    val2 = digitalRead(tsensor);   // read input value again to check for bounces
  
    if(val1 == val2) {              // make sure the 2 readings are consistent
      if(val1 != TS_State) {        // check if state has changed
        if(val1 == HIGH) {          // check if touch sensor was triggered
          Serial.write("1");        // send a signal
          receive_mode = HIGH;
        }
        
        TS_State = val1;
      }
    }
  }

  // need to reset numFeedings at the end of each day
  // need to sync current system time to arduino
  setTime(hr, mi, sc, dy, mo, yr);
}

void displayFeedSchedule() {  
  lcd.setCursor(0, 0);
  lcd.print(monthShortStr(month()));
  lcd.print(" ");
  
  if(day() < 10) {
    lcd.print("0");
  }
  lcd.print(day());

  lcd.print(" (");
  lcd.print(numFeedings);
  lcd.print(")");
  
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

void translateIR() {
  switch(results.value) {
    
  case 0xFFC23D:  
    bp1 = 'p';
    break;

  case 0xFFE01F:  
    bp1 = '-';
    break;

  case 0xFFA857:  
    bp1 = '+';
    break;

  case 0xFF6897:  
    buttonPressed = 0;
    break;
    
  case 0xFF30CF:  
    buttonPressed = 1;
    break;

  case 0xFF18E7:  
    buttonPressed = 2;
    break;

  case 0xFF7A85:  
    buttonPressed = 3;
    break;

  case 0xFF10EF:  
    buttonPressed = 4;
    break;

  case 0xFF38C7:  
    buttonPressed = 5;
    break;

  case 0xFF5AA5:  
    buttonPressed = 6;
    break;

  case 0xFF42BD:  
    buttonPressed = 7;
    break;

  case 0xFF4AB5:  
    buttonPressed = 8;
    break;

  case 0xFF52AD:  
    buttonPressed = 9;
    break;

  default: 
    buttonPressed = -99;

  }

  delay(500);
}
