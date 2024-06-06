#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.mqtt.cool";

const char* serverName = "https://platform.antares.id:8443/~/antares-cse/antares-id/AQWM/weather_airQuality_nodeCore_teknik/la";
const char* apiKey = "ee8d16c4466b58e1:3b8d814324c84c89";
const char* sensorDataTopic = "uas/sensor";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  displaySensorData();
  delay(2000);
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void displaySensorData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("X-M2M-Origin", apiKey);
    http.addHeader("Accept", "application/json");

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Received payload: " + payload);

      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        JsonObject cinObj = doc["m2m:cin"];
        String conData = cinObj["con"];

        StaticJsonDocument<600> conDoc;
        DeserializationError conError = deserializeJson(conDoc, conData);

        if (!conError) {
          int pm1 = conDoc["PM1.0"].as<int>();
          int pm25 = conDoc["PM2.5"].as<int>();
          int pm10 = conDoc["PM10"].as<int>();

          Serial.print("PM1.0: ");
          Serial.println(pm1);
          Serial.print("PM2.5: ");
          Serial.println(pm25);
          Serial.print("PM10: ");
          Serial.println(pm10);

          publishSensorData(pm1, pm25, pm10);
        } else {
          Serial.print("Error parsing 'con' JSON: ");
          Serial.println(conError.c_str());
        }
      } else {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      }
    } else {
      Serial.print("Error on HTTP request: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    connectToWiFi();
  }
}

void publishSensorData(int pm1, int pm25, int pm10) {
  if (!client.connected()) {
    reconnect();
  }

  StaticJsonDocument<600> jsonDoc;
  JsonObject JSONencoder = jsonDoc.to<JsonObject>();

  JSONencoder["PM1.0"] = pm1;
  JSONencoder["PM2.5"] = pm25;
  JSONencoder["PM10"] = pm10;

  char buffer[600];
  size_t n = serializeJson(JSONencoder, buffer, sizeof(buffer));
  Serial.println("Publishing message: " + String(buffer));
  bool success = client.publish(sensorDataTopic, buffer, n);
  if (success) {
    Serial.println("Message published successfully");
  } else {
    Serial.println("Message publishing failed");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
