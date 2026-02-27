# SensorHub

SensorHub is an ESP32-based home telemetry prototype. The current build runs on an Adafruit HUZZAH32, reads temperature from an ADT7410 sensor, renders status to an OLED FeatherWing, and publishes telemetry over MQTT using TLS.

## Current Status

### Implemented

- Single hardware node based on [Adafruit HUZZAH32 - ESP32 Feather](https://www.adafruit.com/product/3405).
- OLED output on [Adafruit OLED FeatherWing](https://learn.adafruit.com/adafruit-oled-featherwing).
- Temperature capture from [Adafruit ADT7410 High Accuracy I2C Temperature Sensor](https://www.adafruit.com/product/4089).
- Wi-Fi connection and TLS MQTT connection using device certificates.
- MQTT publish to `drsensors/pub` with JSON payload containing `time` and `temperature`.
- MQTT subscribe to `drsensors/sub` (no active message handler yet).

### Planned

- Multi-node sensor cluster.
- Cloud ingestion and storage path documented in this repo.
- Mobile and web dashboards consuming telemetry.
- Message acknowledgements, retries, and health monitoring.

### Service Lifecycle Caveat

This project currently assumes AWS IoT will be closed in **May 2026**. Migration work to a replacement service must be factored into roadmap and architecture decisions now, not deferred.

## Architecture

Implemented today:

`ADT7410 -> ESP32 (SensorHub firmware) -> TLS MQTT publish -> AWS IoT topic`

Planned expansion:

`AWS IoT topic -> ingestion/storage -> mobile app + web dashboard`

## Hardware

- Adafruit HUZZAH32 (ESP32 Feather)
- Adafruit OLED FeatherWing (128x32)
- Adafruit ADT7410 I2C temperature breakout

Basic wiring:

- `ADT7410 VCC -> 3V`
- `ADT7410 GND -> GND`
- `ADT7410 SDA -> SDA`
- `ADT7410 SCL -> SCL`

## Software Stack

- Arduino IDE / Arduino CLI
- ESP32 board support package
- Libraries used by firmware:
  - `Adafruit_GFX`
  - `Adafruit_SSD1306`
  - `WiFiClientSecure`
  - `MQTTClient`
  - `ArduinoJson`
  - `Adafruit_ADT7410`

## Repository Structure

- `main/main.ino`: Firmware entrypoint and runtime loop.
- `main/secrets.h`: Wi-Fi credentials, AWS endpoint, device certificate material.
- `main/vgaSplash.h`: Splash bitmap used on OLED during startup.
- `assets/`: Source bitmap and generated C arrays for display assets.

## Setup

1. Install ESP32 board support in Arduino IDE.
2. Install required libraries listed above.
3. Update `main/secrets.h` with:
   - `THINGNAME`
   - `WIFI_SSID`
   - `WIFI_PASSWORD`
   - `AWS_IOT_ENDPOINT`
   - `AWS_CERT_CA`
   - `AWS_CERT_CRT`
   - `AWS_CERT_PRIVATE`
4. Open `main/main.ino`.
5. Select the HUZZAH32 target board and serial port.
6. Build and flash.

## Runtime Behavior

On boot the device:

1. Draws splash screen on OLED.
2. Connects to Wi-Fi and displays local IP.
3. Connects to AWS MQTT endpoint over TLS.
4. Initializes ADT7410 sensor.

In the main loop:

1. Samples and displays temperature every second for ~60 seconds.
2. Reconnects MQTT.
3. Publishes one JSON message to `drsensors/pub`.

## MQTT Contract

- Publish topic: `drsensors/pub`
- Subscribe topic: `drsensors/sub`
- Payload example:

```json
{
  "time": 1234567,
  "temperature": 22.75
}
```

## Security Notes

- `main/secrets.h` currently stores certificate and key material in source form.
- Treat credentials as sensitive and never commit real values.
- Rotate certificates/keys if shared accidentally.

## Known Limitations

- Reconnect loops can block indefinitely when Wi-Fi or MQTT is unavailable.
- MQTT subscription is configured but not processed by a handler.
- Only one payload is published per ~60-second cycle.
- No local buffering when cloud connectivity is down.
- Cloud integration beyond MQTT publish is not implemented in this repository.

## Roadmap

1. Add robust reconnect/backoff and non-blocking loop behavior.
2. Add inbound command handling on `drsensors/sub`.
3. Add structured telemetry schema versioning and error reporting.
4. Document and implement the post-MQTT ingestion and dashboard stack.
5. Define and execute AWS IoT migration path before May 2026 cutoff.

## References

- Adafruit temperature logger tutorial:
  - https://learn.adafruit.com/iot-temperature-logger-with-arduino-and-adafruit-io/arduino-code
- AWS IoT ESP32 walkthrough used as a starting point:
  - https://aws.amazon.com/blogs/compute/building-an-aws-iot-core-device-using-aws-serverless-and-an-esp32/
