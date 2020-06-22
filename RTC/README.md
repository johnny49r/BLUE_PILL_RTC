# STM32 REAL TIME CLOCK LIBRARY
STM32F10x (aka BLUE PILL) RTC Library - VSCode / Arduino / Battery Backup

John Hoeppner@Abbycus 2020

### DESCRIPTION:

A simple and minimalist library to implement real time clock functions on various STM32F10x boards (Blue Pill, Black Pill, etc.). This was created because other libraries didn't work with battery backup and would lose the date on reset/power down.

The STM32F10x MPU sports an embedded RTC with a ***Vbat*** pin that can power the RTC via a coin cell when main power is removed. These boards also contain a low power 32.768 KHz resonator as the precision clock source. 
The RTC is centered around a 32 bit (second) counter which maintains a running 'epoch' counter of the number of seconds elapsed since 1970.
The counter increments once per second and time & date are derived from this singularity.
The RTC has an alarm function which simply reports when the currently running timestamp exceeds the value stored in an alarm register.

Additionally the STM32F10x has 9 16-bit backup registers that can be used to store user configuration / calibration data. These backup registers are non volatile when the Vbat pin is powered and are functionally equivalent to an EEPROM.

I hope this is useful and would appreciate your feedback.

### NOTES:

- Check the STM32LIBS_RTC.h header file for more details about parameter and return data types and possible values.

- The STM32F1xx datasheet shows support for 42 backup registers but not all devices support more than 10 (ex: cheap Blue Pill knockoff's). For this reason the library will only support 9 user resisters. The first register is used for keeping the state of the RTC during power down (with Vbat powered).

- The external Vbat battery (CR2032 or ?) should be connected to the Vbat pin through a shottky diode to prevent current flow into the battery when the board is powered normally.

- Use caution when utilizing GPIO PC13: This I/O is active when Vbat is connected to an external battery. On the Blue Pill boards PC13 is connected to the on-board LED and if it is active (HIGH) when main power drops, the external battery could drain quickly trying to power the LED.

- Many (or perhaps most) cheap BLUE PILL boards use Chinese STM32F10x clones. 
These chips have a slightly different signature than 'original' STMicro chips and may fail during firmware uploading. Here is one workaround:
```
> Locate the file 'stm32f1x.cfg',
    Linux path example: '.platformio/packages/tool-openocd/scripts/target/stm32f1x.cfg'.
> Open this file with a text editor.
> Find and change "set _CPUTAPID 0x1ba01477" to "set _CPUTAPID 0x2ba01477"
> Save & exit.
```

### SAMPLE BUILD ENVIRONMENT

Visual Studio (VSCode) with PlatformIO IDE using the arduino framework.
```
platformio.ini:
       platform = ststm32
       board = bluepill_f103c8
       framework = arduino
       upload_port = /dev/ttys0
       monitor_speed = 115200
       upload_protocol = stlink
       ; *** Build flags enable USB serial monitor functionality.
       ; *** Can be eliminated to save memory if USB serial is not needed
       build_flags = 
	      -D PIO_FRAMEWORK_ARDUINO_ENABLE_CDC
	      -D USBCON
	      -D USBD_VID=0x0483
	      -D USB_MANUFACTURER="Unknown"
	      -D USB_PRODUCT="\"BLUEPILL_F103C8\""
        -D HAL_PCD_MODULE_ENABLED
```

### INSTALLATION:

Arduino: just add the library folder to the Arduino/library directory.

VSCode: Place the library folder in the .platformio/lib folder. Close and restart VSCode.


### USAGE
**Add in your source code:**
```
#include "STM32LIBS_RTC.h"       // include this header
STM32LIBS_RTC& rtc = STM32LIBS_RTC::getInstance();   // instanciate the lib object
RTC_datetime_t datetime;	 // create structures for time & date
RTC_datetime_t alarm_datetime;   // create structure for alarm if needed
```

#### DateTime Structure
Create a _dateTimeStruct_ to keep all time & date items.
```
typedef struct 
{
uint8_t hour_format;       // 12 hour = 0, 24 hour = 1
bool am_pm;                // true if PM
uint8_t seconds;
uint8_t minutes;
uint8_t hours;
uint32_t epoch;            // time in secs since 1970
uint8_t weekday;
uint8_t day;
uint8_t month;
uint8_t year;
} RTC_datetime_t;
```

#### Function Descriptions 

##### begin(initActions) 
```
RTC initialization. Usually called in setup(): rtc.begin();
Arg: initActions:
	a) INIT_NONE - no special action
	b) INIT_TIME_RESET - reset time to Jan 1, 1970
	c) INIT_ALARM_RESET - clear alarm
	d) INIT_RTC_RESET - *** WARNING! *** this resets the entire RTC domain including backup regs
Ret: Nothing
```

##### end() 
```
Stop RTC and disable alarm.
Arg: None
Ret: Nothing
```

##### attachInterrupt(voidFuncPtr callback, void *data)
```
Attach a callback for alarm event.
Arg: callback - pointer to your alarm event handler.
Arg: data - pointer to any data that might be passed to the alarm event handler.
Ret: Nothing
```

##### deattachInterrupt()
```
Removes the callback for alarm event.
Arg: None 
Ret: Nothing
```

##### setDateTime(datetime)
```
Sets the RTC date & time.
Arg: datetime is a RTC_datetime_t structure containing date & time elements.
Ret: nothing.
```

##### getDateTime(datetime, hour_format)
```
Gets the current the date & time from the RTC clock.
Arg: datetime is a RTC_datetime_t structure which will be updated with date & time elements.
Arg: <OPTIONAL> hour_format sets 12 or 24 hour time format. If not included the hour format is derived from the datetime structure.
Ret: nothing.
```

##### setEpoch(epoch)
```
Sets the RTC epoch time (32 bit number of seconds since 1970).
Ret: nothing.
```

##### getEpoch()
```
Gets the RTC epoch time (32 bit number of seconds since 1970).
Ret: 32 bit epoch value..
```

##### dateTimeToEpoch(datetime)
```
Converts the date & time elements in the datetime structure to a 32 bit epoch.
Arg: datetime - pointer to RTC_datetime_t structure containing date time elements.
Ret: 32 bit epoch. Also updates epoch field in datetime.
```

##### epochToDateTime(datetime, epoch)
```
Converts the epoch 32 bit value to date & time elements and stores them in datetime.
Arg: datetime - pointer to RTC_datetime_t structure.
Arg: epoch - 32 bit epoch value.
Ret: nothing.
```

##### setAlarmDateTime(alarmtime)
```
Sets an ABSOLUTE alarm using the values in alarmtime.
Arg: alarmtime - pointer to RTC_datetime_t structure.
Ret: Error code if alarm date/time is invalid (<= current date/time). 0 otherwise.
```

##### setAlarmFromEpoch(alarm_epoch)
```
Sets a RELATIVE alarm using an epoch value.
Arg: alarm_epoch - epoch value of new alarm value.
Ret: Error code if alarm epoch is invalid (<= current date/time). 0 otherwise.
```

##### eepromWrite(data_array[], indx, len)
```
Writes user data to the RTC backup registers. These registers are non-volatile if Vbat is powered with an external coin cell or equivalent. 
Arg: data_array[] - an array of 16 bit data words to write, maximum of 9.
Arg: indx - Starting register number (0 - 8).
Arg: len - number of registers to write. 
Ret: Nothing
```

##### eepromRead(data_array[], indx, len)
```
Reads user data from the RTC backup registers. These registers are non-volatile if Vbat is powered with an external coin cell or equivalent. 
Arg: data_array[] - an array of 16 bit data words, maximum of 9.
Arg: indx - Starting register number (0 - 8).
Arg: len - number of registers to read. 
Ret: Nothing
```

##### getWeekdayName(DOW)      
```
Returns a pointer to a char string containing the name of the day of the week. 
Arg: DOW - number of the weekday (1 - 7). Sunday = 1, Saturday = 7.
Ret: char * to string.
```

##### getMonthName(month)
```
Returns a pointer to a char string containing the name of the month. 
Arg: month - number of the month (1 - 12). 
Ret: char * to string.
```

##### getDateTimeStr(datetime, format) ***NOT IMPLEMENTED YET***
```
Returns a pointer to a char string containing the formatted date & time string. 
Arg: datetime - pointer to RTC_datetime_t structure.
Arg: format - format type: YYMMDD_HHMMSS, etc.
Ret: char * to string.
```

##### isConfigured()
```
Checks if the RTC has been configured.
Ret: true if configured via call to begin().
```

##### isTimeSet()
```
Checks if date & time have been set. 
Ret: true if date/time has been set. 
Note: This can be useful after a call to begin() to know if date/time was set prior to the last reset.
```

##### isAlarmEnabled()
```
Checks if the alarm has been set. 
Ret: true if alarm has been set. 
Note: This can be useful after a call to begin() to know if the alarm was set prior to the last reset.
```
