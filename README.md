# Pond Web Server
Arduino Web Server for my pond that allows the pump lights to be controlled and returns temperature readings.

# Prerequisites
This was written for an Arduion UNO R3 and requires the following extras:
- Ethernet WS5100 module
- 20x4 character LCD display connected to pins A4 & A5 that uses the I2C bus address 0x3F.
- 2 x Dallas Temperature sensors. Both connected to pin D2 with a 4k7ohm pull-up resistor to 5v VCC
- 8 relay module where relays 1-3 are connected to pins D3-D5
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
  "temperatures":[16,20],
  "relays":[
    {
      "name":"Pond pump",
      "state":0
    },
    {
      "name":"LED Lights",
      "state":0
    },
    {
      "name":"Rockery Lights",
      "state":0
    }
  ]
}
```

## POST http://192.168.1.177/api/v1/relays/[0-3]/[on|off]/
Allows the realys to be turned on or off.

# Other Features

1. The LCD display shows the server IP and the IP of the last connected client as well as the two temperatures. There is also a heartbeat
indicator in the top-right of the display to show it is not frozen.

2. The Serial port is used for debug output but has been kept to a minimum. Only the HTTP METHOD, URL and VERSION is displayed and when a client conenction is dropped.

```
21:13:26.682 -> GET /api/v1/status/ HTTP/1.1
21:13:26.751 -> Client disconnected
```

# Known issues
1. I have found an issue with the WS5100 modules that I am using where if you connect the device to a switch hub that is also
connected to another switching hub the LINK, 100M and FULLD lights on the module will flash and the device seems to be 
resetting constantly. This was with HP PRO CURVE network switches.
2. URL requests can anything from 130ms to 1 second in some cases to respond.
