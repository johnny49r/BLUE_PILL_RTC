#include <Arduino.h>
#include "blue_pill_rtc.h"

BLUE_PILL_RTC rtc;
_dateTimeStruct dateTime;
uint16_t sysParams[42];     // mirrors the STM32F10x backup registers

/******************************************************************************
** alarmCallback()
** 
** Called on alarm interrupt. NOTE: Not yet implemented.
******************************************************************************/
void alarmCallback(void)
{
  Serial.println("Alarm!!!");
}


/******************************************************************************
**    setup()
******************************************************************************/
void setup()
{
  uint8_t err;
  pinMode(PC13, INPUT);       // don't use PC13 - see blue_pill_rtc.h for explaination
  Serial.begin(115200);

  err = rtc.begin((voidFuncPtr) alarmCallback);              // initialize & activate RTC
  if(err != RTC_OK)
  {
    Serial.print("RTC ERR=0x");
    Serial.println(err, HEX);
  }
  dateTime.hour_format = TIME_FORMAT_24;
  dateTime.seconds = 00;
  dateTime.minutes = 24;
  dateTime.hours = 14;
  dateTime.day = 12;
  dateTime.month = 6;
  dateTime.year = 50;
  
  // UNCOMMENT TO SET THE DATE / TIME
  //rtc.setDateTime(&dateTime);

  // UNCOMMENT TO SET ALARM
  //rtc.setAlarm(rtc.getTimeStamp() + 15);  // alarm will go off 15 seconds from the current time
  
  // UNCOMMENT TO SET BACKUP REGISTERS
  // sysParams[0] = 0xDEAD;
  // sysParams[1] = 0xBEEF;
  //rtc.writeBackup(sysParams, 0, 2);

}

/******************************************************************************
**    loop()
******************************************************************************/
void loop()
{
  
  rtc.getDateTime(&dateTime);
  Serial.print("hms:");
  Serial.print(dateTime.hours);
  
  Serial.print(":");
  Serial.print(dateTime.minutes);
  Serial.print(":");
  Serial.print(dateTime.seconds);
  if(dateTime.hour_format == TIME_FORMAT_12)
  {
    if(!dateTime.am_pm)
      Serial.print(" AM");
    else
      Serial.print(" PM");
  }
  Serial.println("");

  // Print date...
  Serial.print("Y:");
  Serial.print(dateTime.year + 1970);
  Serial.print(" M:");
  Serial.print(dateTime.month);
  Serial.print(" D:");
  Serial.print(dateTime.day);
  Serial.print(" DOW:");
  Serial.println(weekdays[dateTime.weekday]);

  Serial.print("timestamp=");
  Serial.println(dateTime.timestamp);
  if(rtc.checkAlarm())
  {
    Serial.println("ALARM!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    rtc.clearAlarm();
  }
 
  sysParams[0] = 0x0;
  sysParams[1] = 0x0;
  rtc.readBackup(sysParams, 0, 2);

  Serial.print("bkp=0x");
  Serial.print(sysParams[0], HEX);
  Serial.print(" ");
  Serial.println(sysParams[1], HEX);

  delay(1000);
}
