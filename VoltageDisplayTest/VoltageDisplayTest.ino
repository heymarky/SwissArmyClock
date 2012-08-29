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

int brightness = 800;

void setup()
{
  /* Call Tlc.init() to setup the tlc.
     You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
  clock_display.setup();
  clock_display.SetBrightness(brightness);
  Serial.begin(57600);
}

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

// voltage monitoring stuff
const float referenceVolts = 5.0;
const int batteryPin = A5;

void loop()
{
  handleInput();
  int v = analogRead(batteryPin);
  float volts = ((float)v / 1023.0) * referenceVolts;
  clock_display.SetFloat(volts);
  Serial.print("brightness: "); Serial.print(brightness); Serial.print(", ");
  Serial.print(volts, 3);Serial.println(" volts");
  delay(1000);
}


