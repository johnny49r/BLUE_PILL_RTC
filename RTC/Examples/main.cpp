/********************************************************************
 *    main.cpp
 * 
 *    Example program using the STM32LIBS_RTC real time clock library.
 *    This demonstrates basic RTC functions of setting date & time, 
 *    getting date & time, using the alarm with a callback, and saving 
 *    values in the RTC backup registers.
 * 
 * 
\*******************************************************************/

#include <Arduino.h>
#include <STM32LIBS_RTC.h>

/* Get the rtc object */
STM32LIBS_RTC& rtc = STM32LIBS_RTC::getInstance();
RTC_datetime_t datetime;
RTC_datetime_t alarm_datetime;
volatile bool alarmEvent = false;
uint8_t rtc_config_status;
uint16_t user_data[10];


/********************************************************************
 *  @brief alarm event callback
 *  @param data - pointer to data that can be passed to this callback.
 *    In this case a number is passed which causes another alarm event.
\*******************************************************************/
void alarmMatch(void *data)
{
  uint32_t sec = 1;
  uint32_t ts;

  RTC_CRH &= ~RTC_ALRIE;      // disable alarm interrupt

  if(data != NULL) {
    sec = *(uint32_t*)data;
    // Minimum is 1 second
    if (sec == 0)
      sec = 1;
  }
  alarmEvent = true;
  ts = rtc.getEpoch() + sec;
  rtc.setAlarmFromEpoch(ts);
}


/* Change this value to set alarm match offset */
static uint32_t atime = 5;

/********************************************************************
 ** @brief  setup()
\*******************************************************************/
void setup()
{
  static uint8_t i;
  Serial.begin(9600);

  /**
  ** initialize the RTC 
  **
  ** if time has not been set, do whatever to properly set the time
  **/
  datetime.hour_format = RTC_HOUR_FORMAT_12;  // set structure default hour format
  rtc.begin(INIT_NONE);                       // initialize the RTC
  if(!rtc.isTimeSet())
  {
    // initialize the datetime structure 
    datetime.year = 2020;
    datetime.month = 6;
    datetime.day = 22;
    datetime.hours = 8;
    datetime.minutes = 39;
    datetime.seconds = 0;
    datetime.am_pm = RTC_HOUR_AM;
    rtc.setDateTime(&datetime);
  }

  /**
  ** attach a callback function to the alarm interrupt
  **/
  rtc.attachInterrupt(alarmMatch, &atime);    // the second arg can pass data to alarm callback

  /**
  ** set a relative alarm using the current epoch + 'n' seconds
  **/
  rtc.setAlarmFromEpoch( rtc.getEpoch() + 10);  // alarm 10 secs from now

  /**
  ** set an absolute alarm using a RTC_datetime_t structure
  **/
  // alarm_datetime.hour_format = RTC_HOUR_FORMAT_24;
  // alarm_datetime.year = 2020;
  // alarm_datetime.month = 6;
  // alarm_datetime.day = 21;
  // alarm_datetime.hours = 15;
  // alarm_datetime.minutes = 11;
  // alarm_datetime.seconds = 48;
  // rtc.setAlarmDateTime(&alarm_datetime);

  // load 'eeprom' with data
  for(i=0; i<10; i++)
    user_data[i] = (i+1)*16;    // increment test data by 10

  //rtc.eepromWrite(user_data, 0, 9);
}


/********************************************************************
 ** @brief  loop()
\*******************************************************************/
void loop()
{
  static uint8_t i;

  for(i=0; i<10; i++)
  {
    user_data[i] = 0x0;
  }
  rtc.getDateTime(&datetime);
  Serial.print("Time(hms) ");
  Serial.print(datetime.hours);
  Serial.print(":");
  Serial.print(datetime.minutes);
  Serial.print(":");  
  Serial.print(datetime.seconds);
  if(datetime.hour_format == RTC_HOUR_FORMAT_12)
  {
    if(datetime.am_pm == RTC_HOUR_AM)
      Serial.print(" AM");
    else
      Serial.print(" PM");
  }
  Serial.println("");

  Serial.print("Date(DMY) ");
  Serial.print(rtc.getWeekdayName(datetime.weekday));
  Serial.print(", ");
  Serial.print(rtc.getMonthName(datetime.month));
  Serial.print(" ");
  Serial.print(datetime.day);
  Serial.print(", ");
  Serial.println(datetime.year);

  if(alarmEvent)
  {
    Serial.println(">>>>> Alarm Event!!!!!!!!!!!");
    alarmEvent = false;
  }

  // print debug stuff
  rtc.eepromRead(user_data, 0, 9);
  Serial.print("eeprom= ");
    for(i=0; i<9; i++)
    {
      Serial.print(user_data[i], HEX);
      Serial.print(" ");
    }
  Serial.println("");  
  Serial.println("");

  delay(1000);
}
