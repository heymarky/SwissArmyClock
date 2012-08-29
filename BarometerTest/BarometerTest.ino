#include <DS1307_RTC.h>

#include <SoftwareSerial.h>
const int rx1pin = 2; // unused
const int tx1pin = 3;
const int rx2pin = 4; // unused
const int tx2pin = 5;

SoftwareSerial myDisplay1(rx1pin, tx1pin);
SoftwareSerial myDisplay2(rx2pin, tx2pin);

DS1307_RTC rtc;

/* BMP085 Extended Example Code
  by: Jim Lindblom
  SparkFun Electronics
  date: 1/18/11
  license: CC BY-SA v3.0 - http://creativecommons.org/licenses/by-sa/3.0/
  
  Get pressure and temperature from the BMP085 and calculate altitude.
  Serial.print it out at 9600 baud to serial monitor.

  Update (7/19/11): I've heard folks may be encountering issues
  with this code, who're running an Arduino at 8MHz. If you're 
  using an Arduino Pro 3.3V/8MHz, or the like, you may need to 
  increase some of the delays in the bmp085ReadUP and 
  bmp085ReadUT functions.
*/

#include <Wire.h>

#define BMP085_ADDRESS 0x77  // I2C address of BMP085

const unsigned char OSS = 3;  // Oversampling Setting

// Calibration values
int ac1;
int ac2; 
int ac3; 
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int b1; 
int b2;
int mb;
int mc;
int md;

// b5 is calculated in bmp085GetTemperature(...), this variable is also used in bmp085GetPressure(...)
// so ...Temperature(...) must be called before ...Pressure(...).
long b5; 

short temperature;
long pressure;

/// Thermistor stuff
#define ThermistorPIN A0                 // Analog Pin 0

float vcc = 4.91;                       // only used for display purposes, if used
                                        // set to the measured Vcc.
float pad = 9770;                       // balance/pad resistor value, set this to
                                        // the measured resistance of your pad resistor
float thermr = 10000;                   // thermistor nominal resistance

float Thermistor(int RawADC) {
  long Resistance;  
  float Temp;  // Dual-Purpose variable to save space.

  Resistance=((1024.0 * pad / (float)RawADC) - pad); 
  float logTemp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
  Temp = 1 / (0.001129148 + (0.000234125 * logTemp) + (0.0000000876741 * logTemp * logTemp * logTemp));
  Temp = Temp - 273.15;  // Convert Kelvin to Celsius                      

  // BEGIN- Remove these lines for the function not to display anything
  //Serial.print("ADC: "); 
  //Serial.print(RawADC); 
  //Serial.print("/1024");                           // Print out RAW ADC Number
  //Serial.print(", vcc: ");
  //Serial.print(vcc,2);
  //Serial.print(", pad: ");
  //Serial.print(pad/1000,3);
  //Serial.print(" Kohms, Volts: "); 
  //Serial.print(((RawADC*vcc)/1024.0),3);   
  //Serial.print(", Resistance: "); 
  //Serial.print(Resistance);
  //Serial.print(" ohms, ");
  // END- Remove these lines for the function not to display anything

  // Uncomment this line for the function to return Fahrenheit instead.
  //temp = (Temp * 9.0)/ 5.0 + 32.0;                  // Convert to Fahrenheit
  return Temp;                                      // Return the Temperature
}

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  bmp085Calibration();
  myDisplay1.begin(9600);
  myDisplay2.begin(9600);
  
  myDisplay1.write(0x76);
  myDisplay2.write(0x76);

}
int H = 12;
int M = 15;

void PrintTime()
{
    unsigned long secs = millis() / 1000L + M * 60L + H * 3600L;
    unsigned long mins = secs / 60L;

    int hrs = 0;
    while(mins > 60) {
      mins -= 60;
      hrs++;
      if(hrs > 12) hrs = 1;
    }
    if(hrs < 10) Serial.print("0");
    Serial.print(hrs);
    Serial.print(":");
    if(mins < 10) Serial.print("0");
    Serial.print(mins);
    Serial.print(" - ");
}

unsigned long prnt_delay = 1000L;// * 60L * 5L;
unsigned long prnt_start = 0; // print a line right away
void loop()
{
  rtc.update();
  
  float thermistorTemp = Thermistor(analogRead(ThermistorPIN));
  
  temperature = bmp085GetTemperature(bmp085ReadUT());
  pressure = bmp085GetPressure(bmp085ReadUP());
  
  float C = (float)temperature * 0.1;
  float F = (C * 9.0)/ 5.0 + 32.0;
  float F2 = (thermistorTemp * 9.0) / 5.0 + 32.0;
  
  unsigned long prnt_elapsed = millis() - prnt_start;
  //Serial.print("Elapsed: "); Serial.print(prnt_elapsed);
  //Serial.print(", Delay: "); Serial.print(prnt_delay);
  //Serial.println("");
  if(prnt_start == 0 || prnt_elapsed >= prnt_delay) {
    //PrintTime();
    Serial.print(rtc.getHourMinuteString());
    Serial.print(" - Temperature: ");
    //Serial.print(temperature, DEC);
    //Serial.print(" *0.1 deg C, ");
    Serial.print(C, 2); Serial.print(" (");
    Serial.print(thermistorTemp); Serial.print(") C, ");
    Serial.print(F, 2); Serial.print(" (");
    Serial.print(F2); Serial.print(") F, ");
    Serial.print("Pressure: ");
    Serial.print(pressure, DEC);
    Serial.print(" Pa, ");
    float MM = (float)pressure * 0.0075;
    Serial.print(MM); Serial.print(" mm, ");
    float inHG = pressure/3386.389;
    Serial.print(inHG); Serial.print(" in");
    Serial.println();
    // 0.0075
    
    prnt_start = millis();
  }
  DisplayVal(myDisplay1, C, 'C');
  DisplayVal(myDisplay2, F, 'F');
  delay(1000);
}


char s[5];

void DisplayVal(SoftwareSerial& myDisplay, float val, char unit)
{
  int val_i = (int)(val * 10.0);
  
  myDisplay.write(0x77);
  myDisplay.write(0x02);
  
  //Serial.print("val_i: ");Serial.println(val_i);

  long  a,b,c,d;
  
  a = val_i/100;
  b = (val_i - (a * 100))/10;
  c = (val_i - (a * 100) - (b * 10));
  
  s[0] = a == 0 ? ' ' : (unsigned char)a + '0';
  s[1] = b == 0 && a == 0 ? ' ' : (unsigned char)b + '0';
  s[2] = c == 0 && b == 0 && a == 0 ? ' ' : (unsigned char)c + '0';
  s[3] = unit;
  s[4] = 0;
  //Serial.print("'");Serial.print(s);Serial.println("'");
  myDisplay.print(s);
}

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void bmp085Calibration()
{
  ac1 = bmp085ReadInt(0xAA);
  ac2 = bmp085ReadInt(0xAC);
  ac3 = bmp085ReadInt(0xAE);
  ac4 = bmp085ReadInt(0xB0);
  ac5 = bmp085ReadInt(0xB2);
  ac6 = bmp085ReadInt(0xB4);
  b1 = bmp085ReadInt(0xB6);
  b2 = bmp085ReadInt(0xB8);
  mb = bmp085ReadInt(0xBA);
  mc = bmp085ReadInt(0xBC);
  md = bmp085ReadInt(0xBE);
}

// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
short bmp085GetTemperature(unsigned int ut)
{
  long x1, x2;
  
  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  return ((b5 + 8)>>4);  
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long bmp085GetPressure(unsigned long up)
{
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;
  
  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;
  
  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;
  
  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;
    
  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;
  
  return p;
}

// Read 1 byte from the BMP085 at 'address'
char bmp085Read(unsigned char address)
{
  unsigned char data;

  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP085_ADDRESS, 1);
  while(!Wire.available())
    ;
    
  return Wire.read();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int bmp085ReadInt(unsigned char address)
{
  unsigned char msb, lsb;
  
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP085_ADDRESS, 2);
  while(Wire.available()<2)
    ;
  msb = Wire.read();
  lsb = Wire.read();
  
  return (int) msb<<8 | lsb;
}

// Read the uncompensated temperature value
unsigned int bmp085ReadUT()
{
  unsigned int ut;
  
  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();
  
  // Wait at least 4.5ms
  delay(50);
  
  // Read two bytes from registers 0xF6 and 0xF7
  ut = bmp085ReadInt(0xF6);
  return ut;
}

// Read the uncompensated pressure value
unsigned long bmp085ReadUP()
{
  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;
  
  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x34 + (OSS<<6));
  Wire.endTransmission();
  
  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));
  
  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF6);
  Wire.endTransmission();
  Wire.requestFrom(BMP085_ADDRESS, 3);
  
  // Wait for data to become available
  while(Wire.available() < 3)
    ;
  msb = Wire.read();
  lsb = Wire.read();
  xlsb = Wire.read();
  
  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);
  
  return up;
}

