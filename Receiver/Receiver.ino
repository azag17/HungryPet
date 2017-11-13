#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <TimeLib.h>
#include <IRremote.h>
#include <TuneManager.h>

TuneManager* songManager = NULL;
boolean alert = false;

LiquidCrystal lcd (9, 8, 7, 6, 5, 4);
SoftwareSerial BTserial(3, 2);

IRrecv irrecv(A1);
decode_results results;

int redLED = A2;
int greenLED = A3;
int blueLED = A4;

unsigned long last_screen_refresh = 0;
unsigned long last_led_refresh = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(redLED, OUTPUT);
  digitalWrite(redLED, LOW);
  pinMode(greenLED, OUTPUT);
  digitalWrite(greenLED, LOW);
  pinMode(blueLED, OUTPUT);
  digitalWrite(blueLED, LOW);
  lcd.begin(16, 2);
  BTserial.begin(9600);
  irrecv.enableIRIn();
  Serial.begin(9600);
  setupTime();
  songManager = new TuneManager("/tunes");
}

void loop() {

  if(BTserial.available()) {
    if(BTserial.read() == '1')
      alert = true;
  }

  if(alert) {
    if(songManager)
      songManager->playTunes();
    cycleLED();
  }
    
  if (irrecv.decode(&results)) {
    if(translateIR() == '-') {
      alert = false;
      digitalWrite(redLED, LOW);
      digitalWrite(greenLED, LOW);
      digitalWrite(blueLED, LOW);
    }
    irrecv.resume(); // receive the next value
  }

  if(millis() - last_screen_refresh > 1000) {
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print(hourFormat12());
    lcd.print(":");
    lcd.print(minute());
    lcd.print(":");
    lcd.print(second());
    lcd.print(" ");
    if(isAM())
      lcd.print("AM");
    else
      lcd.print("PM");
    last_screen_refresh = millis();
  }
}

void cycleLED() {
  if(millis() - last_led_refresh >= 250) {
    if(digitalRead(redLED) == HIGH) {
      digitalWrite(redLED, LOW);
      digitalWrite(blueLED, LOW);
      digitalWrite(greenLED, HIGH);
    }
    else if(digitalRead(greenLED) == HIGH) {
      digitalWrite(greenLED, LOW);
      digitalWrite(redLED, LOW);
      digitalWrite(blueLED, HIGH);
    }
    else {
      digitalWrite(blueLED, LOW);
      digitalWrite(greenLED, LOW);
      digitalWrite(redLED, HIGH);
    }
    last_led_refresh = millis();
  }
  
}

char translateIR() { 
  // takes action based on IR code received
  switch(results.value) {
  
  case 0xFF22DD:  
    return 'B'; 

  case 0xFF02FD:  
    return 'F'; 

  case 0xFFC23D:  
    return 'P'; 

  case 0xFFE01F:  
    return '-'; 

  case 0xFFA857:  
    return '+'; 

  case 0xFF6897:  
    return '0'; 

  case 0xFF30CF:  
    return '1'; 

  case 0xFF18E7:  
    return '2'; 

  case 0xFF7A85:  
    return '3'; 

  case 0xFF10EF:  
    return '4'; 

  case 0xFF38C7:  
    return '5'; 

  case 0xFF5AA5:  
    return '6'; 

  case 0xFF42BD:  
    return '7'; 

  case 0xFF4AB5:  
    return '8'; 

  case 0xFF52AD:  
    return '9'; 

  default: 
    return 'Z';
  } 
}

void setupTime() {
  int cursorLocation = 0; 
  String Time = "  :  :   AM";

  lcd.setCursor(3,0);
  lcd.print("Input Time");
  lcd.setCursor(3, 1);
  lcd.print(Time);
  lcd.setCursor(cursorLocation+3, 1);
  lcd.blink();
  int hour_ = 0;
  while(true) {
    char rec = 'Z';
    if (irrecv.decode(&results)) {
      rec = translateIR();
      irrecv.resume(); // receive the next value
      if(rec == 'Z' || rec == '-' || rec == '+') { continue; }
    
      if(rec == 'P'){
        if(!isDigit(Time[0]) || !isDigit(Time[1]) || Time.substring(0,2).toInt() < 1 || Time.substring(0,2).toInt() > 12) {
          Serial.println("Invalid hour");
          Time = "  :  :   AM";
          lcd.setCursor(3, 1);
          lcd.print(Time);
          cursorLocation = 0;
          lcd.setCursor(cursorLocation+3, 1);
          continue;
        }
        else {
          hour_ = Time.substring(0,2).toInt();
          if(Time[9] == 'P' && hour_ != 12)
            hour_ += 12;
          else if (Time[9] == 'A' && hour_ == 12)
            hour_ -= 12;
        }
          
        if(!isDigit(Time[3]) || !isDigit(Time[4]) || Time.substring(3,5).toInt() < 0 || Time.substring(3,5).toInt() > 59) {
          Serial.println("Invalid minute");
          Time = "  :  :  AM";
          lcd.setCursor(3, 1);
          lcd.print(Time);
          cursorLocation = 0;
          lcd.setCursor(cursorLocation+3, 1);
          continue;
        }
        
        if(!isDigit(Time[6]) || !isDigit(Time[7]) || Time.substring(6,8).toInt() > 59) {
          Serial.println("Invalid seconds");
          Time = "  :  :  AM";
          lcd.setCursor(3, 1);
          lcd.print(Time);
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
        if(cursorLocation < 7) {
          cursorLocation++;
          if(cursorLocation == 2 || cursorLocation == 5) {
            cursorLocation++;
          }
          lcd.setCursor(cursorLocation+3, 1);
        }
        else {
          if (Time[9] == 'A')
            Time[9] = 'P';
          else {
            Time[9] = 'A';
          }
          lcd.setCursor(3, 1);
          lcd.print(Time);
          lcd.setCursor(cursorLocation+3, 1);
        }
      }

      else {
        Time[cursorLocation] = rec;
        if(cursorLocation < 7) {
          cursorLocation++;
          if(cursorLocation == 2 || cursorLocation == 5) {
            cursorLocation++;
          }
        }
        lcd.setCursor(3, 1);
        lcd.print(Time);
        lcd.setCursor(cursorLocation+3, 1);
      }
      delay(500);
    }
  }
  lcd.noBlink();
  lcd.clear();
  setTime(hour_, Time.substring(3,5).toInt(), Time.substring(6,8).toInt(), 1, 1, 1);
}

