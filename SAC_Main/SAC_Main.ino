#include <SAC_Joystick.h>

#include <DisplayModule.h>
#include <SAC_Main.h>

#include <BMP085.h>

#define SAC_MASTER 1
#undef SAC_DISPLAY

#include <ClockDisplay.h>
#include <SwissArmyClock.h>

#include <Wire.h>
#include <RTClib.h>

DisplayModule display;

const int LED_PIN = 13;

/// MUX CHANNEL ASSIGNMENTS
const int VOLTAGE_MAIN = 0;
const int VOLTAGE_DISP = 1;
const int VOLTAGE_LED  = 2;

const int JOY_SEL = 12;
const int JOY_X = 3;
const int JOY_Y = 4;

/**
 * Swiss Army Clock Master Processor
 */
 SAC_Joystick joystick(&readMux, JOY_X, JOY_Y, JOY_SEL);
 
 RTC_DS1307 RTC;
 bmp085 bmp;     // Barometric Pressure Sensor
 
 void setup()
 {
   DebugLn("SETUP");

   Wire.begin();
#if defined(DEBUG)
   Serial.begin(9600);
#endif // DEBUG

   RTC.begin();
   bmp.begin();
   
   pinMode(LED_PIN, OUTPUT);
   
   mux_setup();
   
   joystick.setup();
   joystick.RegisterOnSelect(&OnJoySelect);
   joystick.RegisterOnLeft(&OnJoyLeft);
   joystick.RegisterOnRight(&OnJoyRight);
   joystick.RegisterOnUp(&OnJoyUp);
   joystick.RegisterOnDown(&OnJoyDown);
 }
 enum State {ST_WAIT_DISP, ST_SET_TIME, ST_NORMAL};
 
 State state = ST_WAIT_DISP;

short temperature = 0;
long pressure = 0;
int voltage_main = 0;

void loop()
{
  CheckTimeSet();

  joystick.update();
  
  switch(state) {
    case ST_WAIT_DISP: // Waiting for display 
      WaitForDisplay();
      break;
    case ST_SET_TIME:
      SetTime();
      break;
    case ST_NORMAL:
      CheckStuff();
      break;
    default:
      break;
  }
}

uint32_t timeset_t = 0;
void CheckTimeSet() {
  if(timeset_t != millis()/1000) {
    timeset_t = millis()/1000;
    
    byte status = display.GetStatus();
    if(status == DisplayModule::STARTUP) {
      state = ST_SET_TIME;
      temperature = -255;
      pressure = -255;
      voltage_main = -255;
    }
  }
}

uint32_t bright_t = 0;
int bright = 2000;
int bdir = 100;
const int bmin = 100;
const int bmax = 4000;

void CheckBrightness() {
  // once every second, see if brightness has changed
  if(bright_t != millis()/1000) {
    bright_t = millis()/1000;
    
    //display.SetBrightness(bright);
    //if(bright + bdir > bmax || bright + bdir < bmin) bdir *= -1;
    //bright += bdir;
  }
}

int volt_t = 0;

void CheckVoltage() {
  if(volt_t != millis()/10000) {
    volt_t = millis()/10000;
    
    int v = readMux(VOLTAGE_MAIN);
    if(v != voltage_main) {
      voltage_main = v;
      float f1 = ((float)v / 1023.0) * 10.0;
      Debug(" *** VOLTAGE(MAIN) = ");
      Debug(f1, 2);
      DebugLn(" ***");
      float f2 = 0.0;
      float f3 = 0.0;
      
      display.SetVoltages(f1, f2, f3);
    }    
  }
}

// ST_NORMAL state handler
uint32_t t = 0;
void CheckStuff() {
  CheckBrightness();
  CheckVoltage();
  // once every five seconds, see if temperature or pressure have changed
  if(t != millis()/5000) {
    t = millis()/5000;
    short temp;
    long press;
    bmp.GetTemperatureAndPressure(temp, press);
    if(temp != temperature) {
      temperature = temp;
      display.SetTemperature(temperature);
    }
    if(press != pressure) {
      pressure = press;
      display.SetPressure(pressure);
    }
  }
}

// Read the current time/date from RTC and send it to
// the SAC_Display device.
void SetTime()
{
  uint32_t now = 0;
  if(RTC.isrunning()) {
    DateTime dt = RTC.now();
    now = dt.unixtime();
  }
  display.SetTime(now);
  state = ST_NORMAL;
}

void WaitForDisplay()
{  
  /*
  byte buffer[8];
  Debug("WAITING FOR DISPLAY... r = ");
  
  int r = Wire.requestFrom(display_address, 5);
  delay(10);
  Debug(r);
  Debug(", available: ");
  DebugLn(Wire.available());

  int i = 0;
  while(Wire.available() > 0 && i < 5) {
    buffer[i++] = Wire.read();
    delay(1);
    Debug("'");
    Debug((char)buffer[i-1]);
    DebugLn("'");
  }
  buffer[i] = 0;
  String s = (const char*)buffer;
  if(s.equals("ready")) {
    state = ST_SET_TIME;
    DebugLn(" - DISPLAY IS READY");
    digitalWrite(LED_PIN, HIGH);
    temperature = -255;
    pressure = -255;
    delay(5);
  }
  else if(s.equals("going")) {
    delay(1000);
  }
  else {
    delay(1000);
  }
  */
}
///////////// Joystick callbacks ////////
void OnJoySelect() {
  DebugLn("*** ON SELECT");
}
void OnJoyLeft() {
  DebugLn("*** ON LEFT");
}
void OnJoyRight() {
  DebugLn("*** ON RIGHT");
}
void OnJoyUp() {
  DebugLn("*** ON UP");
}
void OnJoyDown() {
  DebugLn("*** ON DOWN");
}
///////////// Mux Control ///////////////
const int mux_s[] = {2, 4, 7, 8};

int mux_sig_pin = A3;

void mux_setup() {
  int i = 0;
  for(i = 0; i < 4; i++) {
    pinMode(mux_s[i], OUTPUT);
  }
  for(i = 0; i < 4; i++) {
    digitalWrite(mux_s[i], LOW);
  }
}

// returns 0 - 1023
int readMux(int channel){
  const int muxChannel[16][4]={
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1}, //channel 14
    {1,1,1,1}  //channel 15
  };

  //loop through the 4 sig
  for(int i = 0; i < 4; i ++){
    digitalWrite(mux_s[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  int val = analogRead(mux_sig_pin);

  //return the value
  return val;
}
