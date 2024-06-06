#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.mqtt.cool";
const char* controlling1 = "uas/control1";
const char* controlling2 = "uas/control2";
const char* controlling3 = "uas/control3";

const int led1 = 14;
const int led2 = 19;
const int led3 = 5;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe(controlling1);
      client.subscribe(controlling2);
      client.subscribe(controlling3);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  payload[length] = '\0';
  String message = String((char*)payload);

  if (strcmp(topic, controlling1) == 0) {
    if (message == "0") {
      digitalWrite(led1, LOW);
    } else {
      digitalWrite(led1, HIGH);
    }
  }

  if (strcmp(topic, controlling2) == 0) {
    if (message == "0") {
      digitalWrite(led2, LOW);
    } else {
      digitalWrite(led2, HIGH);
    }
  }

  if (strcmp(topic, controlling3) == 0) {
    if (message == "0") {
      digitalWrite(led3, LOW);
    } else {
      digitalWrite(led3, HIGH);
    }
  }
}
