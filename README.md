# Getting Started

## Installing

You may need to [update firmware](https://support.arduino.cc/hc/en-us/articles/360013896579-How-to-update-the-WiFi-Nina-and-WiFi101-firmware)

### Useful extensions for VSCode

- [Arduino](https://github.com/Microsoft/vscode-arduino.git)
- [REST Client](https://github.com/Huachao/vscode-restclient)

## Running

create `/arduino_secrets.h` with the following contents

```cpp
#define SECRET_SSID "your network name"
#define SECRET_PASS "your password"
```

Once the sketch is uploaded to the board,
Open the Serial Monitor, and you should eventually get the following messages:

```md
Please upgrade the firmware
Attempti��t�����сto Network <your network name>
SSID: <your network name>
IP Address: <ip>
```

 You can turn on/off the builtin led with these requests:

```md
`GET http://<ip>/L`
`GET http://<ip>/H`
```
