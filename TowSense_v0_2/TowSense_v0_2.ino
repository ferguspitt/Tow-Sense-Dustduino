

#include <SD.h>
#include <Wire.h>
//#include <avr/wdt.h>
#include "RTClib.h"
#include <Time.h> 
RTC_DS1307 rtc;

// Chip Select on DustDuino for Tow is Digital 10
const int chipSelect = 10;
unsigned long starttime;
unsigned int sample;
      
      // Setting average, max, running total, and counters
      long AvgSound = 0;
//     int MaxSound = 0;
      long SoundTotal = 0;
      long i = 0;
      
     // double AvgDust = 0;
      double dustVal = 0;
      double MaxDust = 0;
      double DustTotal = 0;
      long k = 0;
      
      int TheSeconds = 0;
      

void setup() {
  // setting WTDT for 8 seconds
  //wdt_enable(WDTO_8S);
  
  Serial.begin(9600);
  pinMode(5,OUTPUT);
  
  // Dust reading notification LED on digital pin 2
 pinMode (2, OUTPUT);
  
  #ifdef AVR
  Wire.begin();
  #else
  Wire1.begin();
  #endif
  rtc.begin();
  
  Serial.print("Initializing SD card...");
  // CS Pin must be set to output
  pinMode(10, OUTPUT);
  
  // SD card check
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("card initialized.");
  
//  String startString = "Month,Day,Hour,Min,Sec,AvgDust,MaxDust,AvgSound,MaxSound";
  String startString = "DateAndTime,dustVal,MaxDust,AvgSound";
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
    if (dataFile) {
    // Print first row with column headings in comma separated value format.  
    dataFile.println(startString);
    dataFile.close();
    Serial.println("Column headings printed...");
    Serial.println(startString);
    }
    
  starttime = millis();
  //
  //wdt_reset();
}

void loop() {
    //Serial.println("start of loop");
    GetSound();
    //wdt_reset();
    
    // every 1 seconds, sample dust and increase counter;
    if ((millis() - starttime) > 1000) {
      GetDust();
      digitalWrite(2, HIGH);
      delay(100);
      digitalWrite(2, LOW);      
      TheSeconds++;
      starttime = millis();
    }
    
    // when counter hits 1 second, find time and log data
    if (TheSeconds > .9) {
      LogData();
      TheSeconds = 0;
      starttime = millis();
    }
    
}

void GetSound() {
  
  // Finds peak-to-peak amplitude of audio signal.
  // Samples audio at 20Hz, the lower limit of human hearing.
  // Upper limit of the microphone is 20kHz.
  // Posted on Adafrut: https://learn.adafruit.com/adafruit-microphone-amplifier-breakout?view=all
  
  unsigned long startMillis= millis(); // Start of sample window
  unsigned int peakToPeak = 0; // peak-to-peak level
 
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;
 
  // collect audio for 50 mS
  while (millis() - startMillis < 50)
  {
    sample = analogRead(0);
    if (sample < 1024) // toss out spurious readings
    {
    if (sample > signalMax)
    {
    signalMax = sample; // save just the max levels
    }
    else if (sample < signalMin)
    {
    signalMin = sample; // save just the min levels
    }
    }
  }
  peakToPeak = signalMax - signalMin; // max - min = peak-peak amplitude
  
/*  if (peakToPeak > MaxSound)
    {
      MaxSound = peakToPeak;
    }
*/
    SoundTotal = SoundTotal+ peakToPeak;
    i++;
}

void GetDust() {
    digitalWrite(5,LOW); // power on dust sensor LED
    delayMicroseconds(280);
    int dustSen = analogRead(A1); // read voltage off sensor pin5
    delayMicroseconds(40);
    digitalWrite(5,HIGH); // turn the LED off
    
    // Algorithm to find dust concentration from voltage, based on spec sheet
    double dustVal = ((((dustSen * 0.0049) * (0.172)) - 0.0999) * 100);
   if (dustVal < 0)
      {
        dustVal = 0;
      }

    if (dustVal > MaxDust) {
      MaxDust = dustVal;
    }
    
    DustTotal = dustVal + DustTotal;

    k++;
}

// Captures time and logs the data to SD
void LogData()
{
      DateTime now = rtc.now();
      //Serial.println("We're in the log data loop");
    //  AvgDust = DustTotal / k;
      AvgSound = SoundTotal / i;
      
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
      if (dataFile) {
      /* Print first row with column headings in comma separated value format.  
      dataFile.print(now.month(), DEC);
      dataFile.print("-");
      dataFile.print(now.day(), DEC);
      dataFile.print("-");
      dataFile.print(now.hour(), DEC);
      dataFile.print("-");
      dataFile.print(now.minute(), DEC);
      dataFile.print("-");
      dataFile.print(now.second(), DEC);
      dataFile.print("-");
      dataFile.print(AvgDust);
      dataFile.print(",");
      dataFile.print(MaxDust);
      dataFile.print(",");
      dataFile.print(AvgSound);
      dataFile.print(",");
      dataFile.println(MaxSound);
      dataFile.close();
      */

      dataFile.print(now.year());
      dataFile.print("-");
      if (now.month()<10)
       dataFile.print('0');
      dataFile.print(now.month());
      dataFile.print("-");
      if (now.day()<10)
       dataFile.print('0');
      dataFile.print(now.day());
      dataFile.print(" ");
      if (now.hour()<10)
       dataFile.print('0');
      dataFile.print(now.hour());
      dataFile.print(":");
      if (now.minute()<10)
       dataFile.print('0');
      dataFile.print(now.minute());
      dataFile.print(":");
      if (now.second()<10)
       dataFile.print('0');
      dataFile.print(now.second());
      dataFile.print(",");
      //dataFile.print(AvgDust);
      //dataFile.print(dustVal);
      //dataFile.print(",");
      dataFile.print(MaxDust);
      dataFile.print(",");
      dataFile.println(AvgSound);
//      dataFile.print(",");
//      dataFile.println(MaxSound);
      dataFile.close();
      
      
      
      // Also print to serial, if serial is connected
      Serial.print(now.year());
      dataFile.print(now.year());
      Serial.print("-");
      if (now.month()<10)
       Serial.print('0');
      Serial.print(now.month());
      Serial.print("-");
      if (now.day()<10)
       Serial.print('0');
      Serial.print(now.day());
      Serial.print(" ");
      if (now.hour()<10)
       Serial.print('0');
      Serial.print(now.hour());
      Serial.print(":");
      if (now.minute()<10)
       Serial.print('0');
      Serial.print(now.minute());
      Serial.print(":");
      if (now.second()<10)
       Serial.print('0');
      Serial.print(now.second());
      //Serial.print(",");
     // Serial.print(AvgDust);
     // Serial.print(dustVal);
      Serial.print(",");
      Serial.print(MaxDust);
      Serial.print(",");
      Serial.println(AvgSound);
      Serial.println(i);
//      Serial.print(",");
//      Serial.println(MaxSound);
      
      // Reset mean, max, running totals, and counters
      AvgSound = 0;
//      MaxSound = 0;
      SoundTotal = 0;
      i = 0;
      
     // dustVal = 0;
      MaxDust = 0;
    //  DustTotal = 0;
      k = 0;
      }


}


