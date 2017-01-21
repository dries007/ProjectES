Project Embedded
================

Via ini file configurable temperature logger, with RTC & SD card.
Also has a UDP based server with a basic command set.
Can also send out UDP packets (the push option) when a new measurement is taken.

Style guide followed as best as I could, but:
- Added type prefix 'b' for `bool`, because its a std-type, and the compiler decides which one.
- Added type prefix 'str' for `char*` or `const char*`

All files in /src/ are my own, with exception of ini.c and ini.h.
The other files are all under various different licences, see there headers.

Libs used: inih (a ini file parser, added in /src/) & LWIP.

Where applicable: Copyright (c) 2017 Dries007 

Config
------

A default is made if there is no `logger.ini` file on de SD.

UDP connections
---------------

`ncat <ip> 41752 -u -v` (41752 is a [random](https://xkcd.com/221/) port, settable via config.)

Available commands:
- `last`        Get last log entry.
- `log`         Make new log entry now. Won't disturb scheduled next log.
- `config`      Return config entries: ID, Interval and Push settings.
- `time`        Return current RTC time.

Its also possible to have the log entry pushed to a UDP port on a server or broadcast (255.255.255.255).

Simple debug receive server in python:
```python
import socket
port = 41752
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(("", port))
print("waiting on port:", port)
while 1:
    data, addr = s.recvfrom(1024)
    print(data)
```

Extras
------

Splint output is included.

My entire Embedded STM with GCC, OpenOCD & RemoteGDB on CLion 'tutorial' can be found here:
 https://github.com/dries007/Embedded_CLion

(See also CMake files.)

MISRA C
-------

- [x] 5.2
- [x] 6.1
- [x] 6.3 (except where it causes warnings because of stdlib or inih lib)
- [x] 7.1
- [x] 8.5
- [x] 8.7
- [x] 8.11 
- [x] 9.1
- [x] 13.3 
- [x] 13.4
- [x] 13.6 
- [x] 14.1
- [x] 14.4
- [x] 14.8
- [x] 14.9
- [x] 14.10
- [x] 15.2
- [x] 15.3
- [x] 16.2
- [x] 16.5
- [x] 16.7
- [x] 16.8
- [x] 17.5
- [x] 21.1 (CLion has code static analysis & inspection)


ToDo
----

- [ ] Start working earlier.
- [ ] Remove stupid 'humor' written at 3am 12h before the deadline.
- [x] Write information.
- [ ] Hal_Tick Overflow management (now 24 days is max operation time).
- [ ] Add onewire temp sensor support.
- [ ] RTC time set menu or UDP command.
- [x] Style stuff
    - [x] Un-indent the variable declaration.
    - [x] Replace magic numbers with magic defines.
    - [x] MISRA C


Changes made in BSP:
--------------------

### fonts.h

`bsp/Font/fonts.h`

Fixed the fonts.h inclusion of `font*.c` files.

Changes made in CubeMX generated source:
----------------------------------------

Items marked **OUTSIDE USER CODE** must be redone after regenerating the sources with CubeMX.

### BYTE_ORDER macro **OUTSIDE USER CODE**

`CubeMX/Middlewares/Third_Party/LwIP/system/arch/cpu.h`

Comment out BYTE_ORDER. Its already defined in gcc's includes.

### BSP_PlatformIsDetected() function

`CubeMX/Src/fatfs_platform.c`
    
This function is just wrong. There is a correct version in BSP, but its easier and less intrusive to just fix tis one.
    
#### Before:
```c
uint8_t	BSP_PlatformIsDetected(void) {
  uint8_t status = (uint8_t)0x01;
  /* Check SD card detect pin */
  if (HAL_GPIO_ReadPin(SD_PORT,SD_PIN) == GPIO_PIN_RESET) {
    status = (uint8_t)0x00;
  }
  /* USER CODE BEGIN 1 */
  /* user code can be inserted here */
  /* USER CODE END 1 */
  return status;
}
```

#### After:
```c
uint8_t	BSP_PlatformIsDetected(void) {
  uint8_t status = (uint8_t)0x01;
  /* Check SD card detect pin */
  if (HAL_GPIO_ReadPin(SD_PORT,SD_PIN) == GPIO_PIN_RESET) {
    status = (uint8_t)0x00;
  }
  /* USER CODE BEGIN 1 */
  /* WHY ?? Its correct in stm32746g_discovery_sd.c ... */
  status = ( uint8_t ) !status;
  /* USER CODE END 1 */
  return status;
}
```

### get_fattime() function

`CubeMX/Src/fatfs.c`

Redirect to ulAppGetFATTimestamp()

