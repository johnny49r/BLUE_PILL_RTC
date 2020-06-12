# BLUE_PILL_RTC
STM32F10x (aka BLUE PILL) RTC Library - VSCode / Arduino / Battery Backup

John Hoeppner@Abbycus 2020

### DESCRIPTION:

A simple and minimalist library to implement real time clock functions on various STM32F10x boards (Blue Pill, Black Pill, etc.). This was created because other libraries didn't work with battery backup and would lose the date on reset/power down.

The STM32F10x MPU sports an embedded RTC with a ***Vbat*** pin that can power the RTC via a coin cell when main power is removed. These boards also contain a low power 32.768 KHz resonator as the precision clock source. 
The RTC is centered around a 32 bit (second) counter which keeps a timestamp of the number of seconds elapsed since 1970.
The counter increments once per second and time & date are derived from this singularity.
The RTC has an alarm function which simply reports when the currently running timestamp exceeds the value stored in an alarm register.

Additionally the STM32F10x has 42 16-bit backup registers that can be used to store user configuration / calibration data. These backup registers are non volatile when the Vbat pin is powered and are functionally equivalent to an EEPROM.

I hope this is useful and would appreciate your feedback.

### NOTES:

- The external battery (CR2032 or ?) should be connected to the Vbat pin through a shottky diode to prevent current flow into the battery when the board is powered normally.

- Use caution when utilizing GPIO PC13: This I/O is active when Vbat is connected to an external battery. On the Blue Pill boards PC13 is connected to the on-board LED and if it is active when main power drops, the external battery could drain quickly.

- Many (or perhaps most) cheap BLUE PILL boards use Chinese STM32F10x clones. 
These chips have a slightly different signature than 'real' STMicro chips and may fail during firmware uploading. Here is one workaround:
> Locate the file 'stm32f1x.cfg',
    Linux path example: '.platformio/packages/tool-openocd/scripts/target/stm32f1x.cfg'.
> Open this file with a text editor.
> Find and change "set _CPUTAPID 0x1ba01477" to "set _CPUTAPID 0x2ba01477"
> Save & exit.


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

The library is intended to be included with the project files. Simply add the *blue_pill_rtc.h* and *blue_pill_rtc.cpp* files into your project folder and #include blue_pill_rtc.h in your source.


### USAGE
**Add in your source code:**
```
#include "blue_pill_rtc.h"
BLUE_PILL_RTC rtc;        
_dateTimeStruct dateTime;    
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
uint32_t timestamp;         // time in secs since 1970
uint8_t weekday;
uint8_t day;
uint8_t month;
uint8_t year;
}_dateTimeStruct;
```

#### Function Descriptions 

##### begin(callback) 
```
One time RTC initialization. Usually called in setup(): rtc.begin();
Arg: callback - pointer to alarm handler function (not implemented)
Ret: error code. 0 = no error.
```

##### setDateTime(_dateTimeStruct_)
```
Set the clock to the date & time contained in the _dateTimeStruct.
Ret: nothing.
```

##### getDateTime(_dateTimeStruct_)
```
Loads the date & time into the _dateTimeStruct.
Ret: nothing.
```

##### setAlarm(timestamp)
```
Sets the alarm with the value in timestamp.
Ret: nothing.
```

##### checkAlarm()
```
Checks if an alarm event has occured.
Ret: true if alarm event.
```

##### clearAlarm()
```
Clears alarm. This is usually called after checkAlarm() to clear the alarm.
Ret: nothing.
```

##### getTimeStamp()
```
Get the current RTC count 'timestamp'.
Ret: 32 bit timestamp - number of seconds since 1970.
```

##### setTimeStamp(timestamp)
```
Sets the current RTC count 'timestamp'.
Ret: nothing.
```

##### timeCompress(_dateTimeStruct_)
```
Converts the date & time values in the _dateTimeStruct to a singular timestamp.
Ret: 32 bit timestamp. Also updates timestamp in the _dateTimeStruct.
```

##### timeExpand(_dateTimeStruct, timestamp_)
```
Converts the singular timestamp to date & time values and stores them in the _dateTimeStruct.
Ret: nothing.
```

##### writeBackup(words[], start, length)
```
Writes 'length' 16-bit values from the words[] array from offset 'start'.
Note that start + length shouldn't be greater than 42.
Ret: nothing.
```

##### readBackup(words[], start, length)
```
Reads 'length' 16-bit values from offset 'start' and stores into the words[] array.
Note that start + length shouldn't be greater than 42.
Ret: nothing.
```

##### clearBackup()
```
Clears all backup registers to 0x0.
Ret: nothing.
```

