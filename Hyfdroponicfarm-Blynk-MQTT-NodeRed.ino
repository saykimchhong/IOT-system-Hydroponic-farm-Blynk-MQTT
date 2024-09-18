/*
  README

  Arduino Pin usage

  04 : LED wifi status front box (Actived when wifi is connected)
  02 : Buzzer (Actived once when wifi is connected)
  23 : DHT11 sensor
  16 : Water temperature sensor A
  17 : Sprayer pump
  18 : Water Flow sensor
  19 : WAter temperature B
  25 : LED wifi is connected (on board)
  32 : LED wifi is disconnect (on board)

*/


#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "blynk-template-id"
#define BLYNK_TEMPLATE_NAME "blynk-template-name"
#define BLYNK_AUTH_TOKEN "blynk-auth-token"
#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BlynkSimpleEsp32.h>
#include <ezTime.h>
#include <FlowSensor.h>  // Install FlowSensor by hafidhh
#include <BH1750.h>      // Install BH1750 by Christohper law
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#define ssid "wifi-ssid"
#define password "wifi-password"

// Define for line API
#define LINE_ACCESS_TOKEN "line-access-token"
// Replace with the user ID of the LINE user or group to which you want to send a message
const String userId = "";  // You need to get this ID from the LINE bot setup
const char* lineMessagingUrl = "https://api.line.me/v2/bot/message/push";


// MQTT Configuration
const char* mqtt_server = "your-mqtt-server";
const int mqtt_port = 10774;

WiFiClient espClient;
PubSubClient client(espClient);

Timezone local;
BlynkTimer timer;

#define DHT_SENSOR_PIN 23
#define DHT_SENSOR_TYPE DHT11
DHT dht(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

// Water Temperature A
#define ONE_WIRE_BUS_WaterTemp_A 16  //Define water sensor 18B20 to pin 16
OneWire oneWireA(ONE_WIRE_BUS_WaterTemp_A);
DallasTemperature waterTempSensorA(&oneWireA);
// Water Temperature B
#define ONE_WIRE_BUS_WaterTemp_B 19  //Define water sensor 18B20 to pin 19
OneWire oneWireB(ONE_WIRE_BUS_WaterTemp_B);
DallasTemperature waterTempSensorB(&oneWireB);

// Water flow sensor
uint16_t waterFlowSensorType = YFS201;
uint8_t waterFlowSensorPin = 18;
FlowSensor waterFlowSensor(waterFlowSensorType, waterFlowSensorPin);


// Light meter
BH1750 lightMeter;

void IRAM_ATTR count() {
  waterFlowSensor.count();
}

// Define sprayer pump relay pin 17
int switch_sprayer_pump = 17;

int ledConnect_onBoard = 25;
int ledDisConnect_onBoard = 32;
int ledConnect_Front = 4;
int Buzzer = 2;

unsigned long lastReadTime = 0;
unsigned long lastSendTime = 0;
const unsigned long readInterval = 100;
const unsigned long sendInterval = 5000;

int systemMode;

const char* timestamp;

void printClock() {
  Serial.println("Time: " + local.dateTime());
}

BLYNK_WRITE(V2) {
  if (param.asInt() == 0) {
    systemMode = 0;
    Serial.print("System Mode: Auto");
    Serial.println(systemMode);
  } else {
    systemMode = 1;
    Serial.print("System Mode: Manual");
    Serial.println(systemMode);
  }
}

BLYNK_WRITE(V8) {
  if (param.asInt() == 1) {
    sendLineMessage("เติมชีวภัณฑ์ Bacillus subtilis ควบคุมโรครากเน่า วิธีการใช้ การเตรียมเพื่อใช้ทันที: ใช้ชีวภัณฑ์ 100 กรัมต่อสารละลายในระบบปลูก 100 ลิตร เมื่อย้ายปลูก และถ้ามีอาการรากเน่าโคนเน่าใส่เพิ่มทุก 7 วัน");
    } else {
  }
}

BLYNK_CONNECTED() {
  Blynk.sendInternal("utc", "time");
  Blynk.sendInternal("utc", "tz_rule");
}

BLYNK_WRITE(InternalPinUTC) {
  String cmd = param[0].asStr();
  if (cmd == "time") {
    const uint64_t utc_time = param[1].asLongLong();
    UTC.setTime(utc_time / 1000, utc_time % 1000);
    Serial.print("Unix time (UTC): ");
    Serial.println(utc_time);
  } else if (cmd == "tz_rule") {
    String tz_rule = param[1].asStr();
    local.setPosix(tz_rule);
    Serial.print("POSIX TZ rule:   ");
    Serial.println(tz_rule);
  }
}

BLYNK_WRITE(V7) {
  TimeInputParam t(param);

  if (t.hasStartTime()) {
    Serial.println(String("Start: ") + t.getStartHour() + ":" + t.getStartMinute() + ":" + t.getStartSecond());
  } else if (t.isStartSunrise()) {
    Serial.println("Start at sunrise");
  } else if (t.isStartSunset()) {
    Serial.println("Start at sunset");
  } else {
  }

  if (t.hasStopTime()) {
    Serial.println(String("Stop: ") + t.getStopHour() + ":" + t.getStopMinute() + ":" + t.getStopSecond());
  } else if (t.isStopSunrise()) {
    Serial.println("Stop at sunrise");
  } else if (t.isStopSunset()) {
    Serial.println("Stop at sunset");
  } else {
  }

  Serial.println(String("Time zone: ") + t.getTZ());
  Serial.println(String("Time zone offset: ") + t.getTZ_Offset());

  for (int i = 1; i <= 7; i++) {
    if (t.isWeekdaySelected(i)) {
      Serial.println(String("Day ") + i + " is selected");
    }
  }

  Serial.println();
}

void WIFI_Connect() {
  digitalWrite(ledDisConnect_onBoard, HIGH);
  digitalWrite(ledConnect_onBoard, LOW);
  digitalWrite(ledConnect_Front, LOW);

  WiFi.disconnect();
  Serial.println("Booting Sketch...");
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  for (int i = 0; i < 10; i++) {
    if (WiFi.status() != WL_CONNECTED) {
      delay(250);
      digitalWrite(ledDisConnect_onBoard, LOW);
      digitalWrite(ledConnect_onBoard, LOW);
      digitalWrite(ledConnect_Front, LOW);
      Serial.print(".");
      delay(250);
      digitalWrite(ledDisConnect_onBoard, HIGH);
      digitalWrite(ledConnect_onBoard, LOW);
      delay(250);
      Serial.println("Device is disconnected!");
    }
  }

  digitalWrite(ledDisConnect_onBoard, LOW);
  digitalWrite(ledConnect_onBoard, HIGH);
  digitalWrite(ledConnect_Front, HIGH);
}


void MQTTReconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ArduinoClient")) {
      Serial.println("connected");
      client.publish("hydroponic/status", "Arduino connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Send a message to the LINE user or group
void sendLineMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(lineMessagingUrl);
    http.addHeader("Authorization", "Bearer " + String(LINE_ACCESS_TOKEN));
    http.addHeader("Content-Type", "application/json");

    String postData = "{\"to\": \"" + userId + "\", \"messages\": [{\"type\": \"text\", \"text\": \"" + message + "\"}]}";
    
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("LINE API Response: " + response);
    } else {
      Serial.println("Error code: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("Error: Not connected to WiFi");
  }
}

void setup() {
  Serial.begin(9600);

  Wire.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  dht.begin();
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  waterTempSensorA.begin();
  waterTempSensorB.begin();
  waterFlowSensor.begin(count);

  // Set up the MQTT server
  client.setServer(mqtt_server, mqtt_port);

  // timer.setInterval(10000, printClock);

  pinMode(switch_sprayer_pump, OUTPUT);
  pinMode(ledConnect_onBoard, OUTPUT);
  pinMode(ledDisConnect_onBoard, OUTPUT);
  pinMode(ledConnect_Front, OUTPUT);
  pinMode(Buzzer, OUTPUT);

  WIFI_Connect();
  digitalWrite(Buzzer, HIGH);
  delay(300);
  digitalWrite(Buzzer, LOW);

  delay(1000);
}

float temperature;

void loop() {
  waterFlowSensor.read();
  waterTempSensorA.requestTemperatures();
  waterTempSensorB.requestTemperatures();
  // Serial.print("Flow rate (L/minute) : ");
  // Serial.println(waterFlowSensor.getFlowRate_m());
  // Serial.print("Volume (L) : ");
  // Serial.println(waterFlowSensor.getVolume());
  float waterFlowVolume = waterFlowSensor.getFlowRate_m();
  float luxValue = lightMeter.readLightLevel();
  Serial.print("Lux value = ");
  Serial.println(luxValue);
  delay(3000);
  unsigned long currentMillis = millis();
  Blynk.run();
  timer.run();

  if (!client.connected()) {
    MQTTReconnect();
  }
  client.loop();

  if (currentMillis - lastSendTime >= sendInterval) {
    temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    float waterTempSensorAValue = waterTempSensorA.getTempCByIndex(0);
    float waterTempSensorBValue = waterTempSensorB.getTempCByIndex(0);
    Serial.print("Water temperature sensor value: ");
    Serial.println(waterTempSensorAValue);

    if (!isnan(temperature) && !isnan(humidity)) {
      Serial.print("Temperature value: ");
      Serial.println(temperature);
      Serial.print("Humidity value: ");
      Serial.println(humidity);
      Blynk.virtualWrite(V0, temperature);
      Blynk.virtualWrite(V1, humidity);
      Blynk.virtualWrite(V4, waterTempSensorAValue);
      Blynk.virtualWrite(V6, waterTempSensorBValue);
      Blynk.virtualWrite(V7, luxValue);
      Blynk.virtualWrite(V5, waterFlowVolume);

      // Create JSON object
      StaticJsonDocument<200> doc;
      doc["Atmophere-Temperature"] = temperature;
      doc["Atmophere-Humidity"] = humidity;
      doc["Water-Temperature-Box-A"] = waterTempSensorAValue;
      doc["Water-Temperature-Box-B"] = waterTempSensorBValue;
      doc["Light"] = luxValue;

      // Convert JSON object to string
      char jsonBuffer[256];
      serializeJson(doc, jsonBuffer);

      // Publish the JSON string to the MQTT topic
      client.publish("sensor", jsonBuffer);
      Serial.println("Published JSON:");
      Serial.println(jsonBuffer);
      
    } else {
      Serial.println("Failed to read data from DHT or BH1750 sensor!");
    }
    lastSendTime = currentMillis;
  }

  if (systemMode == 0) {
    Blynk.setProperty(V3, "isHidden", true);
  } else {
    Blynk.setProperty(V3, "isHidden", false);
  }

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(ledConnect_onBoard, HIGH);
    digitalWrite(ledConnect_Front, HIGH);
  } else {
    digitalWrite(ledConnect_onBoard, LOW);
    digitalWrite(ledConnect_Front, LOW);
    Serial.println("Device Disconnected!");
    ESP.restart();
    WIFI_Connect();
  }
}

BLYNK_WRITE(V3) {
  if (param.asInt() == 0) {
    digitalWrite(switch_sprayer_pump, 0);
    sendLineMessage("Pump OFF\n\n Temperature: " + String(temperature, 2));
  } else {
    digitalWrite(switch_sprayer_pump, 1);
    sendLineMessage("Pump ON\n\n Temperature: " + String(temperature, 2));
  }
}