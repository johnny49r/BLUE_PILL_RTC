#include "blue_pill_rtc.h"

const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; 
const char* weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// constructor
BLUE_PILL_RTC::BLUE_PILL_RTC()
{
   // nothing to do
}


/******************************************************************************
**    begin()
**
**    initialize the RTC
******************************************************************************/
uint8_t BLUE_PILL_RTC::begin(voidFuncPtr callback)
{
   uint8_t retn = RTC_OK;
   uint32_t tmo = 0;

   // init callback for alarm
   _callback = callback;

   // enable peripheral clocks 
   RCC_APB1ENR |= PWREN;

   // enable access to RCC & RTC
   PWR_CR |= DBP;

   // select the LSE (external 32768 Khz clock) and enable RTC
   if(RCC_BDCR & LSEBYP)      // if the 32KHz osc is bypassed the backup domain must be reset
   {
      RCC_BDCR = BKP_RESET;
      RCC_BDCR = 0x0UL;
   }
   RCC_BDCR = BDCR_INIT;

   // wait for ext clk stable   
   tmo = millis();
   while((RCC_BDCR & LSERDY) == 0) 
   {
      if(millis() - tmo > REG_TIMEOUT)
      {
         retn = RTC_TIMEOUT;
         break;
      }
   }        
   // wait for register sync
   tmo = millis();
   while(true) 
   {
      if((RTC_CRL & RTOFF_RSF) == RTOFF_RSF)    // exit of RTOFF & RSF bits are both set
         break;
      else if(millis() - tmo > REG_TIMEOUT)
      {
         retn = RTC_TIMEOUT;
         break;
      }
   }   

   return retn;
}


/******************************************************************************
**    writeBackup()
**
**    Writes up to 42 half word (16 bit) values to the STM32F10x backup regs.
**    The backup regs are non-volatile if the Vbat input is powered with a 
**    coin-cell or other 3V power source.
******************************************************************************/
void BLUE_PILL_RTC::writeBackup(uint16_t words[], uint8_t indx, uint8_t len)
{
   uint8_t i;
   uint32_t _indx = (indx * 0x04UL) + 0x04UL;

   if((indx + len) > 42)         // check for overflow - only 42 backup regs available
      len = 42 - indx;

   for(i = 0; i < len; i++)
   {
      if(_indx <= 0x28)
         *(BKP_REGS +_indx) = words[i];
      else
         *(BKP_REGS +_indx + 0x14) = words[i];
      _indx += 4;
   }
}
      
      
/******************************************************************************
**    readBackup()
**
**    Reads up to 42 half word (16 bit) values from the STM32F10x backup regs.
**    The backup regs are non-volatile if the Vbat input is powered with a 
**    coin-cell or other 3V power source.
******************************************************************************/      
void BLUE_PILL_RTC::readBackup(uint16_t words[], uint8_t indx, uint8_t len)
{
   uint8_t i;
   uint32_t _indx = (indx * 0x04UL) + 0x04UL;

   if((indx + len) > 42)         // check for overflow - only 42 backup regs available
      len = 42 - indx;

   for(i = 0; i < len; i++)
   {
      if(_indx <= 0x28)
         words[i] = *(BKP_REGS +_indx);
      else
         words[i] = *(BKP_REGS +_indx + 0x14);
      _indx += 4;
   }
}


/******************************************************************************
**    clearBackup()
**
**    Clears all backup registers.
******************************************************************************/ 
void BLUE_PILL_RTC::clearBackup(void)
{
   RCC_BDCR |= 0x00010000UL;     // assert BDRST
   RCC_BDCR &= 0x00008307UL;     // deassert BDRST

   // backup domain reset disables RTC access - reenable
   PWR_CR |= DBP;
}

/******************************************************************************
**    setAlarm()
**
**    Sets alarm time and enables alarm interrupt
**    alarmTime - 32 bit value representing the number of seconds from 1970.
******************************************************************************/
void BLUE_PILL_RTC::setAlarm(uint32_t alarmTime)
{
   rtc_config(CONFIG_ENTER);
   RTC_CRH &= RTC_ALRIE_MASK;          // disable interrupt
   RTC_CRL &= ALARMF_MASK;             // clear alrf, rsf, owf, & secf
   RTC_ALRH = alarmTime >> 16UL;       // load alarm count reg
   RTC_ALRL = alarmTime & 0xFFFF;
   //if(_callback != NULL)               // if callback initialized, enable alarm interrupt
      //RTC_CRH |= RTC_ALRIE;   
   rtc_config(CONFIG_EXIT);

}


/******************************************************************************
**    clearAlarm()
**
**    Disables any alarm
******************************************************************************/
void BLUE_PILL_RTC::clearAlarm(void)
{
   rtc_config(CONFIG_ENTER);
   RTC_CRL &= ALARMF_MASK;             // clear alrf, rsf, owf, & secf
   RTC_CRH &= RTC_ALRIE_MASK;          // disable interrupt
   rtc_config(CONFIG_EXIT);
}


/******************************************************************************
**    checkAlarm()
**
**    Retruns true if alarm has been triggered.
******************************************************************************/
bool BLUE_PILL_RTC::checkAlarm(void)
{
   return (RTC_CRL & ALARMF);    // true if alarm has been set

}

/******************************************************************************
// setDateTime()
//
// datetime - ptr to _dateTimeStruct
// useRaw: if true use 'time_stamp' to set RTC count register. Otherwise convert
// date & time info in datetime to a 32 bit value representing the number of 
// seconds since midnight of Jan 1, 2000.
******************************************************************************/
void BLUE_PILL_RTC::setDateTime(_dateTimeStruct *datetime)
{
   timeCompress((_dateTimeStruct *)datetime); // convert date/time units to 32 bit timestamp
   setTimeStamp(datetime->timestamp);
}


/******************************************************************************
**    getDateTime()
**
**    datetime - ptr to _dateTimeStruct
**    useRaw: if true use 'time_stamp' to set RTC count register. Otherwise convert
**    date & time info in datetime to a 32 bit value representing the number of 
**    seconds since midnight of Jan 1, 2000.
******************************************************************************/
void BLUE_PILL_RTC::getDateTime(_dateTimeStruct *_datetime)
{
   _datetime->timestamp = getTimeStamp();  // get current timestamp

   timeExpand((_dateTimeStruct *)_datetime);
   if(_datetime->hour_format == TIME_FORMAT_12)
   {
      _datetime->am_pm = (_datetime->hours >= 12) ? true : false;
      _datetime->hours %= 12;
      if(_datetime->hours == 0)
         _datetime->hours = 12;        // no zero o'clock
   }
   else
      _datetime->am_pm = false;        // its AM
}


/******************************************************************************
**    getTimeStamp()
**
**    Returns the RTC raw count (num of secs from 1970)
**
******************************************************************************/
uint32_t BLUE_PILL_RTC::getTimeStamp(void)
{
   uint32_t _tm;
   _tm = (RTC_CNTH << 16UL) | RTC_CNTL;
   return _tm;
}


/******************************************************************************
**    setTimeStamp()
**
**    Returns the RTC raw count (num of secs from 1970)
**
******************************************************************************/
void BLUE_PILL_RTC::setTimeStamp(uint32_t ts)
{
   rtc_config(CONFIG_ENTER);
   RTC_CNTL = ts & 0xFFFF;
   RTC_CNTH = ts >> 16;
   rtc_config(CONFIG_EXIT);
}


/******************************************************************************
**    timeExpand()
**
**    Converts timestamp (raw seconds) into date & time 
**    secsFrom1970 - 32 bit number of seconds since time began: Jan 1, 1970
**    datetime - ptr to _dateTimeStruct
**
******************************************************************************/
void BLUE_PILL_RTC::timeExpand(_dateTimeStruct *datetime, uint32_t _timestamp)
{
   // break the given time_t into time components
   // this is a more compact version of the C library localtime function
   // note that year is offset from 1970 !!!

   // leap year calculator expects year argument as years offset from 1970
   #define LEAP_YEAR(Y)   ( ((1970+(Y))>0) && !((1970+(Y))%4) && ( ((1970+(Y))%100) || !((1970+(Y))%400) ) )

   // days in each month   
   uint8_t _month, monthLength;
   uint32_t _days = 0;
   uint32_t _time = _timestamp;
   datetime->timestamp = _timestamp;
   datetime->seconds = _time % 60;
   _time /= 60;                     // time = minutes
   datetime->minutes = _time % 60;
   _time /= 60;                     // time = hours
   datetime->hours = _time % 24;
   _time /= 24;                     // time = days
   // calc day of the week
   datetime->weekday = ((_time + 4) % 7); // + 1; ;  // jan 1 was a thursday
 
   datetime->year = 0;
   while((unsigned)(_days += (LEAP_YEAR(datetime->year) ? 366 : 365)) <= _time) 
   {
      datetime->year++;
   }
  
   _days -= LEAP_YEAR(datetime->year) ? 366 : 365;
   _time -= _days; // now it is days in this year, starting at 0
  
   _days = 0;
   monthLength = 0;
   for (_month=0; _month<12; _month++) 
   {
      if (_month == 1) // february
      { 
         if (LEAP_YEAR(datetime->year)) 
         {
         monthLength = 29;
         } 
         else 
         {
         monthLength=28;
         }
      } 
      else 
      {
         monthLength = monthDays[_month];
      }
    
      if (_time >= monthLength) 
      {
         _time -= monthLength;
      } 
      else 
      {
         break;
      }
   }
   datetime->month = _month + 1;  // jan is month 1  
   datetime->day = _time + 1;     // day of month starting with 1
}


/******************************************************************************
**    timeCompress()
**
**    Compress time elements into a 32 bit value - number of seconds since 1970
**    datetime - ptr to _dateTimeStruct
**    Returns a 32 bit number of seconds
**
******************************************************************************/
uint32_t BLUE_PILL_RTC::timeCompress(_dateTimeStruct *datetime)
{
   int16_t i;
   uint32_t _seconds;

   // seconds from 1970 till 1 jan 00:00:00 of the given year
   _seconds = datetime->year * (SECS_PER_DAY * 365);
   for (i = 0; i < datetime->year; i++) {
      if (LEAP_YEAR(i)) {
         _seconds += SECS_PER_DAY;   // add extra days for leap years
      }
   }
  
   // add days for this year, months start from 1
   for (i = 1; i < datetime->month; i++) 
   {
      if ( (i == 2) && LEAP_YEAR(datetime->year)) 
      { 
         _seconds += SECS_PER_DAY * 29;
      } 
      else 
      {
         _seconds += SECS_PER_DAY * monthDays[i-1];  // monthDay array starts from 0
      }
   }
   _seconds += (datetime->day-1) * SECS_PER_DAY;
   _seconds += datetime->hours * SECS_PER_HOUR;
   _seconds += datetime->minutes * SECS_PER_MIN;
   _seconds += datetime->seconds;
   datetime->timestamp = _seconds;
   return (uint32_t) _seconds; 
}


// --------------------------------------------------------
// rtc_config() - enter/exit RTC configure mode
// --------------------------------------------------------
uint8_t BLUE_PILL_RTC::rtc_config(uint8_t _config)
{
   uint32_t tmo = millis();
   uint8_t retn = RTC_OK;

   switch(_config)
   {
      case CONFIG_ENTER:
         while((RTC_CRL & RTOFF) == 0)       // wait for last write to terminate
         {
            if(millis() - tmo > REG_TIMEOUT)
            {
               retn = RTC_TIMEOUT;
               break;
            }
         }    
         RTC_CRL |= CNF;                     // enter config mode
         break;

      case CONFIG_EXIT:
         RTC_CRL &= ~CNF;                    // exit config mode
         while(true)                         // wait for last write & reg sync
         {       
            if((RTC_CRL & RTOFF_RSF) == RTOFF_RSF)
               break;
            else if(millis() - tmo > REG_TIMEOUT)
            {
               retn = RTC_TIMEOUT;
               break;
            }
         }    
         break;         
   }
   return retn;
}
