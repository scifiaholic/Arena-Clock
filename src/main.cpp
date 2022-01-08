#include <Arduino.h>
#include "main.h"
 
void setup()
{
  //Serial setup
  Serial.begin(115200); 
  Serial.println();

  //Neopixel setup
  pixels.begin();
  pixels.clear(); 

  pinMode(STARTSWITCH, INPUT);
  pinMode(ENCODERBUTTON, INPUT);

  //OLED setup
  while(!OLEDdisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed... trying again..."));
  }
  Serial.println(F("OLED display online."));

  OLEDdisplay.clearDisplay();
  OLEDdisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  OLEDdisplay.setTextSize(1);             // Normal 1:1 pixel scale
  OLEDdisplay.setCursor(0,0);             // Start at top-left corner
  OLEDdisplay.println(F("Time on the Clock:"));

  OLEDdisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK);        // Draw white text
  OLEDdisplay.println(RANGE);

  OLEDdisplay.display();

  //MP3 setup
  mySoftwareSerial.begin(9600);
  
  while(!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("DFPlayer Unable to begin... board connected? SD card?"));
    delay(100);
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  //myDFPlayer.play(1);  //Play the first mp3

}

void loop()
{
  currentTime = millis();
  if(JudgeState == 0 && digitalRead(ENCODERBUTTON) == 1){
    JudgeState = 1;
  } else if(JudgeState == 1 && digitalRead(ENCODERBUTTON) == 0){
    ClockTime = clockinterval;
    JudgeState = 0;
  }

  if(ClockState == 0 && digitalRead(STARTSWITCH) == 1){
    ClockState = 1;
  } else if(ClockState == 1 && digitalRead(STARTSWITCH) == 0){
    ClockState = 0;
  }

  if(ClockState == 0 && ClockTime > 0){ //Counting Down...
    if((StopTime - currentTime) < ClockTime){
      ClockTime = StopTime - currentTime;
    } else {
      ClockTime = 0;
    }
    Serial.printf(" Clock Time %lu = %lu - %lu ", ClockTime, currentTime, StopTime);
  } else if(ClockState == 1 || ClockTime == 0){//Suspend Count Down...
    StopTime = currentTime + ClockTime;
    Serial.printf(" Stop Time %lu = %lu + %lu ", StopTime, currentTime, ClockTime);
  } 

  unsigned long previousseconds = previousTime / 1000;
  unsigned long previousminutes = previousseconds / 60;
  previousseconds %= 60;
  previousminutes %= 60;

  unsigned long seconds = ClockTime / 1000;
  unsigned long minutes = seconds / 60;
  seconds %= 60;
  minutes %= 60;

  CLKrgb(ClockTime, clockinterval);
  CLKdisplay(minutes*100 + seconds);
  
  OLEDdisplay.setCursor(0,0); // Start at top-left corner
  OLEDdisplay.println();
  OLEDdisplay.printf(" Time - %lu:%.2lu\n", minutes, seconds);
  OLEDdisplay.printf(" Clock : %d\n", ClockState);
  OLEDdisplay.printf(" Judge : %d\n", JudgeState);
  OLEDdisplay.display();

  Serial.printf(" Time - %lu:%.2lu", minutes, seconds);
  Serial.printf(" Clock : %d", ClockState);
  Serial.printf(" Judge : %d\n", JudgeState);
  
  delay(delayval);

  if(!myDFPlayer.available() && minutes == 0){
    if((previousseconds == 4 && seconds == 3) || 
       (previousseconds == 3 && seconds == 2) || 
       (previousseconds == 2 && seconds == 1) || 
       (previousseconds == 1 && seconds == 0)){
        myDFPlayer.play(1);  //Play the first mp3
       }
  }

  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }

  previousTime = ClockTime;
}

void CLKsetdigit(int digit, int number){
  for (int i = 0; i < 14; i += 2) {
    if(segs[number][i/2]){
      pixels.setPixelColor(digits[digit][i+1]-1, pixels.Color(red, green, blue));
      pixels.setPixelColor(digits[digit][i]-1, pixels.Color(red, green, blue));
    } else {
      pixels.setPixelColor(digits[digit][i+1]-1, pixels.Color(0, 0, 0));
      pixels.setPixelColor(digits[digit][i]-1, pixels.Color(0, 0, 0));
    }
    pixels.show();
  }
}

void CLKrgb(int angle, int range){
  angle = map(angle, 0, RANGE, 0, 360);
  if (angle<60)       {red = 255;                green = rainbow[angle];     blue = 0;} 
  else if (angle<120) {red = rainbow[120-angle]; green = 255;                blue = 0;} 
  else if (angle<180) {red = 0,                  green = 255;                blue = rainbow[angle-120];} 
  else if (angle<240) {red = 0,                  green = rainbow[240-angle]; blue = 255;} 
  else if (angle<300) {red = rainbow[angle-240], green = 0;                  blue = 255;} 
  else                {red = 255,                green = 0;                  blue = rainbow[360-angle];} 

}

void CLKdisplay(int value){
    int thos = value/1000;
    int huns = value/100 - 10*thos;
    int tens = value/10 - 10*huns - 100*thos;
    int ones = value - 10*tens - 100*huns - 1000*thos;
    Serial.print(" thos: ");
    Serial.print(thos);
    Serial.print(" huns: ");
    Serial.print(huns);
    Serial.print(" tens: ");
    Serial.print(tens);
    Serial.print(" ones: ");
    Serial.print(ones);
    CLKsetdigit(0,ones);
    
    if(tens || huns || thos){
      CLKsetdigit(1, tens);
    }else{
      CLKsetdigit(1, 10);
    }
    
    if(huns || thos){
      CLKsetdigit(2, huns);
    }else{
      CLKsetdigit(2, 10);  
    }

    if(thos){
      CLKsetdigit(3, thos);
    }else{
      CLKsetdigit(3, 10);
    }
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  
}