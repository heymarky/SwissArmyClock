#include <LiquidCrystal.h>

const int LCD_AltColorPin = 5;
#include <Wire.h>

#include <DS1307_RTC.h>
DS1307_RTC rtc;
// I2C hookup:
// A4 - SDA
// A5 - SCL
#include <Tlc5940.h>

#include <ClockDisplay.h>

/*
    Basic Pin setup:
    ------------                                  ---u----
    ARDUINO   13|-> SCLK (pin 25)           OUT1 |1     28| OUT channel 0
              12|                           OUT2 |2     27|-> GND (VPRG)
              11|-> SIN (pin 26)            OUT3 |3     26|-> SIN (pin 11)
              10|-> BLANK (pin 23)          OUT4 |4     25|-> SCLK (pin 13)
               9|-> XLAT (pin 24)             .  |5     24|-> XLAT (pin 9)
               8|                             .  |6     23|-> BLANK (pin 10)
               7|                             .  |7     22|-> GND
               6|                             .  |8     21|-> VCC (+5V)
               5|                             .  |9     20|-> 2K Resistor -> GND
               4|                             .  |10    19|-> +5V (DCPRG)
               3|-> GSCLK (pin 18)            .  |11    18|-> GSCLK (pin 3)
               2|                             .  |12    17|-> SOUT
               1|                             .  |13    16|-> XERR
               0|                           OUT14|14    15| OUT channel 15
    ------------                                  --------

    -  Put the longer leg (anode) of the LEDs in the +5V and the shorter leg
         (cathode) in OUT(0-15).
    -  +5V from Arduino -> TLC pin 21 and 19     (VCC and DCPRG)
    -  GND from Arduino -> TLC pin 22 and 27     (GND and VPRG)
    -  digital 3        -> TLC pin 18            (GSCLK)
    -  digital 9        -> TLC pin 24            (XLAT)
    -  digital 10       -> TLC pin 23            (BLANK)
    -  digital 11       -> TLC pin 26            (SIN)
    -  digital 13       -> TLC pin 25            (SCLK)
    -  The 2K resistor between TLC pin 20 and GND will let ~20mA through each
       LED.  To be precise, it's I = 39.06 / R (in ohms).  This doesn't depend
       on the LED driving voltage.
    - (Optional): put a pull-up resistor (~10k) between +5V and BLANK so that
                  all the LEDs will turn off when the Arduino is reset.

    If you are daisy-chaining more than one TLC, connect the SOUT of the first
    TLC to the SIN of the next.  All the other pins should just be connected
    together:
        BLANK on Arduino -> BLANK of TLC1 -> BLANK of TLC2 -> ...
        XLAT on Arduino  -> XLAT of TLC1  -> XLAT of TLC2  -> ...
    The one exception is that each TLC needs it's own resistor between pin 20
    and GND.

    This library uses the PWM output ability of digital pins 3, 9, 10, and 11.
    Do not use analogWrite(...) on these pins.

    This sketch does the Knight Rider strobe across a line of LEDs.

    Alex Leone <acleone ~AT~ gmail.com>, 2009-02-03 */

ClockDisplay clock_display;
// lcd(rd, enable, d0, d1, d2, d3)
LiquidCrystal lcd(2,8,A3,A2,6,7);

int brightness = 800;

void setup()
{
  Wire.begin();

  //pinMode(LCD_AltColorPin, OUTPUT);
  
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.print("Voltage Time");
  /* Call Tlc.init() to setup the tlc.
     You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
  clock_display.setup();
  clock_display.SetBrightness(brightness);
  Serial.begin(57600);
}

/* This loop will create a Knight Rider-like effect if you have LEDs plugged
   into all the TLC outputs.  NUM_TLCS is defined in "tlc_config.h" in the
   library folder.  After editing tlc_config.h for your setup, delete the
   Tlc5940.o file to save the changes. */

int hour = 0;
int minute = 0;
int second = 0;

int b_dir = -1;
void setBrightnessFromInput()
{
  int b = 0;
  while(Serial.available()){
    
    byte i = Serial.read();
    Serial.print("Read '"); Serial.print((char)i);Serial.print("', adding: ");
    if(i == '\n') break;
    i = i - '0';
    Serial.print(b);
    b = b*10 + i;
    Serial.print(", brightness = ");Serial.println(b);
  }
  brightness = b;
  if(brightness > 4095) brightness = 4095;
  else if(brightness < 0) brightness = 0;
  
  clock_display.SetBrightness(brightness);
  clock_display.update();
}

void handleInput()
{
  if(Serial.available()) {
    char cmd = Serial.read();
    switch(cmd)
    {
      case '+':
      brightness += 10;
      if(brightness > 4095) brightness = 4095;
      break;
      case '-':
      brightness -= 10;
      if(brightness < 0) brightness = 0;
      break;
      case 'b':
      case 'B':
      setBrightnessFromInput();
      break;
    }
  }
}
bool inError = false;
int errCount = 0;
uint32_t prev_update = 0;

const int batteryPin = A0;
void loop()
{
  int lcd_mode = (millis()/1000)%2;
  
  handleInput();
  uint32_t elapsed = millis() - prev_update;
  float voltage = getVoltage(batteryPin);
  analogWrite(LCD_AltColorPin, lcd_mode == 0 ? 0 : 255);
  lcd.setCursor(0,1);
  lcd.print(voltage);
  lcd.setCursor(8,1);
  //lcd.print(analogRead(batteryPin)); // TEMP
  if(rtc.update() || elapsed > 250)
  {
    prev_update = millis();
    lcd.print(errCount);
    if(inError) {
      inError = false;
      if(errCount > 0)
      {
    Serial.print(voltage, 2); Serial.print(" volts, ");
        Serial.print("Error (");
        Serial.print(errCount + 1);
        Serial.println(" retries)");
        errCount = 0;
      }
    }
    lcd.print(errCount);
    int h = hour;
    hour = rtc.getHour();
    while(hour > 12) hour -= 12;
    if(hour == 0) hour = 12;
    int m = minute;
    minute = rtc.getMinute();
    int s = second;
    second = rtc.getSecond();

    if(m != minute || h != hour || s != second) 
    {
  DisplayVoltage(batteryPin);
      Serial.print("B: ");
      Serial.print(brightness);
      Serial.print(", ");
      Serial.print(voltage, 2); Serial.print(" volts, ");
      Serial.println(rtc.getHourMinuteString());
      //lcd.print(rtc.getHourMinuteString());
      /*
      brightness += 10 * b_dir;
      if(brightness > 1000) {
        b_dir = -1;
        brightness += 20 * b_dir;
      }
      else if(brightness < 0)
      {
        b_dir = 1;
        brightness += 20 * b_dir;
      }
      */
      clock_display.SetTime(hour, minute);
      clock_display.update();
      delay(100);
    }
  }
  else {
    if(inError){errCount++;}
    inError = true;
    delay(50);
  }
}

// voltage monitoring stuff
const float referenceVolts = 4.47; // measured voltage running from USB
const float R1 = 1024.0;
const float R2 = 1024.0;
const float resistorFactor = 1023.0 * (R2/(R1 + R2));

float getVoltage(int batteryPin)
{
  float val = (float)analogRead(batteryPin);
  //float volts = (val/1023.0) * (2.0 * referenceVolts); //((float)val / resistorFactor) * referenceVolts;
  float volts = (val / 1023.0) * referenceVolts;
  return volts;
}
void DisplayVoltage(int batteryPin)
{
  float volts = getVoltage(batteryPin);
  //clock_display.SetFloat(volts);
  Serial.print("brightness: "); Serial.print(brightness); Serial.print(", ");
  Serial.print(" rF: "); Serial.print(resistorFactor); Serial.print(", ");
  Serial.print(" val: "); Serial.print(analogRead(batteryPin));Serial.print(", ");
  Serial.print(volts, 3);Serial.println(" volts");
}


