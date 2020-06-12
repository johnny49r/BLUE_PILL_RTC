#ifndef _BLUE_PILL_RTC_H
#define _BLUE_PILL_RTC_H

 /*****************************************************************************
 * BLUE_PILL_RTC
 * 
 * John Hoeppner @ Abbycus Consultants
 * June 11, 2020
 * 
 * A simple class to implement RTC funtionality on the STM32F103 (AKA 'BLUE PILL')
 * development boards. 
 * These functions work with a coin cell (CR2032) battery connected to the Vbat
 * input and keeps proper date/time during power down using the on-board 32.768 Khz
 * resonator.
 * Also included are functions for saving and restoring data from the STM32F10x backup 
 * registers. These registers are non-volatile if the Vbat pin is powered with an
 * external battery. This is the equivalent of having an EEPROM (OK a very small EEPROM).
 * 
 * Build environment:
 *    VSCode with PlatformIO IDE on Linux Mint
 * 
 *    platformio.ini:
 *       platform = ststm32
 *       board = bluepill_f103c8
 *       framework = arduino
 *       upload_port = /dev/ttys0
 *       monitor_speed = 115200
 *       upload_protocol = stlink
 *       ; *** Build flags enable USB serial monitor functionality.
 *       ; *** Can be eliminated to save memory if USB serial is not needed
 *       build_flags = 
 *	      -D PIO_FRAMEWORK_ARDUINO_ENABLE_CDC
 *	      -D USBCON
 *	      -D USBD_VID=0x0483
 *	      -D USB_MANUFACTURER="Unknown"
 *	      -D USB_PRODUCT="\"BLUEPILL_F103C8\""
 * 	   -D HAL_PCD_MODULE_ENABLED
 * 
 * 
 * NOTES: 
 * 1) The battery should be connected to Vbat through a shottky diode to prevent
 * reverse current flow into the battery when the board is powered normally.
 * 
 * 2) Application code should use caution when using PC13 as this I/O is powered
 * by the coin cell during power off. On the generic BLUE PILL board PC13 is connected to
 * the on-board LED which if left active at power down would drain the coin cell quickly. 
 * 
 * 3) Many (or perhaps most) cheap BLUE PILL boards use STM32F10x clones. 
 * These chips have a slightly different signature than 'real' STMicro chips and may 
 * fail during firmware uploading. Here is one workaround:
 * > Locate the file 'stm32f1x.cfg',
 *    Path example: '.platformio/packages/tool-openocd/scripts/target/stm32f1x.cfg'.
 * > Open this file with a text editor.
 * > Find and change "set _CPUTAPID 0x1ba01477" to "set _CPUTAPID 0x2ba01477"
 * > Save & exit.
 * 
 * ***************************************************************************/
#include <Arduino.h>

// misc time defines
#define SECS_PER_MIN    60
#define SECS_PER_HOUR   (SECS_PER_MIN * 60)
#define SECS_PER_DAY    SECS_PER_HOUR * 24


// STM32 Power control registers
#define PWR_REG_BASE    0x40007000UL
#define PWR_CR          (*(volatile uint32_t *)(PWR_REG_BASE))  // power control reg 
#define PWR_CSR         (*(volatile uint32_t *)(PWR_REG_BASE + 0x00000004UL))  // power control/status reg 

#define DBP             0x0100         // disable backup domain write protection

// Reset & clock control registers (32 bit regs)
#define RCC_REG_BASE    0x40021000UL
#define RCC_BDCR        (*(volatile uint32_t *)(RCC_REG_BASE + 0x00000020UL))  // RTC ctl reg low
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_REG_BASE + 0x0000001CUL)) 

#define LSEON           0x00000001UL   // external 32.768 KHz clk enable (LSE)
#define LSERDY          0x00000002UL   // status - external clk is stable
#define LSEBYP          0x00000004UL   // external 32KHz osc bypass bit
#define BKP_RESET       0x00010000UL
#define BDCR_INIT       0x00008101UL   // RTCEN, LSE clk, LSEON
#define PWREN           0x18000000UL

// RTC register memory mapped addresses
#define RTC_REG_BASE    0x40002800UL
#define RTC_CRH         (*(volatile uint32_t *)(RTC_REG_BASE))  // RTC ctl reg high
#define RTC_CRL         (*(volatile uint32_t *)(RTC_REG_BASE + 0x00000004UL))  // RTC ctl reg low
#define RTC_PRLH        (*(volatile uint32_t *)(RTC_REG_BASE + 0x00000008UL))  // RTC prescaler reg high
#define RTC_PRLL        (*(volatile uint32_t *)(RTC_REG_BASE + 0x0000000CUL))  // RTC prescaler reg low
#define RTC_DIVH        (*(volatile uint32_t *)(RTC_REG_BASE + 0x00000010UL))  // RTC prescaler divide reg high
#define RTC_DIVL        (*(volatile uint32_t *)(RTC_REG_BASE + 0x00000014UL))  // RTC prescaler divide reg low
#define RTC_CNTH        (*(volatile uint32_t *)(RTC_REG_BASE + 0x00000018UL))  // RTC count reg high
#define RTC_CNTL        (*(volatile uint32_t *)(RTC_REG_BASE + 0x0000001CUL))  // RTC count reg low
#define RTC_ALRH        (*(volatile uint32_t *)(RTC_REG_BASE + 0x00000020UL))  // RTC alarm reg high
#define RTC_ALRL        (*(volatile uint32_t *)(RTC_REG_BASE + 0x00000024UL))  // RTC alarm reg low

// RTC CRL control reg bits
#define RTOFF           0x0020UL
#define CNF             0x0010UL
#define RSF             0x0008UL
#define ALARMF          0x0002UL
#define ALARMF_MASK     0x0010UL
#define RTOFF_RSF       0x0028

// RTC CRH control reg bits
#define RTC_ALRIE_MASK  0x0005
#define RTC_ALRIE       0x0002

// backup registers
#define BKP_REG_BASE    0x40006C00UL
#define BKP_REGS        ((volatile uint32_t *)(BKP_REG_BASE))

#define REG_TIMEOUT 1000UL

// configure defines
enum { 
   CONFIG_ENTER,
   CONFIG_EXIT,
};

// hour format
enum {
   TIME_FORMAT_12,
   TIME_FORMAT_24,
};

// date / time structure
typedef struct 
{
   uint8_t hour_format;
   bool am_pm;              // true if PM
   uint8_t seconds;
   uint8_t minutes;
   uint8_t hours;
   uint32_t timestamp;         // time in secs since 1970

   uint8_t weekday;
   uint8_t day;
   uint8_t month;
   uint8_t year;
   
}_dateTimeStruct;

enum {
   RTC_OK,
   RTC_TIMEOUT,
};

// function pointer prototype
typedef void(*voidFuncPtr)(void *);

/******************************************************************************
**    BLUE_PILL_RTC class implements RTC functions for the STM32F10x MPU
**
******************************************************************************/
class BLUE_PILL_RTC {
   public:
      BLUE_PILL_RTC();     // constructor

      uint8_t begin(voidFuncPtr callback);
      void setDateTime(_dateTimeStruct *datetime);
      void getDateTime(_dateTimeStruct *datetime);
      void writeBackup(uint16_t words[], uint8_t indx, uint8_t len);
      void readBackup(uint16_t words[], uint8_t indx, uint8_t len);
      void clearBackup(void);
      void setAlarm(uint32_t alarmTime);
      void clearAlarm(void);
      bool checkAlarm(void);
      void timeExpand(_dateTimeStruct *datetime, uint32_t _timestamp);        // make time values from timestamp
      uint32_t timeCompress(_dateTimeStruct *datetime);  // convert time values into a timestamp
      uint32_t getTimeStamp(void);                       // get raw timestamp (secs from 1970)  
      void setTimeStamp(uint32_t ts);

   private:
      uint8_t rtc_config(uint8_t _config);
      voidFuncPtr _callback;


};

extern const uint8_t monthDays[];   
extern const char* weekdays[];

#endif
