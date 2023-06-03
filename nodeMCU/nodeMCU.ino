#include <Wire.h>
#include <ESP8266WiFi.h>
#include <iostream>
#include <sstream>
#include <string>
#include <SoftwareSerial.h>

float temperature, humidity, rain, UV, air, windSpeed;
String apiKey = "PEM4TE3RXFQSF7DK";  //  Enter your Write API key from ThingSpeak
const char *ssid = "HoangThiHong";   // replace with your wifi ssid and wpa2 key
const char *pass = "59x112752";
const char *server = "api.thingspeak.com";
WiFiClient client;

void setup() {
  Serial.begin(9600); /* begin serial for debug */
  Wire.begin(D1, D2); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() {
  Serial.println("Starting devices...");
  char message[100] = "";
  Wire.beginTransmission(8); /* begin with device address 8 */
  //Wire.write("Hello Slave"); /* sends hello slave string */
  Wire.endTransmission(); /* stop transmitting */
  String dataMessage = "";
  Wire.requestFrom(8, 100); /* request & read data of size 13 from slave */
  while (Wire.available()) {
    char c = Wire.read();
    if (std::isalnum(c)) dataMessage += c;
  }
  if (!dataMessage.isEmpty()) {
    // Copy the contents of the string to the character array
    strncpy(message, dataMessage.c_str(), sizeof(message) - 1);
    message[sizeof(message) - 1] = '\0';  // Ensure null-termination

    std::stringstream ss(message);
    std::string token;

    std::getline(ss, token, 'a');
    temperature = std::stof(token) / 10;

    std::getline(ss, token, 'a');
    humidity = std::stof(token) / 10;

    std::getline(ss, token, 'a');
    rain = std::stof(token) / 10;

    std::getline(ss, token, 'a');
    UV = std::stof(token) / 10;

    std::getline(ss, token, 'a');
    air = std::stof(token) / 10;

    std::getline(ss, token, 'a');
    windSpeed = std::stof(token) / 10;

    if (client.connect(server, 80)) {
      String postStr = apiKey;
      postStr += "&field1=";
      postStr += String(temperature);
      postStr += "&field2=";
      postStr += String(humidity);
      postStr += "&field3=";
      postStr += String(rain);
      postStr += "&field4=";
      postStr += String(UV);
      postStr += "&field5=";
      postStr += String(air);
      postStr += "&field6=";
      postStr += String(windSpeed);
      postStr += "\r\n\r\n";
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);
      Serial.println("%. Send to Thingspeak.");
      Serial.print("postStr: ");
      Serial.print(postStr);
    }
    client.stop();
  }

  delay(10000);
}
