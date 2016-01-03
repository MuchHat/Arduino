/*************************************************** 
  This is a library for our I2C LED Backpacks

  Designed specifically to work with the Adafruit LED Matrix backpacks 
  ----> http://www.adafruit.com/products/872
  ----> http://www.adafruit.com/products/871
  ----> http://www.adafruit.com/products/870

  These displays use I2C to communicate, 2 pins are required to 
  interface. There are multiple selectable I2C addresses. For backpacks
  with 2 Address Select pins: 0x70, 0x71, 0x72 or 0x73. For backpacks
  with 3 Address Select pins: 0x70 thru 0x77

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include <Adafruit_GPS.h>

Adafruit_BicolorMatrix matrix1 = Adafruit_BicolorMatrix();
Adafruit_BicolorMatrix matrix2 = Adafruit_BicolorMatrix();
Adafruit_BicolorMatrix matrix3 = Adafruit_BicolorMatrix();
Adafruit_BicolorMatrix matrix4 = Adafruit_BicolorMatrix();


Adafruit_GPS GPS( &Serial3 );


int gpsTimeHour = -1;
int gpsTimeMin = -1;
int gpsTimeSec = -1;
////////////////////////////////////////////
// 
//
boolean getGPSTime()
{
      int i;
      char c;
      int a;

       for ( a = 0; a < 11; a++)
       {
           for( i = 0; i < 999; i++ )
           {
                while( Serial3.available() > 0 )
                { 
                    c = GPS.read(); 
                };
                delay( 1 );
           } 
           if( GPS.newNMEAreceived() )
           { 
                GPS.parse( GPS.lastNMEA() );
                break;
           }
       }      
       gpsTimeHour = -1;
       gpsTimeMin = -1;
       gpsTimeSec = -1;

       if( GPS.year > 24  ) return false;
       if( GPS.year < 14  ) return false;
       if( GPS.hour > 23  ) return false;
       if( GPS.hour < 0  ) return false;
       if( GPS.minute > 59  ) return false;
       if( GPS.minute < 0  ) return false;
        
      int previousMonthLastDay[ 13 ]={31,31,28,31,30,31,30,31,31,30,31,30,31};
      int daysInMonth[ 12 ]={31,28,31,30,31,30,31,31,30,31,30,31};
      
      int firstMonthDST2014[ 11 ]={3, 3, 3, 3, 3, 3, 3, 3 };
      int firstDayDST014[ 11 ]=   {9, 8, 13, 12, 11, 10, 8, 14, 13, 12, 10 };
  
      int lastMonthDST2014[ 11 ]= {11, 11, 11, 11, 11, 11, 11, 11 };
      int lastDayDST2014[ 11 ]=   {2, 1, 6, 5, 4, 3, 1, 7, 6, 5, 3 };
 
     if( ( GPS.year % 4 ) == 0 )
     {
        previousMonthLastDay[2]=29;
        daysInMonth[1]=29;
     }
    
     int hour = GPS.hour - 8;
     int day = GPS.day;
     int month = GPS.month;
     int year = GPS.year;
    
      if(  (month == firstMonthDST2014[year-14]&& day >=firstDayDST014[year-14])  ||
           (month >firstMonthDST2014[year-14] && month <lastMonthDST2014[year-14] )||
           (month==lastMonthDST2014[year-14] && day<lastDayDST2014[year-14] ) )
      {
        hour++;
      }

      if( hour < 0 )
      {
          day--;
          if( day < 1 ){  month--; day = previousMonthLastDay[month]; }
          if( month < 1 ){ year--; month = 12; }  
          hour += 24;
      }

       gpsTimeHour = hour;
       gpsTimeMin = GPS.minute;
       gpsTimeSec = GPS.seconds;

         Serial.print( "(g" );
         Serial.print( GPS.fix );
        Serial.print( " " );
        Serial.print( gpsTimeHour );
        Serial.print( "h" );
        Serial.print( gpsTimeMin );
        Serial.print( "m" );
        Serial.print( gpsTimeSec );
        Serial.print( "s)" );
       
       return ( GPS.fix > 0 );
}

void setup() {
  Serial.begin(9600);
  Serial.println("8x8 LED Matrix Test");
  
  matrix1.begin(0x70);  // pass in the address 70, 74, 72,76
  matrix2.begin(0x74);  // pass in the address 70, 74, 72,76
  matrix3.begin(0x72);  // pass in the address 70, 74, 72,76
  matrix4.begin(0x76);  // pass in the address 70, 74, 72,76

  matrix1.setBrightness( 2 ); //0-16
    matrix2.setBrightness( 2 );
      matrix3.setBrightness( 2 );
        matrix4.setBrightness( 2 );

             GPS.begin( 9600 );
     
     GPS.sendCommand( PMTK_SET_NMEA_OUTPUT_RMCONLY );
     GPS.sendCommand( PMTK_SET_NMEA_UPDATE_10HZ );   // 1 Hz update rate
     GPS.sendCommand( PGCMD_NOANTENNA );

 
 }

static const uint8_t PROGMEM
  smile_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10100101,
    B10011001,
    B01000010,
    B00111100 },
  neutral_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10111101,
    B10000001,
    B01000010,
    B00111100 },
  frown_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10011001,
    B10100101,
    B01000010,
    B00111100 };

void loop() {

  matrix1.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix1.setTextSize(1);
  matrix1.setTextColor(LED_GREEN);

  matrix2.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix2.setTextSize(1);
  matrix2.setTextColor(LED_GREEN);

  matrix3.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix3.setTextSize(1);
  matrix3.setTextColor(LED_GREEN);

  matrix4.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix4.setTextSize(1);
  matrix4.setTextColor(LED_GREEN);

  matrix1.setRotation(3);
  matrix2.setRotation(3);
  matrix3.setRotation(3);
  matrix4.setRotation(3);

  getGPSTime();
  
  if( gpsTimeHour > 0 && gpsTimeMin > 0 )
  {

  matrix1.setBrightness( 2 ); //0-16
    matrix2.setBrightness( 2 );
      matrix3.setBrightness( 2 );
        matrix4.setBrightness( 2 );
        
  matrix1.clear();
  matrix1.setCursor(2,1);
  matrix1.print(gpsTimeHour/10,DEC);
  matrix1.writeDisplay();
  
  matrix2.clear();
  matrix2.setCursor(1,1);
   matrix2.print(gpsTimeHour % 10,DEC);
   

  matrix2.writeDisplay();
  
  matrix3.clear();
  matrix3.setCursor(3,1);
 matrix3.print(gpsTimeMin/10,DEC);
      matrix3.setCursor(-2,1);
   matrix3.print(":");
   matrix3.writeDisplay();
  
  matrix4.clear();
  matrix4.setCursor(2,1);
  matrix4.print(gpsTimeMin%10,DEC);
  matrix4.writeDisplay();

  delay(1000);

  }
  else
  {

  matrix1.clear();
  matrix1.drawBitmap(0, 0, smile_bmp, 8, 8, LED_GREEN);
  matrix1.writeDisplay();
  delay(500);

  matrix2.clear();
  matrix2.drawBitmap(0, 0, neutral_bmp, 8, 8, LED_YELLOW);
  matrix2.writeDisplay();
  delay(500);

  matrix3.clear();
  matrix3.drawBitmap(0, 0, frown_bmp, 8, 8, LED_RED);
  matrix3.writeDisplay();
  delay(500);

  matrix4.clear();
  matrix4.drawBitmap(0, 0, smile_bmp, 8, 8, LED_GREEN);
  matrix4.writeDisplay();
  delay(500);

  delay(2000);
  }
}

/*
  matrix4.clear();      // clear display
  matrix4.drawPixel(0, 0, LED_GREEN);  
  matrix4.writeDisplay();  // write the changes we just made to the display
  delay(500);

  matrix1.clear();
  matrix1.drawLine(0,0, 7,7, LED_YELLOW);
  matrix1.writeDisplay();  // write the changes we just made to the display
  delay(500);

  matrix2.clear();
  matrix2.drawRect(0,0, 8,8, LED_RED);
  matrix2.fillRect(2,2, 4,4, LED_GREEN);
  matrix2.writeDisplay();  // write the changes we just made to the display
  delay(500);

  matrix3.clear();
  matrix3.drawCircle(3,3, 3, LED_YELLOW);
  matrix3.writeDisplay();  // write the changes we just made to the display
  delay(500);

  matrix4.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix4.setTextSize(1);
  matrix4.setTextColor(LED_GREEN);
  for (int8_t x=7; x>=-36; x--) {
    matrix4.clear();
    matrix4.setCursor(x,0);
    matrix4.print("Hello");
    matrix4.writeDisplay();
    delay(100);
  }
  matrix1.setRotation(3);
  matrix1.setTextColor(LED_RED);
  for (int8_t x=7; x>=-36; x--) {
    matrix1.clear();
    matrix1.setCursor(x,0);
    matrix1.print("World");
    matrix1.writeDisplay();
    delay(100);
  }
  matrix1.setRotation(0);
  
} */
