# Hydroponic System with WiFi, Sensor Monitoring, and LINE Notifications

## Overview

This project is an automated hydroponic system using an ESP32 microcontroller. The system monitors various environmental parameters like air temperature, humidity, water temperature, water flow, and light intensity. It also provides real-time control of a sprayer pump and sends alerts via LINE notifications. The system integrates with Blynk for remote control and monitoring, as well as an MQTT broker for data communication.

## Features

* **WiFi Connectivity:** Connects to WiFi and Blynk for remote monitoring and control.
* **Sensors:**
    * DHT11 for air temperature and humidity.
    * DS18B20 for water temperature monitoring.
    * YFS201 for water flow measurement.
    * BH1750 for light intensity monitoring.
* **Sprayer Pump Control:** Automatically or manually controls a sprayer pump for plant hydration.
* **LINE Notifications:** Sends messages to LINE when system events occur, such as turning the pump on/off or adding biological agents.
* **MQTT Integration:** Sends sensor data to an MQTT broker for logging or further processing.

## Hardware Components

* ESP32 Microcontroller
* DHT11 Sensor (Air temperature and humidity)
* DS18B20 Sensors (Water temperature monitoring)
* YFS201 Flow Sensor (Water flow rate measurement)
* BH1750 Light Meter (Light intensity)
* Relay Module (For sprayer pump control)
* Buzzer (For WiFi connection status)
* LEDs (Indicate WiFi connection status)
* Sprayer Pump

## Pin Configuration

| Component                      | Pin | Description                                  |
| ------------------------------ | --- | -------------------------------------------- |
| WiFi Status LED (Front Box)   | 04  | Indicates if WiFi is connected (active HIGH) |
| Buzzer                         | 02  | Activates once when WiFi is connected       |
| DHT11 Sensor                  | 23  | Measures air temperature and humidity       |
| Water Temp Sensor A (18B20) | 16  | Measures water temperature in Box A          |
| Water Temp Sensor B (18B20) | 19  | Measures water temperature in Box B          |
| Water Flow Sensor              | 18  | Measures water flow rate                    |
| Sprayer Pump                   | 17  | Controls the sprayer pump                    |
| LED (Onboard, Connected)       | 25  | Indicates WiFi is connected                |
| LED (Onboard, Disconnected)    | 32  | Indicates WiFi is disconnected               |

## Software Setup

### Libraries Used

Install the following libraries via Arduino IDE:

* BlynkSimpleEsp32: For Blynk integration.
* DHT: For DHT11 sensor.
* DallasTemperature: For DS18B20 sensors.
* FlowSensor: For water flow sensor (YFS201).
* BH1750: For light meter.
* PubSubClient: For MQTT integration.
* ezTime: For time management and time zone settings.
* ArduinoJson: For handling JSON data.

### Configuration

* **Blynk Setup:**
    * Replace the `BLYNK_AUTH_TOKEN` with your Blynk authentication token.
    * Define your WiFi SSID and password.
* **LINE Notifications:**
    * Replace `LINE_ACCESS_TOKEN` with the access token from your LINE bot setup.
    * Define the `userId` to send messages to a specific user or group.
* **MQTT Broker:**
    * Define the MQTT server address and port (`mqtt_server`, `mqtt_port`).

### Pin Definitions

The following pin definitions are used in the code:

```cpp
#define DHT_SENSOR_PIN 23
#define ONE_WIRE_BUS_WaterTemp_A 16
#define ONE_WIRE_BUS_WaterTemp_B 19
#define waterFlowSensorPin 18
#define switch_sprayer_pump 17
#define ledConnect_onBoard 25
#define ledDisConnect_onBoard 32
#define ledConnect_Front 4
#define Buzzer 2
```

## How It Works

**WiFi and Blynk Connection:**
The system attempts to connect to the configured WiFi. If successful, the LEDs and buzzer will indicate the connection status. Blynk is initialized for remote monitoring.

**Sensor Monitoring:**
Every 5 seconds, the system reads data from the following sensors:

* DHT11: Air temperature and humidity.
* DS18B20: Water temperature in two boxes.
* YFS201: Water flow rate.
* BH1750: Light intensity.

The readings are displayed on the Blynk dashboard and sent to the MQTT broker.

**Sprayer Pump Control:**
The system can automatically control the sprayer pump based on sensor readings or manually via the Blynk app. The pump's status is also reported via LINE notifications.

**LINE Notifications:**
The system sends real-time alerts to the LINE messaging app. For example, when the sprayer pump is turned on/off or when a message about adding biological agents is triggered.

**MQTT Communication:**
Sensor data is packaged into a JSON object and published to the configured MQTT broker. This allows remote logging or additional automation.

## Circuit Diagram

Connect the sensors and components to the ESP32 as per the pin configuration table. Ensure proper power supply for the ESP32 and the connected sensors.

## Future Enhancements

* Implement more advanced control algorithms for the sprayer pump based on environmental conditions.
* Add more sensors, such as pH or EC sensors, for comprehensive monitoring of the hydroponic system.
* Integrate a web dashboard for real-time monitoring using the MQTT data.

## License

This project is open-source and licensed under the MIT License.
