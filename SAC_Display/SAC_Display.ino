#include <LcdDisplayModes.h>
#include <SAC_LcdDisplay.h>

#include <LiquidCrystal_SR3W.h>

#include <SAC_Tlc5940.h>
#include <SAC_ClockDisplay.h>
//
// WIRING FOR TLC5940 in SAC_ClockDisplay
//
//  3 - GSCLK (Purple)
//  9 - XLAT  (Yellow)
// 10 - BLANK (Blue)
// 11 - SIN   (Orange)
// 13 - SCLK  (Green)
//

#define SAC_DISPLAY 1
#undef SAC_MASTER

#include <SwissArmyClock.h>

#include <Wire.h>
#include <RTClib.h>

byte led_display_mode = TIME_MODE;

// PIN DEFINITIONS
// const int colonPin = 6;

const uint8_t LCD_LATCH = 4;  // Green
const uint8_t LCD_DATA = 5;   // Blue
const uint8_t LCD_CLOCK = 2;  // Yellow

const int LCD_Red = 32;
const int LCD_Green = 33;
const int LCD_Blue = 34;

const int COLON1 = 35;
const int COLON2 = 36;

const int DISPLAY_UP_PIN = 7;

/**
 * Swiss Army Clock Display Module
 */
 SAC_ClockDisplay clockDisplay(COLON1, COLON2);
 
 // I2C stuff
 byte dataIn[1 + sizeof(float) * 4] = {};
 byte dataOut[3] = {};
 boolean dataReady;
 
//LiquidCrystal_SR3W _lcd(LCD_DATA,  LCD_CLOCK, LCD_LATCH);
SAC_LcdDisplay lcd(LCD_DATA,  LCD_CLOCK, LCD_LATCH, LCD_Red, LCD_Green, LCD_Blue, &clockDisplay);
 
 void setup()
 {
   //pinMode(DISPLAY_UP_PIN, OUTPUT);
#if defined(DEBUG)
   Serial.begin(9600);
#endif // DEBUG
   DebugLn("SETUP");
   Wire.begin(display_address);          // join I2C bus using this address
   dataReady = false;
   Wire.onRequest(handleRequest);
   Wire.onReceive(handleReceive); // register event handler
   clockDisplay.setup();
   clockDisplay.SetBrightness(1000);
   
   // _lcd.begin(16, 2);
   
   lcd.setup();
 }
 
 void UpdateTimeDisplay() {
   clockDisplay.update();
 } 
 
 uint32_t b = 0;
 
 const int nColors = 7;
 const byte RR[] = {255, 175,   0,   0,   0, 150, 150};
 const byte GG[] = {  0, 255, 250, 250,   0,   0, 255};
 const byte BB[] = {  0,   0,   0, 250, 250, 250, 200};
 
 enum colors { RED=0, YELLOW=1, GREEN=2, TURQUOISE=3, BLUE=4, MAGENTA=5, WHITE=6 };
 
void SetLCDColor(int c) {
  lcd.SetColor(RR[c], GG[c], BB[c]);
}
const char* color_names[] = 
 {
   "Red", "Yellow", "Green", "Turquoise", "Blue", "Magenta", "White"
 };
   
enum LCD_DISPLAYING {LCD_BANNER, LCD_TIME, LCD_TEMP_C, LCD_TEMP_F, LCD_VOLTAGES, LCD_BMP};

int color = 0;
uint32_t t = 0;

void loop()
 {
   //DumpLCDChars();
   
   // all the comm work done in recieveEvent()
   //DebugLn(".");
   if(dataReady) {
     checkCommand();
     dataReady = false;
   }
   delay(10);
   /**/
   if(led_display_mode == TIME_MODE) {
     UpdateTimeDisplay();
   }
   /**/
   lcd.Update();
}
/*

void LCD_startVoltagesDisplay() {
  current_lcd_mode = LCD_VOLTAGES;
  SetLCDColor(RED);
  UpdateLCDVoltagesDisplay();
}
*/
/*
float voltage1 = 0.0;
float voltage2 = 0.0;
float voltage3 = 0.0;

void UpdateLCDVoltagesDisplay() {
  _lcd.clear();
  _lcd.home();
  
  _lcd.print("   VOLTAGES:    ");
  
  // 0123456789012345
  // x.xx  x.xx  x.xx
  _lcd.setCursor(0,1);
  _lcd.print(voltage1,2);
  _lcd.setCursor(6,1);
  _lcd.print(voltage2,2);
  _lcd.setCursor(12,1);
  _lcd.print(voltage3,2);
}
*/
/*
void UpdateLCD()
{
  switch(current_lcd_mode) {
    case LCD_BANNER:
       break;
     case LCD_TIME:
       UpdateLCDTimeDisplay();
       break;
     case LCD_TEMP_C:
       UpdateLCDTempCDisplay();
       break;
     case LCD_TEMP_F:
       UpdateLCDTempFDisplay();
       break;
     case LCD_BMP:
       break;
   }
 }
 */
 void checkCommand()
 {
   Debug("checkCommand: "); DebugLn(dataIn[0]);
   switch(dataIn[0]) {
     case SET_LED_MODE:
       SetLedMode();
       break;
     case SET_TIME:
       SetTime();
       break;
     case SET_TEMP:
       SetTemp();
       break;
     case SET_PRESSURE:
       SetPressure();
       break;
     case SET_BRIGHTNESS:
       SetBrightness();
       break;
     case SET_VOLTAGES:
       SetVoltages();
       break;
   }
 }
 
 void SetLedMode()
 {
 }
 void SerialOutDateTime()
 {
   if(clockDisplay.clockIsSet()) {
     DateTime dt(clockDisplay.now());
     uint8_t second = dt.second();
     uint8_t dayOfMonth = dt.day();
     uint8_t month = dt.month();
     uint8_t year = dt.year();
     uint8_t dayOfWeek = dt.dayOfWeek();
     
     Debug(dt.hour()); Debug(":"); Debug(dt.minute());
     Debug(":"); Debug(dt.second());
     Debug(" - ");
     Debug(SAC_ClockDisplay::Days[dt.dayOfWeek()]); Debug(", ");
     Debug(SAC_ClockDisplay::Months[dt.month()]); Debug(" ");
     Debug(dt.day()); Debug(", ");
     DebugLn(dt.year());
   }
  else {
   DebugLn("Clock is not set"); 
   }
 }

void SetVoltages()
{
  int i = 1;
  float voltage1 = 0.0, voltage2 = 0.0, voltage3 = 0.0;
  
  byte* bPtr = (byte*)&voltage1;
  int n = 0;
  while (n < sizeof(float)) {
    byte b = dataIn[i++];
    bPtr[n++] = b;
  }
  bPtr = (byte*)&voltage2;
  n = 0;
  while (n < sizeof(float)) {
    byte b = dataIn[i++];
    bPtr[n++] = b;
  }
  bPtr = (byte*)&voltage3;
  n = 0;
  while (n < sizeof(float)) {
    byte b = dataIn[i++];
    bPtr[n++] = b;
  }
}

 void SetTime()
 {
   int n = 0;
   uint32_t now = 0;
   byte* bPtr = (byte*)&now;
   //Debug("SetTime: recieving:");
   int i = 1;
   while (i <= 4) {
     byte b = dataIn[i++];
     bPtr[n++] = b;
     //Debug(" ");
     //Debug(b, HEX);
   }
   //Debug(", now = ");
   DebugLn(now, HEX);
   
   clockDisplay.SetTimeDate(now);
   SerialOutDateTime();
 }
  
 void SetTemp()
 {
   short temperature = -255;
   byte* bPtr = (byte*)&temperature;
   
   for(int i = 0; i < sizeof(short); i++) {
     bPtr[i] = dataIn[i+1];
   }
   lcd.SetTemperature(temperature);
   
   Debug("SET TEMP: ");
   DebugLn(temperature);
   //temperature_is_set = true;
}

long pressure = 0; 
void SetPressure()
{
   byte* bPtr = (byte*)&pressure;
   
   for(int i = 0; i < sizeof(long); i++) {
     bPtr[i] = dataIn[i+1];
   }
   
   Debug("SET PRESSURE: ");
   DebugLn(pressure);
}

int brightness_in;
void SetBrightness() {
  byte* bPtr = (byte*)&brightness_in;
  for(int i = 0; i < sizeof(int); i++) {
    bPtr[i] = dataIn[i+1];
  }
  Debug("SET_BRIGHTNESS: ");
  DebugLn(brightness_in);
  clockDisplay.SetBrightness(brightness_in);
}
 void handleRequest()
 {
   // master wants to know if we're here.
   byte status = clockDisplay.clockIsSet() ? TIMESET : STARTUP;
   Debug("Sending '"); Debug(status); DebugLn("'");
   Wire.write(status);
 }
 
 void handleReceive(int howMany)
 {
   //Debug("receiveEvent(");
   //Debug(howMany);
   //DebugLn(")");
   
   int i = 0;
   while(Wire.available() > 0) 
   {
     byte b = Wire.read(); 
     dataIn[i] = b;
     i++;
   }
   dataReady = true;
 }
 
 
