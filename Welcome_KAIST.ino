/*
 * How to start:
 * https://www.instructables.com/id/Programming-the-WeMos-Using-Arduino-SoftwareIDE/
 * 
 * Follow the instruction above, install the latest esp8266 board and choose WeMos D1 R1.
 * Also, install the library [ArduinoJson] version 5.XX from library manager.
 * 
 * About IR motion sensor:
 * http://henrysbench.capnfatz.com/henrys-bench/arduino-sensors-and-input/arduino-hc-sr501-motion-sensor-tutorial/
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

extern "C" {
#include "user_interface.h"
#include "wpa2_enterprise.h"
#include "c_types.h"
}

bool debug = true;

// SSID to connect to
char ssid[] = "Welcome_KAIST";
char wlpasswd[] = "";
char username[] = "웰카ID";
char identity[] = "웰카ID";
char password[] = "웰카PW";

char server[] = "http://socrip4.kaist.ac.kr:2080/arduino-data";
int port = 2080;

void setup() {

  WiFi.mode(WIFI_STA);
  Serial.begin(9600);
  delay(1000);

  if (debug) {
    Serial.setDebugOutput(true);
    Serial.printf("SDK version: %s\n", system_get_sdk_version());
    Serial.printf("Free Heap: %4d\n",ESP.getFreeHeap());
  }
  
  // Setting ESP into STATION mode only (no AP mode or dual mode)
  wifi_set_opmode(STATION_MODE);

  struct station_config wifi_config;

  memset(&wifi_config, 0, sizeof(wifi_config));
  strcpy((char*)wifi_config.ssid, ssid);
  strcpy((char*)wifi_config.password, wlpasswd);

  wifi_station_set_config(&wifi_config);
  
  wifi_station_set_wpa2_enterprise_auth(1);

  // Clean up to be sure no old data is still inside
  wifi_station_clear_cert_key();
  wifi_station_clear_enterprise_ca_cert();
  wifi_station_clear_enterprise_identity();
  wifi_station_clear_enterprise_username();
  wifi_station_clear_enterprise_password();
  wifi_station_clear_enterprise_new_password();
  
  wifi_station_set_enterprise_identity((uint8*)identity, strlen(identity));
  
  wifi_station_set_enterprise_identity((uint8*)identity, strlen(identity));
  wifi_station_set_enterprise_username((uint8*)username, strlen(username));
  wifi_station_set_enterprise_password((uint8*)password, strlen((char*)password));
  
  wifi_station_connect();
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    if (debug) {
      Serial.print(".");
    }
  }

  if (debug) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  
  pinMode(D2,OUTPUT); // 센서 Trig 핀
  pinMode(D3,INPUT); // 센서 Echo 핀
  pinMode(D4,INPUT_PULLUP); // 적외선 센서
}

void loop() {
  HTTPClient http;
  long duration, cm;
  int httpCode;
  StaticJsonBuffer<100> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  char JSONmessageBuffer[100];
  int IR;

  digitalWrite(D2,HIGH); // 센서에 Trig 신호 입력
  delayMicroseconds(10); // 10us 정도 유지
  digitalWrite(D2,LOW); // Trig 신호 off
  duration = pulseIn(D3,HIGH); // Echo pin: HIGH->Low 간격을 측정
  cm = duration / 29 / 2; // 거리(cm)로 변환

  IR = digitalRead(D4);

  JSONencoder["roomNum"] = 1;
  JSONencoder["distance"] = cm;
  JSONencoder["IR"] = IR;
  JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

  if (debug) {
    Serial.println(JSONmessageBuffer);
  }

  http.begin(server);
  http.addHeader("Content-Type", "application/json");
  httpCode = http.POST(JSONmessageBuffer);
  http.end();

  delay(10000);
}
