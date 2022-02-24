#include <Arduino.h>
#include "main.h"
 
void setup()
{
  ClockTime = clockinterval;
  seconds = ClockTime/ 1000;
  minutes = seconds / 60;
  seconds %= 60;
  minutes %= 60;
  
  previousTime = ClockTime;
  previousseconds = previousTime / 1000;
  previousminutes = previousseconds / 60;
  previousseconds %= 60;
  previousminutes %= 60;

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
  
  //watch the reset button
  if(ResetButtonState == DIAL_BUTTON_RELEASED && digitalRead(ENCODERBUTTON) == DIAL_BUTTON_PRESSED){
    ResetButtonState = DIAL_BUTTON_PRESSED;
  } else if(ResetButtonState == DIAL_BUTTON_PRESSED && digitalRead(ENCODERBUTTON) == DIAL_BUTTON_RELEASED){
    ClockTime = clockinterval;
    ResetButtonState = DIAL_BUTTON_RELEASED;
  }

  //watch the start/stop button
  if(ClockState == CLOCK_STOPPED && digitalRead(STARTSWITCH) == START_SWITCH_ON){
    ClockState = CLOCK_RUNNING;
    myDFPlayer.play(START_FILE);
    delay(3500);
    currentTime = millis();
    StopTime = currentTime + ClockTime;
  } else if(ClockState == CLOCK_RUNNING && digitalRead(STARTSWITCH) == START_SWTICH_OFF){
    ClockState = CLOCK_STOPPED;
  }

  

  //if clock enabled, run count down
  if(ClockState == CLOCK_RUNNING && ClockTime > 0){ //When clock is counting down...
    if((StopTime - currentTime) > ClockTime){ //watch for overruns where the time on the clock would appear to increase
      ClockTime = 0; //End the clock
    } else {
      ClockTime = StopTime - currentTime; //Update the time on the clock
    }
    
  } else if(ClockState == CLOCK_STOPPED || ClockTime == 0){//When clock is stopped or at zero
    StopTime = currentTime + ClockTime;//Suspend Count Down...
  }

  //calculate clock timing
  seconds = ClockTime/ 1000;
  minutes = seconds / 60;
  seconds %= 60;
  minutes %= 60;

  //On final clock count, sound buzzer
  if(minutes == 0){
    if((previousseconds == 4 && seconds == 3) || 
        (previousseconds == 3 && seconds == 2) || 
        (previousseconds == 2 && seconds == 1) || 
        (previousseconds == 1 && seconds == 0))
      {
        myDFPlayer.play(BUZZER_FILE);
      }
  }

  Serial.printf(" Stop Time %lu Current Time %lu Clock Time %lu ", StopTime, currentTime, ClockTime);

  //output clock timing to rgb clock display
  CLKrgb(ClockTime/2, clockinterval);
  CLKdisplay(minutes*100 + seconds);

  //output clock timing to oled display
  OLEDdisplay.setCursor(0,0); // Start at top-left corner
  OLEDdisplay.println();
  OLEDdisplay.printf(" Time - %lu:%.2lu\n", minutes, seconds);
  OLEDdisplay.printf(" Clock : %d\n", ClockState);
  OLEDdisplay.printf(" Reset : %d\n", ResetButtonState);
  OLEDdisplay.display();

  //output clock timing to serial port
  Serial.printf(" Time - %lu:%.2lu", minutes, seconds);
  Serial.printf(" PTime - %lu:%.2lu", previousminutes, previousseconds);
  //Serial.printf(" Clock : %d", ClockState);
  //Serial.printf(" Reset : %d ", ResetButtonState);

  //output mp3 player data to serial
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  } else {
    Serial.println();
  }

  //don't run too fast!
  delay(delayval);
  
  //record time used in this loop for next time
  previousTime = ClockTime;
  
  //calculate previous clock timing
  previousseconds = previousTime / 1000;
  previousminutes = previousseconds / 60;
  previousseconds %= 60;
  previousminutes %= 60;
}

/****************************************************************************************
 * RGB Display Functions
*****************************************************************************************/
void CLKrgb(int angle, int range){
  angle = map(angle, 0, RANGE, 0, 360);
  angle = constrain(angle, 0, 360);
  if (angle<60)       {red = 255;                green = rainbow[angle];     blue = 0;} 
  else if (angle<120) {red = rainbow[120-angle]; green = 255;                blue = 0;} 
  else if (angle<180) {red = 0,                  green = 255;                blue = rainbow[angle-120];} 
  else if (angle<240) {red = 0,                  green = rainbow[240-angle]; blue = 255;} 
  else if (angle<300) {red = rainbow[angle-240], green = 0;                  blue = 255;} 
  else                {red = 255,                green = 0;                  blue = rainbow[360-angle];} 

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

void CLKdisplay(int value){
    int thos = value/1000;
    int huns = value/100 - 10*thos;
    int tens = value/10 - 10*huns - 100*thos;
    int ones = value - 10*tens - 100*huns - 1000*thos;
    //Serial.print(" thos: ");
    //Serial.print(thos);
    //Serial.print(" huns: ");
    //Serial.print(huns);
    //Serial.print(" tens: ");
    //Serial.print(tens);
    //Serial.print(" ones: ");
    //Serial.print(ones);
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

/****************************************************************************************
 * MP3 Player Functions
*****************************************************************************************/
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