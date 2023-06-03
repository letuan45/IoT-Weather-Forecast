#include "DHT.h"
#include <Arduino.h>
#include <ML8511.h>
#include <Wire.h>

#define ANALOGPIN A0  //Sensor UV được đọc từ chân A0
#define ENABLEPIN A1  //Chân mức High là A1

const int DHTPIN = 2;
const int ObjPin = 3;
const int DHTTYPE = DHT11;
ML8511 light(ANALOGPIN, ENABLEPIN);
DHT dht(DHTPIN, DHTTYPE);

float time1, time2, duration = 0;
double time = 0.000000;
int lastState = 1;                 // Khởi tạo biến lưu trạng thái trước đó của cảm biến
const int obstaclePresent = 0;     // Khai báo giá trị của cảm biến khi có vật cản
bool obstacleIsNotPresent = true;  //
const float S = 0.2198;         //Chu vi trục quay con quay (m)
double windSpeed = 0.0;
double maxWindSpeed = 0.0;
int airQuality, rainAmountAnalog;
float UV, t, h;

void setup() {
  Serial.begin(115200);
  dht.begin();

  Wire.begin(8);                /* join i2c bus with address 8 */
  Wire.onReceive(receiveEvent); /* register receive event */
  Wire.onRequest(requestEvent); /* register request event */
}

void loop() {
  windSpeed = 0;
  airQuality = analogRead(A2);
  //Đọc lượng mưa từ chân Analog A4
  rainAmountAnalog = analogRead(A3);
  //Điện áp của sensor không ổn định nên cần trừ đi 80
  rainAmountAnalog = rainAmountAnalog - 80;
  if (rainAmountAnalog < 0) rainAmountAnalog = 0;

  int currentState = digitalRead(ObjPin);

  //Bật tắt LED sensor khi đọc
  light.enable();
  UV = light.getUV();

  h = dht.readHumidity();
  t = dht.readTemperature();

  if (isnan(t) || isnan(h) || isnan(rainAmountAnalog)) {
    return;
  }

  // Đo tốc độ gió
  if (currentState == obstaclePresent) {
    if (lastState != obstaclePresent) {
      time1 = millis();
      lastState = obstaclePresent;
    } else if (obstacleIsNotPresent) {  // Tính toán chỉ khi đến lần chạm thứ 2
      time2 = millis();
      duration = (time2 - time1 - 0.05) / 1000;  // Chuyển đổi sang giây
      windSpeed = S / duration;           // Tốc độ gió sẽ có đơn vị là m/s
      windSpeed = windSpeed * 3.6; //Đổi sang km/h

      if (windSpeed > maxWindSpeed) maxWindSpeed = windSpeed;

      lastState = !obstaclePresent;
    }
    obstacleIsNotPresent = false;
  } else if (lastState == obstaclePresent) {
    // không chạm
    obstacleIsNotPresent = true;
  }

  // Serial.print(t);
  // Serial.print(",");
  // Serial.print(h);
  // Serial.print(",");
  // Serial.print(rainAmountAnalog * 0.56);
  // Serial.print(",");
  // Serial.print(UV, 4);
  // Serial.print(",");
  // Serial.print(airQuality);
  // Serial.print(",");
  // Serial.print(maxWindSpeed);
  // Serial.println("");

  light.disable();

  delay(20);
}

// function that executes whenever data is received from master
void receiveEvent(int howMany) {
  while (0 < Wire.available()) {
    char c = Wire.read(); /* receive byte as a character */
    Serial.print(c);      /* print the character */
  }
  //Serial.println(); /* to newline */
}

// function that executes whenever data is requested from master
void requestEvent() {
  //Wire.write("Hello 123"); /*send string on request */
  String result = "";
  float rainAmount = rainAmountAnalog * 0.56;
  float airQ = (float) airQuality;
  result = result + String(t, 1) + "a" + String(h, 1) + "a" + String(rainAmount, 1) + "a" + String(UV, 1) + "a" + String(airQ, 1) + "a" + String(maxWindSpeed, 1);
  const char* request = result.c_str();
  Serial.println(request);

  Wire.write(request); /*send string on request */
}