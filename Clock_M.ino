////////////////////////////////////////////
//  GPS Clock using AdaFruit 8x8 Bicolor Display
//  Jan 2016
//

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include <Adafruit_GPS.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

////////////////////////////////////////////
//  Global Variables
//
Adafruit_BicolorMatrix matrix1 = Adafruit_BicolorMatrix();
Adafruit_BicolorMatrix matrix2 = Adafruit_BicolorMatrix();
Adafruit_BicolorMatrix matrix3 = Adafruit_BicolorMatrix();
Adafruit_BicolorMatrix matrix4 = Adafruit_BicolorMatrix();
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

Adafruit_GPS GPS( &Serial3 );

int gpsTimeHour = -1;
int gpsTimeMin = -1;
int gpsTimeSec = -1;

int lastGpsTimeHour = -1;
int lastGpsTimeMin = -1;
int lastGpsTimeSec = -1;

int nAlarmON = 0;

long lastGpsMillis = -1;
int minsWithoutGPS = 0;

/////////////////////////////////////////////////////////////////////////
// Reads the serial from the GPS and stores the time in global variables
//
boolean getGPSTime()
{
  int i;
  char c;
  int a;

  // will try to read more than once since
  // a full sentence is needed
  //
  for ( a = 0; a < 11; a++)
  {
    for ( i = 0; i < 666; i++ )
    {
      while ( Serial3.available() > 0 )
      {
        c = GPS.read();
      };
      delay( 1 );
    }
    if ( GPS.newNMEAreceived() )
    {
      GPS.parse( GPS.lastNMEA() );
      break;
    }
    delay( 10 );
  }

  // we should have a full sentence here
  // if incorrect sentence or corrupt the global vars will contain -1
  //
  gpsTimeHour = -1;
  gpsTimeMin = -1;
  gpsTimeSec = -1;

  if ( GPS.year > 24  ) return false;
  if ( GPS.year < 14  ) return false;
  if ( GPS.hour > 23  ) return false;
  if ( GPS.hour < 0  ) return false;
  if ( GPS.minute > 59  ) return false;
  if ( GPS.minute < 0  ) return false;

  // account for the daylight savings
  // and adjust for Pacific Time
  //
  int previousMonthLastDay[ 13 ] = {31, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int daysInMonth[ 12 ] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  int firstMonthDST2014[ 11 ] = {3, 3, 3, 3, 3, 3, 3, 3 };
  int firstDayDST014[ 11 ] =   {9, 8, 13, 12, 11, 10, 8, 14, 13, 12, 10 };

  int lastMonthDST2014[ 11 ] = {11, 11, 11, 11, 11, 11, 11, 11 };
  int lastDayDST2014[ 11 ] =   {2, 1, 6, 5, 4, 3, 1, 7, 6, 5, 3 };

  if ( ( GPS.year % 4 ) == 0 )
  {
    previousMonthLastDay[2] = 29;
    daysInMonth[1] = 29;
  }

  int hour = GPS.hour - 8;
  int day = GPS.day;
  int month = GPS.month;
  int year = GPS.year;

  if (  (month == firstMonthDST2014[year - 14] && day >= firstDayDST014[year - 14])  ||
        (month > firstMonthDST2014[year - 14] && month < lastMonthDST2014[year - 14] ) ||
        (month == lastMonthDST2014[year - 14] && day < lastDayDST2014[year - 14] ) )
  {
    hour++;
  }

  if ( hour < 0 )
  {
    day--;
    if ( day < 1 )
    {
      month--;
      day = previousMonthLastDay[month];
    }
    if ( month < 1 )
    {
      year--;
      month = 12;
    }
    hour += 24;
  }

  // now we have the time accounting for dailight savings
  //
  gpsTimeHour = hour;
  gpsTimeMin = GPS.minute;
  gpsTimeSec = GPS.seconds;

  lastGpsMillis = millis();
  lastGpsTimeHour = hour;
  lastGpsTimeMin = GPS.minute;
  lastGpsTimeSec = GPS.seconds;
  
  minsWithoutGPS = 0;

  // log for debugging purposes
  //
  Serial.print( "(g" );
  Serial.print( GPS.fix );
  Serial.print( " " );
  Serial.print( gpsTimeHour );
  Serial.print( "h" );
  Serial.print( gpsTimeMin );
  Serial.print( "m" );
  Serial.print( gpsTimeSec );
  Serial.print( "s)" );
  Serial.println( "" );

  // the return indicates if the GPS locked on the signal
  // if a sentence was received a time will be provided from GPS or RTC
  //
  return ( GPS.fix > 0 );
}

/////////////////////////////////////////////////////////////////////////
// Compute time based on millis() if GPS not available
//
void getClockTime()
{
  if ( lastGpsMillis < 0 || lastGpsTimeHour < 0 || lastGpsTimeMin < 0 || lastGpsTimeSec < 0  ) return;
  if ( gpsTimeHour > 0 && gpsTimeMin > 0 && gpsTimeSec > 0 ) return;

  long secsSinceLastGpsTime;

  secsSinceLastGpsTime = ( millis() - lastGpsMillis ) / 1000;
  minsWithoutGPS = 999;

  if( millis() < lastGpsMillis ) return;
  if ( secsSinceLastGpsTime < 0 ) return;

  int crrGpsHour = lastGpsTimeHour;
  int crrGpsMin = lastGpsTimeMin;  
  int crrGpsSec = lastGpsTimeSec;

  crrGpsSec += secsSinceLastGpsTime;
  if ( crrGpsSec > 59 )
  {
    crrGpsMin += ( crrGpsSec - crrGpsSec % 60 ) / 60;
    crrGpsSec = crrGpsSec % 60;
  }
  
  if ( crrGpsMin > 59 )
  {
    crrGpsHour += ( crrGpsMin - crrGpsMin % 60 ) / 60;
    crrGpsMin = crrGpsMin % 60;
  }
  
  if ( crrGpsHour > 23 )
  {
    crrGpsHour =  crrGpsHour % 24;
  }

  gpsTimeHour = crrGpsHour;
  gpsTimeMin = crrGpsMin;
  gpsTimeSec = crrGpsSec;

  if ( gpsTimeHour > 23 ) 
  {
    gpsTimeHour = -1;
    gpsTimeMin = -1;
    gpsTimeSec = -1;
  }
  if ( gpsTimeMin > 59 ) 
  {
    gpsTimeHour = -1;
    gpsTimeMin = -1;
    gpsTimeSec = -1;
  }
  if ( gpsTimeSec > 59 ) 
  {
    gpsTimeHour = -1;
    gpsTimeMin = -1;
    gpsTimeSec = -1;
  }
  minsWithoutGPS = secsSinceLastGpsTime / 60;
}

/////////////////////////////////////////////////////////////////////////
// setupLightSensor
//
void configureSensor(void)
{
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);          /* Auto-gain ... switches automatically between 1x and 16x */

  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  /* Update these values depending on what you've set above! */
  Serial.println("------------------------------------");
  Serial.print  ("Gain:         "); Serial.println("Auto");
  Serial.print  ("Timing:       "); Serial.println("13 ms");
  Serial.println("------------------------------------");
}

int prevLux = 100;
int prevBrightness = 0;
int mustRefresh = 1;

/////////////////////////////////////////////////////////////////////////
// Set brightness depending on ambient light
//
void setMatrixBrightness()
{
  int nClockBrightness = 0;

  // Get a new sensor event
  sensors_event_t event;
  tsl.getEvent( &event );

  // Display the results (light is measured in lux)
  if ( event.light >= 0 )
  {
    Serial.print(event.light); Serial.println(" lux");
    prevLux = prevLux + ( ( event.light - prevLux) / 2 );
  }
  else
  {
    // If event.light = 0 lux the sensor is probably saturated
    Serial.println("Sensor overload");
  }
  Serial.print(prevLux); Serial.println(" prev lux");

  // convert the 0-600 range into 0-15
  nClockBrightness = ( (prevLux - 30) * 15) / 600;
  if (nClockBrightness > 15)
  {
    nClockBrightness = 15;
  }
  else if (nClockBrightness < 0)
  {
    nClockBrightness = 0;
  }
  if ( nAlarmON == 1 )
  {
    nClockBrightness = 15;
  }

  Serial.print(nClockBrightness); Serial.println(" brightness");

  // in complete darkness turn the clock on green
  if ( nAlarmON == 1 )
  {
    matrix1.setTextColor(LED_YELLOW);
    matrix2.setTextColor(LED_YELLOW);
    matrix3.setTextColor(LED_YELLOW);
    matrix4.setTextColor(LED_YELLOW);
  }
  else if ( prevLux < 5 )
  {
    matrix1.setTextColor(LED_RED);
    matrix2.setTextColor(LED_RED);
    matrix3.setTextColor(LED_RED);
    matrix4.setTextColor(LED_RED);
    nClockBrightness = 0;
  }
  else if ( minsWithoutGPS > 1 )
  {
    matrix1.setTextColor(LED_GREEN);
    matrix2.setTextColor(LED_GREEN);
    matrix3.setTextColor(LED_GREEN);
    matrix4.setTextColor(LED_GREEN);
  }
  else
  {
    matrix1.setTextColor(LED_YELLOW);
    matrix2.setTextColor(LED_YELLOW);
    matrix3.setTextColor(LED_YELLOW);
    matrix4.setTextColor(LED_YELLOW);
  }

  matrix1.setBrightness( nClockBrightness ); //0-16
  matrix2.setBrightness( nClockBrightness );
  matrix3.setBrightness( nClockBrightness );
  matrix4.setBrightness( nClockBrightness );

  if ( prevBrightness != nClockBrightness )
  {
    mustRefresh = 1;
  }
  prevBrightness = nClockBrightness;

  Serial.print(prevBrightness); Serial.println(" brightness");
}

int prevMinDisplayed = -1;
int prevHourDisplayed = -1;

/////////////////////////////////////////////////////////////////////////
// Display GPS time based on the global variables
//
void displayGPSTime()
{
  if ( gpsTimeHour > 0 && gpsTimeMin > 0 )
  {
    if ( ( gpsTimeHour != prevHourDisplayed ) || ( mustRefresh == 1 ) )
    {
      matrix1.clear();
      matrix1.setCursor(2, 1);
      matrix1.print(gpsTimeHour / 10, DEC);
      matrix1.writeDisplay();

      matrix2.clear();
      matrix2.setCursor(1, 1);
      matrix2.print(gpsTimeHour % 10, DEC);
      matrix2.writeDisplay();

      prevHourDisplayed = gpsTimeHour;
    }

    if ( ( gpsTimeMin != prevMinDisplayed ) || ( mustRefresh == 1 ) )
    {
      matrix3.clear();
      matrix3.setCursor(3, 1);
      matrix3.print(gpsTimeMin / 10, DEC);
      matrix3.setCursor(-2, 1);
      matrix3.print(":");
      matrix3.writeDisplay();

      matrix4.clear();
      matrix4.setCursor(2, 1);
      matrix4.print(gpsTimeMin % 10, DEC);
      matrix4.writeDisplay();

      prevMinDisplayed = gpsTimeMin;
    }
  }
  else
  {
    matrix1.clear();
    matrix1.setCursor(2, 1);
    matrix1.print(".");
    matrix1.writeDisplay();

    matrix2.clear();
    matrix2.setCursor(1, 1);
    matrix2.print(".");
    matrix2.writeDisplay();

    matrix3.clear();
    matrix3.setCursor(3, 1);
    matrix3.print(".");
    matrix3.writeDisplay();

    matrix4.clear();
    matrix4.setCursor(2, 1);
    matrix4.print(".");
    matrix4.writeDisplay();

    prevHourDisplayed = -1;
    prevMinDisplayed = -1;
  }
}

/////////////////////////////////////////////////////////////////////////
// ARDUINO setup routine
//
void setDefaultMatrix()
{

  matrix1.setTextWrap(false);
  matrix1.setTextSize(1);
  matrix1.setTextColor(LED_YELLOW);
  matrix1.setRotation(3);
  matrix1.setBrightness( 2 ); //0-16

  matrix2.setTextWrap(false);
  matrix2.setTextSize(1);
  matrix2.setTextColor(LED_YELLOW);
  matrix2.setRotation(3);
  matrix2.setBrightness( 2 );

  matrix3.setTextWrap(false);
  matrix3.setTextSize(1);
  matrix3.setTextColor(LED_YELLOW);
  matrix3.setRotation(3);
  matrix3.setBrightness( 2 );

  matrix4.setTextWrap(false);
  matrix4.setTextSize(1);
  matrix4.setTextColor(LED_YELLOW);
  matrix4.setRotation(3);
  matrix4.setBrightness( 2 );
}

/////////////////////////////////////////////////////////////////////////
// ARDUINO setup routine
//
void setup()
{
  Serial.begin( 9600 );
  Serial.println("Clock M");

  matrix1.begin( 0x70 );  // pass in the address 70, 74, 72,76
  matrix2.begin( 0x74 );  // pass in the address 70, 74, 72,76
  matrix3.begin( 0x72 );  // pass in the address 70, 74, 72,76
  matrix4.begin( 0x76 );  // pass in the address 70, 74, 72,76

  GPS.begin( 9600 );

  GPS.sendCommand( PMTK_SET_NMEA_OUTPUT_RMCONLY );
  GPS.sendCommand( PMTK_SET_NMEA_UPDATE_10HZ );   // 10 Hz update rate
  GPS.sendCommand( PGCMD_NOANTENNA );

  setDefaultMatrix();
  configureSensor();
}

/////////////////////////////////////////////////////////////////////////
// Main loop
//
void loop()
{

  getGPSTime();

  getClockTime();

  setMatrixBrightness();

  displayGPSTime();

  delay(300);
}
