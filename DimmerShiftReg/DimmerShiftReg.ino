#include <Tlc5940.h>
#include <tlc_animations.h>
#include <tlc_config.h>
#include <tlc_fades.h>
#include <tlc_progmem_utils.h>
//#include <tlc_servos.h>
#include <tlc_shifts.h>

/* Example code for driving a single 7-segment display with
 a 74HC595 8-bit shift register

 The code cycles through the the entire 7-segment character
 set that was written by Eli McIlveen:

 (http://www.forgeryleague.com/lab/entry/arduino_7_segment_output/)

 In case of a COMMON ANODE display the bytes need to be inverted
 as shown in the loop() function: bitsToSend = bitsToSend ^ B11111111

 Hardware:

 * 74HC595 shift register attached to pins 2, 3, and 4 of the Arduino,
   as detailed below.

 * Pins to connect to common-cathode LED display via a 74HC595:
   DP-15, A-1, B-2, C-3, D-4, E-5, F-6, G-7 (shiftOut using LSBFIRST)
   Or:
   DP-7, A-6, B-5, C-4, D-3, E-2, F-1, G-15 (shiftOut using MSBFIRST)
 */
/**
 * My bit / segment mappings:
 *
 * bit  segment
 *  0   A   (10000000)
 *  1   B   (01000000)
 *  2   C   (00100000)
 *  3   D   (00010000)
 *  4   E   (00001000)
 *  5   F   (00000100)
 *  6   G   (00000010)
 *  7   DP  (00000001)
 *
 *       +---A---+
 *       |       |
 *       F       B
 *       |       |
         +---G---+
         |       |
         E       C
         |       |
         +---D---+
                    DP
 */ 
byte getSegments(byte b)
{
  byte rval = 0;
  
  if(b >= 0 && b <= 9) {
    char c = '0' + b;
    
    rval = getSegments(c);
  }
  return rval;
}

byte getSegments(char c)
{
  Serial.print("getSegments('");
  Serial.print(c);
  Serial.print("') = B");
  byte rval = 0;
  switch(c) {
    case '0': rval = B11111100; break;
    case '1': rval = B01100000; break;
    case '2': rval = B11011010; break;
    case '3': rval = B11110010; break;
    case '4': rval = B01100110; break;
    case '5': rval = B10110110; break;
    case '6': rval = B10111110; break;
    case '7': rval = B11100000; break;
    case '8': rval = B11111110; break;
    case '9': rval = B11110110; break;
    default: rval =  B10101010; break;
  }
  Serial.println(rval, BIN);
  return rval;
}

//// Pin connected to latch pin (ST_CP) of 74HC595
const int latchPin = 2; //8;
//// Pin connected to clock pin (SH_CP) of 74HC595
const int clockPin = 4; //12;
//// Pin connected to Data in (DS) of 74HC595
const int dataPin  = 7; //11;
//// Pin connected to display's common annode
const int faderPin = 6; //10;

/////////////////////////////////////////////////////////////////////////////////
//
void setup()
{
  Serial.begin(9600);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin,  OUTPUT);
  pinMode(faderPin, OUTPUT);
  
  Tlc.init();
}

/////////////////////////////////////////////////////////////////////////////////
//
void loop()
{
  analogWrite(faderPin, 255);
  for(int i = 0; i < 100; i++)
  {
    int tens = i/10;
    int ones = i - (tens * 10);
    uint8_t bitsToSend1 = getSegments((byte)(ones)); //ledCharSet[i];
    uint8_t bitsToSend2 = getSegments((byte)(tens)); //ledCharSet[i];
    
    //bitSet(bitsToSend, 7);

    // invert bitmas - we're using a common ANODE display.
    // for common cathode, comment out the following line.
    
    bitsToSend1 = bitsToSend1 ^ B11111111; 
    bitsToSend2 = bitsToSend2 ^ B11111111; 

    // turn off the output so the pins don't light up
    // while you're shifting bits:
    digitalWrite(latchPin, LOW);

    //digitalWrite(clockPin, LOW);
    // shift the bits out:
    shiftOut(dataPin, clockPin, LSBFIRST, bitsToSend1);
    shiftOut(dataPin, clockPin, LSBFIRST, bitsToSend2);

    // turn on the output so the LEDs can light up:
    digitalWrite(latchPin, HIGH);

    //delay(5000);
    fadeInOut();
  }
}

/////////////////////////////////////////////////////////////////////////////////
//
const int faderChannel = 0;
const int lcdFaderChannel = 1;

void fadeInOut()
{
  for(int i = 0; i < 255; ++i)
  {
    analogWrite(faderPin, i);
    Tlc.clear();
    //Serial.print("channel: "); Serial.print(faderChannel);
    //Serial.print(" = "); Serial.println(i*16);
    int fade = i*16;
    Tlc.set(faderChannel, fade);
    Tlc.set(lcdFaderChannel, fade);
    Tlc.update();
    delay(1);
  }
  for(int i = 255; i > 0; --i)
  {
    analogWrite(faderPin, i);
    Tlc.clear();
    //Serial.print("channel: "); Serial.print(faderChannel);
    //Serial.print(" = "); Serial.println(i*16);
    int fade = i*16;
    Tlc.set(faderChannel, fade);
    Tlc.set(lcdFaderChannel, fade);
    Tlc.update();
    delay(1);
  }
}
