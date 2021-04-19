#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "vgaSplash.h" 
#include "secrets.h"
#include "Adafruit_ADT7410.h"

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
#define LOGO_HEIGHT   32
#define LOGO_WIDTH    128

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "drsensors/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "drsensors/sub"

// OLED FeatherWing buttons map to different pins depending on board:
#if defined(ESP8266)
  #define BUTTON_A  0
  #define BUTTON_B 16
  #define BUTTON_C  2
#elif defined(ESP32)
  #define BUTTON_A 15
  #define BUTTON_B 32
  #define BUTTON_C 14
#elif defined(ARDUINO_STM32_FEATHER)
  #define BUTTON_A PA15
  #define BUTTON_B PC7
  #define BUTTON_C PC5
#elif defined(TEENSYDUINO)
  #define BUTTON_A  4
  #define BUTTON_B  3
  #define BUTTON_C  8
#elif defined(ARDUINO_FEATHER52832)
  #define BUTTON_A 31
  #define BUTTON_B 30
  #define BUTTON_C 27
#else // 32u4, M0, M4, nrf52840 and 328p
  #define BUTTON_A  9
  #define BUTTON_B  6
  #define BUTTON_C  5
#endif

int status = WL_IDLE_STATUS;     // the Wifi radio's status
IPAddress ip;                    // the IP address of your shield

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

// Create the ADT7410 temperature sensor object
Adafruit_ADT7410 tempsensor = Adafruit_ADT7410();

// Read and print out the temperature, then convert to *F
float c = 0;
  
void setup() {
  Serial.begin(115200);
  // wait for serial monitor to open
  while (!Serial)
    ;

  // Setup the Display
  setupDisplay();
  // setup buttons
  connectToWifi();
  connectAWS();
  setUpTemperatureSensor();
}

void setupDisplay() {
  // I dont know what this section does
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  // Clear the buffer.
  display.clearDisplay();
  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);
  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(1000);
  drawSplashScreen();
  
}

void drawSplashScreen(void) {
  // Clear the buffer.
  display.clearDisplay();
  display.drawBitmap(0,0,veryGoodAppsManualFontHorizontal,128,32,WHITE);
  display.display();
  delay(1000);
}

void connectToWifi() {
  // Clear the buffer.
  display.clearDisplay();
  // text display tests
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print("Connecting to SSID:\n");
  display.print(WIFI_SSID);
  display.print("\n");
  display.display(); // actually display all of the above
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  display.print(".");
  display.display();
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    display.print(".");
    display.display();
  }
  
//  print the local IP address
  ip = WiFi.localIP();
  display.print("\n");
  display.print(ip);
  display.display();
  delay(2000);
}

void resetDisplay() {
  // Clear the buffer.
  display.clearDisplay();
  // text display tests
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.display();
}

void connectAWS()
{
  resetDisplay();
  display.println("AWS");
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  //  client.onMessage(messageHandler);
  
  display.print("Connecting to ");
  display.print(THINGNAME);
  display.display();
  while (!client.connect(THINGNAME)) {
    display.print(".");
    display.display();
    delay(500);
  }

  if(!client.connected()){
    display.println("\nAWS IoT Timeout!");
    display.display();
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  display.println("\nConnected!");
  display.display();
  delay(2000);
}

void connectAWSQuiet() {// Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  //  client.onMessage(messageHandler);
  
  Serial.print("Connecting to ");
  Serial.print(THINGNAME);
  
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(500);
  }

  if(!client.connected()){
    display.println("\nAWS IoT Timeout!");
    display.display();
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("\nConnected!");
}

void publishMessage()
{
//  resetDisplay();
  Serial.println("Publishing a message");
//  display.display();
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  Serial.println(c);
  doc["temperature"] = c;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void setUpTemperatureSensor() {
  resetDisplay();
  // Make sure the sensor is found, you can also pass in a different i2c
  // address with tempsensor.begin(0x49) for example
  if (!tempsensor.begin())
  {
    display.println("Couldn't find ADT7410!");
    display.display();
    while (1)
      ;
  }
  // sensor takes 250 ms to get first readings
  // give it 500 just incase
  delay(500);
}

void getTemperature() {
//  resetDisplay();
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  
  // Read and print out the temperature, then convert to *F
  c = tempsensor.readTempC();
  
  display.println("Temp: ");
  display.print(c);
  display.println("C");
  display.display();
}

void loop() {
//  if(!digitalRead(BUTTON_A)) display.print("A");
//  if(!digitalRead(BUTTON_B)) display.print("B");
//  if(!digitalRead(BUTTON_C)) display.print("C");
//  delay(10);
//  yield();
//  display.display();
//  publishMessage();
  int i = 0;
  for (i = 0; i <= 60; i++) {
    Serial.print(i);
    Serial.println(": Getting Temperature");
    getTemperature();
    delay(1000);
  }
  connectAWSQuiet();
  publishMessage();
  client.loop();
}
