#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <time.h>  

const char *ssid = "Wokwi-GUEST";  // WiFi network name
const char *password = "";  // WiFi password

const char *mqtt_broker = "broker.emqx.io";  // MQTT broker
const char *mqtt_topic = "UTS_152022038/emqx/esp32";  // MQTT topic
const char *mqtt_username = "daniiryanto";  // MQTT username
const char *mqtt_password = "akudaniiryanto1";  // MQTT password
const int mqtt_port = 1883;  // MQTT port

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

#define DHTPIN 32
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const int LED_GREEN = 17;
const int LED_YELLOW = 12;
const int LED_RED = 18;
const int RELAY_PIN = 5;
const int BUZZER_PIN = 19;

void connectToWiFi();
void connectToMQTTBroker();
void mqttCallback(char *topic, byte *payload, unsigned int length);
void initTime();
String getFormattedTime();

void setup() {
    Serial.begin(115200);

    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    dht.begin();

    connectToWiFi();
    initTime();  
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setCallback(mqttCallback);
    connectToMQTTBroker();
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to the WiFi network");
}

void initTime() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    Serial.print("Waiting for time");
    while (!time(nullptr)) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nTime initialized");
}

String getFormattedTime() {
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    char buffer[26];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return String(buffer);
}

void connectToMQTTBroker() {
    while (!mqtt_client.connected()) {
        String client_id = "esp8266-client-" + String(WiFi.macAddress());
        Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
        if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Connected to MQTT broker");
            mqtt_client.subscribe(mqtt_topic);
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 2 seconds");
            delay(2000);
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.println(topic);
}

void loop() {
    if (!mqtt_client.connected()) {
        connectToMQTTBroker();
    }
    mqtt_client.loop();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    String timestamp = getFormattedTime();

    String jsonPayload = "{";
    jsonPayload += "\"timestamp\":\"" + timestamp + "\",";
    jsonPayload += "\"temperature\":" + String(temperature, 2) + ",";
    jsonPayload += "\"humidity\":" + String(humidity, 2);
    jsonPayload += "}";

    if (mqtt_client.publish(mqtt_topic, jsonPayload.c_str())) {
        Serial.println("Message sent successfully");
        Serial.println("Payload: " + jsonPayload);
    } else {
        Serial.println("Failed to send message");
    }

    if (temperature > 35) {
        digitalWrite(LED_RED, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_GREEN, LOW);
    } else if (temperature >= 30 && temperature <= 35) {
        digitalWrite(LED_YELLOW, HIGH);
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, LOW);
    } else {
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, LOW);
    }

    if (temperature > 30) {
        digitalWrite(RELAY_PIN, HIGH);
    } else {
        digitalWrite(RELAY_PIN, LOW); 
    }
    
    delay(5000);
}
