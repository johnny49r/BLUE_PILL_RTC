/**
   ******************************************************************************
  * @file    STM32Libs_RTC.cpp
  * @author  John Hoeppner @Abbycus Consultants
  * @version V1.0.0
  * @date    June 2020
  * @brief   RTC library for STM32F10x using Arduino framework
  * 
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  * 
  *
  ******************************************************************************
  *
  ******************************************************************************
  */

#include "STM32LIBS_RTC.h"

const char *dayNames[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char *monthNames[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

/********************************************************************
 **   @brief initialize the RTC
 **   @param initAction: Reset date/time or do a complete RTC domain reset.
 **   @return None
\*******************************************************************/
void STM32LIBS_RTC::begin(uint8_t initAction)
{
  bool resetRTC = false;
  /*
   ** Do basic RTC initialization. This may be redundant on a reset or power on.
   ** Determine the state of the RTC. Possible states are:
   ** 1) RTC domain has been reset. 
   ** 2) Date time have not been set.
   ** 3) Alarm has been set.
  */
  RCC_APB1ENR |= PWREN;                     // power & backup interface clocks enabled
  PWR_CR |= DBP;                            // allow access to RTC domain
  
  getBackup(0, 10);                         // get all backup registers 
  if (initAction == INIT_TIME_RESET) 
  {
    RTC_CRH &= ~(RTC_ALRIE | RTC_SECIE);    // clear alarm & seconds interrupt
    rtc_config(CONFIG_ENTER);
    RTC_CNTH = 0x0UL;
    RTC_CNTL = 0x0UL;
    RTC_ALRH = 0x0UL;
    RTC_ALRL = 0x0UL;
    rtc_config(CONFIG_ENTER);
    _statusFlagChange((BACKUP_TIME_SET_FLAG | BACKUP_ALARM_SET_FLAG), false);    // clear internal time & alarm flags
    disableAlarm();
  }
  else if(initAction == INIT_ALARM_RESET)
  {
    disableAlarm();
  }
  else if(initAction == INIT_RTC_RESET)     // the big bang!
  {
    resetRTC = true;
  }

  /*
   ** Force LSE clock which is the external 32.768 KHz osc. It keeps running when Vbat is powered
   ** by an external battery.
  */
  _clockSource = LSE_CLOCK;
  setClockSource(_clockSource);
    
  /*
   ** init the RTC
  */
  RTC_init(HOUR_FORMAT_24,
           (_clockSource == LSE_CLOCK) ? ::LSE_CLOCK :
           (_clockSource == HSE_CLOCK) ? ::HSE_CLOCK : ::LSI_CLOCK
#if defined(STM32_CORE_VERSION) && (STM32_CORE_VERSION  > 0x01050000)
             , resetRTC
#endif
          );
  /*
   ** set configuration flag in backup regs
  */             
  _statusFlagChange(BACKUP_CONFIGURED_FLAG, true);    // set internal configured flag

}


/********************************************************************
 **   @brief Deinitialize and stop the RTC
 **   @param None
 **   @returns Nothing
\*******************************************************************/
void STM32LIBS_RTC::end(void)
{
  if (isConfigured()) 
  {
    RTC_CRL = 0x0;            // clear interrupts and pending flags
    RTC_CRH = 0x0;
    _statusFlagChange((BACKUP_CONFIGURED_FLAG | BACKUP_TIME_SET_FLAG | BACKUP_ALARM_SET_FLAG), false); // clear all rtc flags 
  }
}


/********************************************************************
 **   @brief Gets the RTC clock source (should be LSE_CLOCK).
 **   @returns clock source: LSI_CLOCK, LSE_CLOCK or HSE_CLOCK
\*******************************************************************/
STM32LIBS_RTC::Source_Clock STM32LIBS_RTC::getClockSource(void)
{
  return _clockSource;
}


/********************************************************************
 **   @brief Sets the RTC clock source. By default LSI clock is selected. This
 **     method is called from begin().
 **   @param source: clock source: LSI_CLOCK, LSE_CLOCK or HSE_CLOCK
 **   @returns Nothing
\*******************************************************************/
void STM32LIBS_RTC::setClockSource(Source_Clock source)
{
  if (IS_CLOCK_SOURCE(source)) {
    _clockSource = source;
    RTC_SetClockSource((_clockSource == LSE_CLOCK) ? ::LSE_CLOCK :
                       (_clockSource == HSE_CLOCK) ? ::HSE_CLOCK : ::LSI_CLOCK);
  }
}

/********************************************************************
 **   @brief  get user (a)synchronous prescaler values if set else computed
 **     ones for the current clock source.
 **   @param  predivA: pointer to the current Asynchronous prescaler value
 **   @param  predivS: pointer to the current Synchronous prescaler value
 **   @retval None
\*******************************************************************/
void STM32LIBS_RTC::getPrediv(int8_t *predivA, int16_t *predivS)
{
  if ((predivA != nullptr) && (predivS != nullptr)) 
  {
    RTC_getPrediv(predivA, predivS);
  }
}

/********************************************************************
  * @brief  set user (a)synchronous prescalers value.
  * @note   This method must be called before begin().
  * @param  predivA: Asynchronous prescaler value. Reset value: -1
  * @param  predivS: Synchronous prescaler value. Reset value: -1
  * @retval None
\*******************************************************************/
void STM32LIBS_RTC::setPrediv(int8_t predivA, int16_t predivS)
{
  RTC_setPrediv(predivA, predivS);
}


/********************************************************************
  * @brief  Set & enable alarm using date & time values
  * @param  pointer to RTC_datetime_t structure containing the alarm
  * @retval None
\*******************************************************************/
uint8_t STM32LIBS_RTC::setAlarmDateTime(RTC_datetime_t *alarm_datetime)
{
  uint8_t retn;
  uint32_t alarm_epoch = dateTimeToEpoch(alarm_datetime);

  if(alarm_datetime->hour_format == RTC_HOUR_FORMAT_12 && alarm_datetime->am_pm == RTC_HOUR_PM)
  {
    alarm_epoch += (SECS_PER_DAY / 2);
  }

  if(alarm_epoch <= getEpoch())    // alarm must be in the future
    retn = RTC_INVALID_PARAM;
  else
    retn = setAlarmFromEpoch(alarm_epoch);

  return retn;
}


/********************************************************************
  * @brief  Set & enable alarm using an epoch number
  * @param  epoch - 32 bit number of seconds since 1970
  * @retval None
\*******************************************************************/
uint8_t STM32LIBS_RTC::setAlarmFromEpoch(uint32_t alarm_epoch)
{
  uint8_t retn = RTC_OK;
  if(alarm_epoch <= getEpoch())   // alarm must be > current time
    return RTC_INVALID_PARAM;

  __disable_irq();

  // disable RTC alarm interrupt 
  RTC_CRH &= ~RTC_ALRIE;

  // write the alarm registers
  retn = rtc_config(CONFIG_ENTER);
  RTC_ALRH = alarm_epoch >> 16;
  RTC_ALRL = alarm_epoch & 0xFFFF;
  retn = rtc_config(CONFIG_EXIT);

  // clear RTC alarm pending flag in CRL reg
  RTC_CRL &= ~RTC_CRL_ALARMF;
  // enable RTC alarm interrupt flag in CRH reg
  RTC_CRH |= RTC_ALRIE;
  // enable alarm interrupt in EXTI IMR reg
  EXTI_IMR |= EXTI_LINE17;
  // set EXTI rising edge alarm trigger 
  EXTI_RTSR |= EXTI_LINE17; 

  __enable_irq();

  _statusFlagChange(BACKUP_ALARM_SET_FLAG, true);
  return retn;
}


/********************************************************************
  * @brief  disable the RTC alarm.
  * @retval None
\*******************************************************************/
void STM32LIBS_RTC::disableAlarm(void)
{
  if (isConfigured()) 
  {
    RTC_CRL &= ~RTC_CRL_ALARMF;               // clear alarm flag
    RTC_CRH &= ~RTC_ALRIE;
    _statusFlagChange(BACKUP_ALARM_SET_FLAG, false);
  }
}


/********************************************************************
  * @brief  attach a callback to the RTC alarm interrupt.
  * @param  callback: pointer to the callback function
  * @retval None
  * 
\*******************************************************************/
void STM32LIBS_RTC::attachInterrupt(voidFuncPtr callback, void *data)
{
  attachAlarmCallback(callback, data);
}


/********************************************************************
  * @brief  detach the RTC alarm callback.
  * @param  None
  * @retval None
\*******************************************************************/
void STM32LIBS_RTC::detachInterrupt(void)
{
  detachAlarmCallback();
}

// Kept for compatibility. Use STM32LowPower library.
void STM32LIBS_RTC::standbyMode(void)
{

}

/********************************************************************
  * @brief  Get weekday name.
  * @param  DOW (0 - 6), 0 == "Sunday"
  * @retval returns a char * to the name of the week.
\*******************************************************************/
char* STM32LIBS_RTC::getWeekdayName(uint8_t DOW)
{
  if(DOW > 6)
    DOW = 0;
  char *pStr = (char *)dayNames[DOW];
  return pStr;
}


/********************************************************************
  * @brief  Get month name.
  * @param  month (1 - 12), 1 == "January"
  * @retval returns a char * to the name of the month.
\*******************************************************************/
char* STM32LIBS_RTC::getMonthName(uint8_t month)
{
  if(month > 12)
    month = 1;
  char *pStr = (char *)monthNames[month -1];
  return pStr;
}


/********************************************************************
  *  @brief  configure RTC source clock for low power
  *  @param  none
\*******************************************************************/
void STM32LIBS_RTC::configForLowPower(Source_Clock source)
{
#if defined(HAL_PWR_MODULE_ENABLED)
  #ifdef _IGNORE
  if (!_configured) {
    _clockSource = source;
    // Enable RTC
    //begin();
  } else {
    if (_clockSource != source) {
      // Save current config
      AM_PM period, alarmPeriod = _alarmPeriod;
      uint32_t subSeconds;
      uint8_t seconds, minutes, hours, weekDay, day, month, years;
      uint8_t alarmSeconds, alarmMinutes, alarmHours, alarmDay;
      Alarm_Match alarmMatch = _alarmMatch;
      bool alarmEnabled = _alarmEnabled;

      alarmDay = _alarmDay;
      alarmHours = _alarmHours;
      alarmMinutes = _alarmMinutes;
      alarmSeconds = _alarmSeconds;

      getDate(&weekDay, &day, &month, &years);
      getTime(&seconds, &minutes, &hours, &subSeconds, &period);

      end();
      _clockSource = source;
      // Enable RTC
      //begin(period);
      // Restore config
      setTime(seconds, minutes, hours, subSeconds, period);
      setDate(weekDay, day, month, years);
      setAlarmTime(alarmHours, alarmMinutes, alarmSeconds, alarmPeriod);
      setAlarmDay(alarmDay);
      if (alarmEnabled) {
        enableAlarm(alarmMatch);
      }
    }
  }
  #endif
#endif
}


/******************************************************************************
**    @brief Writes an array of user data to the STM32F1xx data registers. Up to 9 
**    16-bit values can be stored and these registers are non-volatile if the 
**    Vbat input is powered with a coin-cell or other equivalent power source.
**    @param data_array - pointer to user data array.
**    @param indx - User register (0 - 8)
**    @param len - number of registers to read (1 - 9).
**    @note: The datasheet says there are 42 regs available but not all devices
**      support more than 10.
**      Indx + len should not be greater than 9.
\*****************************************************************************/
void STM32LIBS_RTC::eepromWrite(uint16_t data_array[], uint8_t indx, uint8_t len)
{
  uint8_t i;
  uint8_t _indx = indx;

  indx += 1;                // can't use first reg
  for(i=0; i<len; i++)
  {
    if(indx > 9)
      break;

    _RTC_BackupRegs[indx++] = data_array[i];
  }
  setBackup(_indx+1, len);
}


/******************************************************************************
**    @brief Reads user data from the RTC backup registers (poor mans EEPROM)
**      Up to nine (9) 16-bit values can be read and these registers are non-volatile 
**      if the Vbat input is powered with a coin-cell or other equivalent power.
**    @param data_array - pointer to user data array.
**    @param indx - User register (0 - 8)
**    @param len - number of registers to read.
**    @note indx + len should not be greater than 9.
\*****************************************************************************/
void STM32LIBS_RTC::eepromRead(uint16_t data_array[], uint8_t indx, uint8_t len)
{
  uint8_t i;
  uint8_t _indx = indx;

  if(data_array != NULL)
  {
    getBackup(indx+1, len);         // read backup regs into local array
    indx = _indx+1;
    for(i=0; i<len; i++)
    {
      if(indx > 9)
        return;

      data_array[i] = _RTC_BackupRegs[indx++];  // copy local array to caller array
    }
  }
}


/******************************************************************************
**    setBackup()
**
**    Private access to first four STM32F10x backup regs. These are used to
**    store status and alarm settings during power down. There are 42 16-bit
**    registers that are non-volatile if the Vbat input is powered with a 
**    coin-cell or other equivalent power source.
\*****************************************************************************/
void STM32LIBS_RTC::setBackup(uint8_t indx, uint8_t len)
{
  pBkpRegs pb;
  pb = reinterpret_cast<BACKUP_REGS *>(BKP_REG_BASE + 0x00000004);

  while(len > 0)
  {
    if(indx > 9)
      break;

    pb->bkup_regs[indx] = _RTC_BackupRegs[indx];
    indx++;
    len--;
  }
  
}
      
      
/******************************************************************************
**    getBackup()
**
**    Reads up to 42 half word (16 bit) values from the STM32F10x backup regs.
**    The backup regs are non-volatile if the Vbat input is powered with a 
**    coin-cell or other 3V power source.
\*****************************************************************************/      
void STM32LIBS_RTC::getBackup(uint8_t indx, uint8_t len)
{
  pBkpRegs pb;
  pb = reinterpret_cast<BACKUP_REGS *>(BKP_REG_BASE + 0x00000004);

  while(len > 0)
  {
    if(indx > 9)
      break;
    
    _RTC_BackupRegs[indx] = (uint16_t)(pb->bkup_regs[indx] & 0xFFFF);
    indx++;
    len--;
  }
}


/******************************************************************************
**    clearBackup()
**
**    Clears all backup registers. *** WARNING *** this resets the RTC domain 
**    including time and alarm settings.
\*****************************************************************************/ 
void STM32LIBS_RTC::clearBackup(void)
{
   RCC_BDCR |= BKP_RESET;     // assert BDRST
   RCC_BDCR &= ~BKP_RESET;     // deassert BDRST

   // backup domain reset disables RTC access - reenable
   PWR_CR |= DBP;
}


/******************************************************************************
// setDateTime()
//
// datetime - ptr to _dateTimeStruct
// useRaw: if true use 'time_stamp' to set RTC count register. Otherwise convert
// date & time info in datetime to a 32 bit value representing the number of 
// seconds since midnight of Jan 1, 2000.
\*****************************************************************************/
void STM32LIBS_RTC::setDateTime(RTC_datetime_t *datetime)
{
  if(isConfigured())
  {
    uint32_t _epoch = dateTimeToEpoch((RTC_datetime_t *)datetime); // convert date/time units to 32 bit epoch
    //
    // if 12 hour format add extra 12 hours for PM
    //
    if(datetime->hour_format == RTC_HOUR_FORMAT_12 && datetime->am_pm == RTC_HOUR_PM)
    {
      _epoch += (SECS_PER_DAY / 2);
    }
    setEpoch(_epoch);
  }
}


/******************************************************************************
**    @brief Gets the current time from the RTC count register and calculates
**      date & time elements from the 'epoch'. This is non-volatile if
**      power if not lost or if Vbat is powered by an external battery.
**    @param datetime - ptr to RTC_datetime_t structure to fill.
**    @param hour_format - if unspecified, the format will use the hour_format
**      specified in the RTC_datetime_t structure. Otherwise the format will 
**      use the optional hour_format parameter. 
**    @note If the optional hour_format parameter is used this value becomes the
**      new default format.
**
\*****************************************************************************/
void STM32LIBS_RTC::getDateTime(RTC_datetime_t *_datetime, uint8_t hour_format)
{
  //uint8_t _hour_fmt;
  if(hour_format != RTC_HOUR_FORMAT_UNDEF)
    _datetime->hour_format = hour_format;

  // epochToDateTime always yeilds 24 hour time
  epochToDateTime((RTC_datetime_t *)_datetime, getEpoch());

  // if 12 hour format is requested, convert to 12 hour AM/PM
  if(_datetime->hour_format == RTC_HOUR_FORMAT_12)
  {
    _datetime->am_pm = (_datetime->hours >= 12) ? RTC_HOUR_PM : RTC_HOUR_AM;
    _datetime->hours %= 12;
    if(_datetime->hours == 0)
       _datetime->hours = 12;               // no zero o'clock
  }
  else
    _datetime->am_pm = RTC_HOUR_AM;         // default - its AM
}


/******************************************************************************
**    @brief Gets the epoch from the RTC count regs (num of secs from 1970)
**
**    @return 32 bit epoch 
**
\*****************************************************************************/
uint32_t STM32LIBS_RTC::getEpoch(void)
{
   uint32_t _tm;
   _tm = (RTC_CNTH << 16UL) | RTC_CNTL;
   return _tm;
}

/******************************************************************************
**    @brief Sets the epoch number in the RTC count regs (num of secs from 1970)
**    @param _epoch - 32 bit number of seconds since 1970
**
\*****************************************************************************/
void STM32LIBS_RTC::setEpoch(uint32_t _epoch)
{
  __disable_irq();

  rtc_config(CONFIG_ENTER);
  RTC_CNTH = _epoch >> 16;
  RTC_CNTL = _epoch & 0xFFFF;
  rtc_config(CONFIG_EXIT);

  __enable_irq();

  _statusFlagChange(BACKUP_TIME_SET_FLAG, true);
}


/******************************************************************************
**    @brief Converts date / time elements to a 32 bit epoch (num of secs since 1970).
**
**    @param datetime - ptr to RTC_datetime_t structure containing datetime elements.
**    @returns A 32 bit epoch.
**    @note: The epoch variable in the datetime struct is updated also.
**
\*****************************************************************************/
uint32_t STM32LIBS_RTC::dateTimeToEpoch(RTC_datetime_t *datetime)
{
  int16_t i;
  uint32_t _seconds;   
  int16_t _year = (datetime->year - 1970);    // 1970 is the beginning of time!

  if(_year < 0)
    _year = 0;

   // seconds from 1970 till 1 jan 00:00:00 of the given year
   _seconds = _year * (SECS_PER_DAY * 365);
   for (i = 0; i < _year; i++) {
      if (IS_LEAP_YEAR(i)) {
         _seconds += SECS_PER_DAY;   // add extra days for leap years
      }
   }
  
   // add days for this year, months start from 1
   for (i = 1; i < datetime->month; i++) 
   {
      if ( (i == 2) && IS_LEAP_YEAR(_year)) 
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
   datetime->epoch = _seconds;
   return (uint32_t) _seconds; 
}


/******************************************************************************
**    @brief Converts a 32 bit epoch value to date & time elements.
**
**    @param datetime - ptr to RTC_datetime_t structure to fill with datetime elements.
**    @param _epoch - 32 bit number of seconds since 1970.
**    @note - Date & Time are returned in 24 hour format. Caller must convert to
**      12 hour format if needed.
**
\*****************************************************************************/
void STM32LIBS_RTC::epochToDateTime(RTC_datetime_t *datetime, uint32_t _epoch)
{
   // working variables
   uint8_t _month, monthLength;
   uint32_t _days = 0;
   uint32_t _time = _epoch;

   datetime->epoch = _epoch;
   datetime->seconds = _time % 60;
   _time /= 60;                     // time = minutes
   datetime->minutes = _time % 60;
   _time /= 60;                     // time = hours
   datetime->hours = _time % 24;
   _time /= 24;                     // time = days
   // calc day of the week
   datetime->weekday = ((_time + 4) % 7);   // jan 1 1970 was a thursday
 
   datetime->year = 0;
   while((unsigned)(_days += (IS_LEAP_YEAR(datetime->year) ? 366 : 365)) <= _time) 
   {
      datetime->year++;
   }

   _days -= IS_LEAP_YEAR(datetime->year) ? 366 : 365;
   _time -= _days; // now it is days in this year, starting at 0
  
   _days = 0;
   monthLength = 0;
   for (_month=0; _month<12; _month++) 
   {
      if (_month == 1) // february
      { 
         if (IS_LEAP_YEAR(datetime->year)) 
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
   datetime->year += 1970;
   datetime->month = _month + 1;  // jan is month 1  
   datetime->day = _time + 1;     // day of month starting with 1
}


/******************************************************************************
**    @brief RTC register CONFIGURATION mode enable/disable.
**
**    @param _config - CONFIG_ENTER or CONFIG_EXIT
**    @note The RTC count, alarm, or prescale regs can only be update when 
**      configuration mode is enabled.
**
\*****************************************************************************/
uint8_t STM32LIBS_RTC::rtc_config(uint8_t _config)
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
          retn = RTC_FAIL_CONFIG_ENTER;
          break;
        }
      }    
      if(retn == RTC_OK)
      {
        RTC_CRL |= CNF;                  // enter config mode
        RTC_CRL &= ~RSF;                 // clear the reg sync flag 
      }
      break;

    case CONFIG_EXIT:
      RTC_CRL &= ~CNF;                    // exit config mode
      while((RTC_CRL & RTOFF_RSF) != RTOFF_RSF)    // wait for last write & reg sync
      {       
        if(millis() - tmo > REG_TIMEOUT)
        {
          retn = RTC_FAIL_CONFIG_EXIT;
          break;
        }
      }    
      break;         
   }
   return retn;
}


/******************************************************************************
**    Change status flags in backup register.
**
**    Maintains the state of time, alarm, and configuration set.
**
\*****************************************************************************/
void STM32LIBS_RTC::_statusFlagChange(uint16_t sbit, bool fset)
{
  if(fset)
    _RTC_BackupRegs[0] |= sbit;
  else   
    _RTC_BackupRegs[0] &= ~sbit;

  setBackup(0, 1);
}
