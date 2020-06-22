/******************************************************************************
  * @file    STM32Libs_RTC.h
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
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  * This library is based on the STM32LIBS_RTC library from STMicro but adds
  * features and corrections. The above copyright and list of conditions for 
  * distribution are preserved for legal purposes.
  * 
  * The library has been tested with the STM32F103 MPU with the Arduino framework.
  * 
  * John Hoeppner @ Abbycus Consultants 2020
  * 
  ****************************************************************************/

#ifndef __STM32LIBS_RTC_H
#define __STM32LIBS_RTC_H

#include "Arduino.h"
#include "STM32LIBS_REGS.h"
#include <time.h>
#include "stm32f1xx_hal.h"  

// Check if RTC HAL enable in variants/board_name/stm32yzxx_hal_conf.h
#ifndef HAL_RTC_MODULE_ENABLED
#error "RTC configuration is missing. Check flag HAL_RTC_MODULE_ENABLED in variants/board_name/stm32yzxx_hal_conf.h"
#endif


const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; 


typedef struct 
{
  uint8_t hour_format;    // RTC_HOUR_FORMAT_12 or RTC_HOUR_FORMAT_24
  uint8_t am_pm;          // RTC_HOUR_AM, RTC_HOUR_PM (if 12 hour time format)
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint32_t epoch;         // raw datetime - seconds since 1970

  uint8_t day;            // day of the month, 1 - 31
  uint8_t weekday;        // 1(Sunday) - 7(Saturday)
  uint8_t month;          // month, 1 - 12
  uint16_t year;          // 1970 & up
} RTC_datetime_t;   

enum  {
  RTC_HOUR_FORMAT_12,     // generic hour format defines
  RTC_HOUR_FORMAT_24, 
  RTC_HOUR_FORMAT_UNDEF,
};

enum  {
  RTC_HOUR_AM,
  RTC_HOUR_PM,
};

// initialization actions
enum {
  INIT_NONE,
  INIT_TIME_RESET,
  INIT_ALARM_RESET,
  INIT_RTC_RESET,         // *** WARNING! *** this resets the RTC domain! Time, date, and backup regs are wiped.
};


// leap year calculator expects year argument as years offset from 1970
#define IS_LEAP_YEAR(Y)   ( ((1970+(Y))>0) && !((1970+(Y))%4) && ( ((1970+(Y))%100) || !((1970+(Y))%400) ) )


typedef void(*voidFuncPtr)(void *);

#define IS_CLOCK_SOURCE(SRC) (((SRC) == STM32LIBS_RTC::LSI_CLOCK) || ((SRC) == STM32LIBS_RTC::LSE_CLOCK) ||\
                              ((SRC) == STM32LIBS_RTC::HSE_CLOCK))
#define IS_HOUR_FORMAT(FMT)  (((FMT) == STM32LIBS_RTC::HOUR_12) || ((FMT) == STM32LIBS_RTC::HOUR_24))

//
// STM32LIBS_RTC class
//
class STM32LIBS_RTC {
  public:

    enum Hour_Format : uint8_t {
      HOUR_12 = HOUR_FORMAT_12,
      HOUR_24 = HOUR_FORMAT_24
    };

    enum AM_PM : uint8_t {
      AM = HOUR_AM,
      PM = HOUR_PM
    };

    // misc time defines
    #define SECS_PER_MIN    60
    #define SECS_PER_HOUR   (SECS_PER_MIN * 60)
    #define SECS_PER_DAY    SECS_PER_HOUR * 24

    enum Source_Clock : uint8_t {
      LSI_CLOCK = ::LSI_CLOCK,
      LSE_CLOCK = ::LSE_CLOCK,
      HSE_CLOCK = ::HSE_CLOCK
    };

    // RTC backup status 
    // Status register is the first register in the 42 backup regs
    #define BACKUP_TIME_SET_FLAG      0x0001
    #define BACKUP_ALARM_SET_FLAG     0x0002
    #define BACKUP_CONFIGURED_FLAG    0x0004
    

    // misc status & error codes
    enum {
      RTC_OK,
      RTC_TIME_NOT_SET,
      RTC_ALARM_IS_SET,
      RTC_FAIL_LSERDY,
      RTC_FAIL_CONFIG_ENTER,
      RTC_FAIL_CONFIG_EXIT,
      RTC_TIMEOUT,
      RTC_INVALID_PARAM,
    };

    #define REG_TIMEOUT 2000

    // configure defines
    enum { 
      CONFIG_ENTER,
      CONFIG_EXIT,
    };

    static STM32LIBS_RTC &getInstance()
    {
      static STM32LIBS_RTC instance; // Guaranteed to be destroyed.
      // Instanciated on first use.
      return instance;
    }

    // initialization functions
    void begin(uint8_t initAction);  // initialize RTC
    void end(void);     // de-initialize RTC

    // interrupt functions
    void attachInterrupt(voidFuncPtr callback, void *data = nullptr);
    void detachInterrupt(void);

    // date/time functions
    void setDateTime(RTC_datetime_t *datetime);
    void getDateTime(RTC_datetime_t *_datetime = nullptr, uint8_t hour_format = RTC_HOUR_FORMAT_UNDEF);

    // conversion functions
    uint32_t getEpoch(void);
    void setEpoch(uint32_t ts);
    uint32_t dateTimeToEpoch(RTC_datetime_t *datetime);
    void epochToDateTime(RTC_datetime_t *datetime, uint32_t _epoch);

    // alarm functions
    uint8_t setAlarmDateTime(RTC_datetime_t *datetime);
    uint8_t setAlarmFromEpoch(uint32_t alarm_epoch);
    void disableAlarm(void);

    // char string functions
    char *getWeekdayName(uint8_t DOW);      
    char *getMonthName(uint8_t month);
    char *getDateTimeStr(uint8_t format);    

    // primitive Functions 
    void getPrediv(int8_t *predivA, int16_t *predivS);
    void setPrediv(int8_t predivA, int16_t predivS);
    Source_Clock getClockSource(void);
    void setClockSource(Source_Clock source);

    // user backup register functions - simulates EEPROM
    void eepromWrite(uint16_t data_array[], uint8_t indx, uint8_t len);
    void eepromRead(uint16_t data_array[], uint8_t indx, uint8_t len);

    // Kept for compatibility: use STM32LowPower library.
    void standbyMode();

    // misc debug
    volatile uint32_t debug1;
    volatile uint32_t debug2;
    volatile uint32_t debug3;
    volatile uint32_t debug4;

    bool isConfigured(void)
    {
      return ((_RTC_BackupRegs[0] & BACKUP_CONFIGURED_FLAG) > 0);
    }
    bool isAlarmEnabled(void)
    {
      return ((_RTC_BackupRegs[0] & BACKUP_ALARM_SET_FLAG) > 0);
    }
    bool isTimeSet(void)
    {
      return ((_RTC_BackupRegs[0] & BACKUP_TIME_SET_FLAG) > 0);
    }


    friend class STM32LowPower;

  private:
    STM32LIBS_RTC(void): _clockSource(LSI_CLOCK) {}
  
    Source_Clock _clockSource;
    uint8_t rtc_config(uint8_t _config);
    void configForLowPower(Source_Clock source);
    void _statusFlagChange(uint16_t sbit, bool fset);

    // private backup functions
    void setBackup(uint8_t indx, uint8_t len);
    void getBackup(uint8_t indx, uint8_t len);
    void clearBackup(void);

    uint8_t _RTC_Status;
    uint16_t _RTC_BackupRegs[11];

};

#endif // __STM32_RTC_H
