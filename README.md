![ojito](docs/ojito.png)

# ojito

### [Web Demo](https://jimothyjohn.github.io/ojito/)

Ojito (oh-hee-toh), a small eye to observe the world.

## Hardware

Ojito utilizes the [Seeed Studio XIAO ESP32S3 Sense](https://www.seeedstudio.com/XIAO-ESP32S3-Sense-p-5639.html). Other ESP32 products and cameras could also be used, but memory-restricted (no PSRAM) platforms will probably not work.

## Setup

- Create a [Replicate API token](https://replicate.com/account/api-tokens)

- Add your token and Wifi credentials to [include/secrets.h](include/secrets.h) 

```cpp
// If you do not do this correctly you will get one of these errors during compilation:
// fatal error: secrets.h: No such file or directory
// error: 'REPLICATE_TOKEN' was not declared in this scope
// error: 'SSID' was not declared in this scope
// error: 'PASSWORD' was not declared in this scope

#define REPLICATE_TOKEN "yourtoken"
#define SSID "yourssid"
#define PASSWORD "yourpass"
```

- Update the host and model version ([example](https://replicate.com/tofighi/yolox/versions)) in [src/main.cpp](https://github.com/JimothyJohn/ojito/blob/531752320c08e60082aeb91b78dd55037f5ecef9/src/main.cpp#L8)

```cpp
// If you do not do this correctly you will get one of these errors one the serial monitor:
// Error on sending POST: -1
// Prediction: {"error":"HTTP Client error!","status":"Error"}

// Do not add a / to the end of the URL path!
// Cloud endpoint ($$$)
#define HOST "https://api.replicate.com/v1/predictions"
// Local endpoint
// #define HOST "http://10.0.0.31/predictions"
// Find your version at https://replicate.com/<user>/<model>/versions
#define MODEL_VERSION "0239647164ce56b643213501d63f6116a76a3c620e4194814968e8ba1aa64cb7"
```

- Update the serial port in [platformio.ini](platformio.ini) to match your OS/device:

```bash
# If you do not do this correctly you will get an error like this:
# A fatal error occurred: Could not open /dev/tty.usbmodem83301, the port doesn't exist
upload_port = /dev/ttyACM0
monitor_port = /dev/ttyACM0
# Windows ports
# upload_port = COM1
# monitor_port = COM1 
# Mac ports (won't be identical)
# upload_port = /dev/tty.usbmodem83301 
# monitor_port = /dev/tty.usbmodem83301
```

- Run the convenience script that installs dependencies, uploads the firmware, then monitors the serial output:

```bash
bash utils/Quickstart.sh
# If all goes well you will see something like
# Prediction: [["n03196217","digital_clock",0.136194795],["n01930112","nematode",0.078128502],["n02259212","leafhopper",0.039054286]]
```

### Testing

***It is highly suggested that you test locally to save money during development!***

- Update the host (version not required locally) in [include/config.h](include/config.h) 

```cpp
// Do not add a / to the end of the URL path!
// #define HOST "https://api.replicate.com/v1/predictions"
#define HOST "http://<your-ip-address-here>/predictions"
```

- Run a test server locally with:

```bash
bash utils/TestServer.sh
```

### Further Reading

- [ESP32-S3 Sense](https://github.com/limengdu/SeeedStudio-XIAO-ESP32S3-Sense-camera/tree/main)
- [Cog](https://github.com/replicate/cog)
- [Base64](https://github.com/adamvr/arduino-base64)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [PlatformIO](https://github.com/platformio/platformio-core)

### Bounties

- [ ] $50 - [Separate "input" schema from Core library](https://github.com/replicate/cog/issues/1259)

- [ ] $5 - [Conditionally save images](https://github.com/limengdu/SeeedStudio-XIAO-ESP32S3-Sense-camera/blob/78ad1b1d80888eb9d571d5725a91b527469fac79/take_photos/take_photos.ino#L33)

- [ ] $200 - Web setup interface (fully-tested)

- [ ] Find ideal resolution/performance

- [ ] $200 - Add error handling for logner inference times

- [ ] $250 - Create test framework
