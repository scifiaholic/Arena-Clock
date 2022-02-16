#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

#define ENCODERPULSE A0
#define ENCODERDIRECTION D0
//D1 I2C SCL
//D2 I2C SDA
#define NEOPIXELPIN D3
#define STARTSWITCH D4
#define ENCODERBUTTON D5
#define TXPIN D6
#define RXPIN D7
#define JUDGESWITCH D8

int delayval = 50;

unsigned long clockinterval = 5000; //milliseconds equal to 3 minutes
unsigned long currentTime = millis(); //current time on the internal clock
unsigned long previousTime = millis();
unsigned long StopTime = millis(); //Time Clock was started
unsigned long ClockTime = clockinterval; //Time displayed on the clock
int ClockState = 0;
int JudgeState = 0;

/****************************************************************************************
 * Neopixel Clock Display
*****************************************************************************************/

#define NUMPIXELS 60
#define RANGE 9999

//map led number to its segment in each digit
int digit1[14] = { 5,  6,  3,  4, 14, 15, 12, 13, 10, 11,  7,  8,  1,  2};
int digit2[14] = {25, 26, 23, 24, 20, 21, 18, 19, 16, 17, 27, 28, 29, 30};
int digit3[14] = {35, 36, 33, 34, 44, 45, 42, 43, 40, 41, 37, 38, 31, 32};
int digit4[14] = {55, 56, 53, 54, 50, 51, 48, 49, 46, 47, 57, 58, 89, 60};
int* digits[4] = {digit4, digit3, digit2, digit1};

//segment 1/0:on/off for each digit: 
//             A  B  C  D  E  F  G
int seg0[] = { 1, 1, 1, 1, 1, 1, 0};
int seg1[] = { 0, 1, 1, 0, 0, 0, 0};
int seg2[] = { 1, 1, 0, 1, 1, 0, 1};
int seg3[] = { 1, 1, 1, 1, 0, 0, 1};
int seg4[] = { 0, 1, 1, 0, 0, 1, 1};
int seg5[] = { 1, 0, 1, 1, 0, 1, 1};
int seg6[] = { 1, 0, 1, 1, 1, 1, 1};
int seg7[] = { 1, 1, 1, 0, 0, 0, 0};
int seg8[] = { 1, 1, 1, 1, 1, 1, 1};
int seg9[] = { 1, 1, 1, 1, 0, 1, 1};
int seg_[] = { 0, 0, 0, 0, 0, 0, 0};
int* segs[] = {seg0, seg1, seg2, seg3, seg4, seg5, seg6, seg7, seg8, seg9, seg_};

//values that represent a rainbow
int rainbow[61] = 
{0, 4, 8, 13, 17, 21, 25, 30, 34, 38, 42, 47, 51, 55, 59, 64, 68, 72, 76,
81, 85, 89, 93, 98, 102, 106, 110, 115, 119, 123, 127, 132, 136, 140, 144,
149, 153, 157, 161, 166, 170, 174, 178, 183, 187, 191, 195, 200, 204, 208,
212, 217, 221, 225, 229, 234, 238, 242, 246, 251, 255};

//current display color values
int red = 255;
int green = 255;
int blue = 255;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

void CLKsetdigit(int digit, int number);

void CLKrgb(int angle, int range);

void CLKdisplay(int value);

/****************************************************************************************
 * SSD1306 128x64 0.96" Display
*****************************************************************************************/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 OLEDdisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/****************************************************************************************
 * MP3 Player
*****************************************************************************************/

SoftwareSerial mySoftwareSerial(D7, D6); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);
int PlayedSound = 0;
