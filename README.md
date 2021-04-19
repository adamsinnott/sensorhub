# sensorhub

The goal of this repo is to create a cluster of sensors around the house that are linked together via a Mobile Phone App and Webpage, communicating via AWS IoT Core.

The current implementation is just a single device. This device is an [Adafruit HUZZAH32 - ESP32 Feather Board](https://www.adafruit.com/product/3405) with an [Adafruit OLED FeatherWing](https://learn.adafruit.com/adafruit-oled-featherwing) sitting on top. Connected to that is an [ADT7410 High Accuracy I2C Temperature Sensor Breakout Board](https://www.adafruit.com/product/4089).

This is a great combination of devices from Adafruit centered around the Adafruit Huzzah32. This is Adafruit's take on the Espressif ESP32 which is 240 MHz dual core microcontroller with 512KB SRAM, 4MByte flash, WiFi, Bluetooth that Adafruit has incorporated in to their "FeatherWing" ecosystem and added in a battery charger and many other features.

This weekend project has essentially hacked together the following 2 HOWTOs.
https://learn.adafruit.com/iot-temperature-logger-with-arduino-and-adafruit-io/arduino-code
https://aws.amazon.com/blogs/compute/building-an-aws-iot-core-device-using-aws-serverless-and-an-esp32/

The Adafruit tutorial uses their own proprietry IoT cloud solution, and uses an older ESP8266 but essentially very similar to the ESP32 based board.
The AWS tutorial is geared towards SPARKFUN other based boards but the main thing is that they are very similar.

## How It Works

So the Huzzah32, loads a bitmap logo, connects to WiFi, tests the connection to AWS, gets the temperature from the temperature sensor, and then posts that data to an MQTT queue using AWS IoT Core. This message is then picked up by a Lambda rule and written to a DynamoDB table.

Eventually this will be read by a Mobile Phone App and Webpage.
