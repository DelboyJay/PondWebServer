# Pond Web Server
Arduino Web Server for my pond that allows the pump lights to be controlled and returns temperature readings.

# Prerequisites
This was written for an Arduion UNO R3 and requires the following extras:
- Ethernet WS5100 module
- 2 x Dallas Temperature sensors. Both connected to pin D2 with a 4k7ohm pull-up resistor to 5v VCC
- 8 relay module where relays 1-3 are connected to pins D3-D5
- Optional watchdog LED on pin D7

# Web Pages
**NOTE:** All urls must end with a forward-slash or the URL is ignored.
Invalid URL requests are met with a ERR_EMPTY_RESPONSE, nothing is returned and the client connection is just closed.

## GET http://192.168.1.177/temperatures/
Returns only the temperatures on a web page.

# RESP APIs
**NOTE:** All urls must end with a forward-slash or the URL is ignored.

## GET http://192.168.1.177/api/v1/status/
Returns the status/values of all relays and temperatures as JSON.

**Example output** 

Please note that the following is returned on one line with no CRLF's but has been pretty 
printed here to make it easy to read.
```
{
  "temperatures": {
    "water": 129,
    "air": 129
  },
  "relays": {
    "pond_pump": 1,
    "waterfall_lights": 0,
    "rockery_lights": 0
  }, 
  "system_info":{ 
    "ticks_ms": 487054,
    "loops": 5051, 
    "loop_time_ms": 96, 
    "requests": {
      "valid": 10, 
      "invalid": 0, 
      "total": 10 
    } 
  } 
}
```

## POST http://192.168.1.177/api/v1/relays/[0-3]/[on|off]/
Allows the realys to be turned on or off.

# Other Features

1. The Rx LED (PIN D0) will flash every second if there is an Ethernet shield detection error

2. Pin D7 is used as a watchdog strobe. An LED can be linked to this to prove the system is running.

# Known issues
1. I have found an issue with the WS5100 modules that I am using where if you connect the device to a switch hub that is also
connected to another switching hub the LINK, 100M and FULLD lights on the module will flash and the device seems to be 
resetting constantly. This was with HP PRO CURVE network switches.
2. URL requests can anything from 130ms to 1 second in some cases to respond.
