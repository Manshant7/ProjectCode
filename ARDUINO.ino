#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>

// WiFi credentials
const char* ssid = "Manshant";
const char* password = "1234567890";

// MQTT settings
const char* mqtt_broker = "broker.hivemq.com";  // Public MQTT broker
const int mqtt_port = 1883;                     // Standard MQTT port
const char* soilMoistureTopic = "device/SoilMoisture";
const char* animalAlertTopic = "device/AnimalAlert";

// IFTTT webhook settings for sending notifications
const char* iftttKey = "daiBqN3cPle_CKgWlkywU3O-tdiCAW5puOJLcs2qMht"; 
const char* iftttEvent = "animal_alert";         

// Pin configuration
const int pirPin = 2;              // PIR sensor connected to digital pin 2
const int soilMoisturePin = A0;    // Soil moisture sensor connected to analog pin A0
const int moistureThreshold = 300; // Moisture threshold for activating motor

// WiFi and MQTT client objects
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

void setup() {
  Serial.begin(9600);

  // Connect to WiFi
  connectWiFi();

  // Connect to MQTT broker
  connectMQTT();

  // Initialize PIR sensor pin
  pinMode(pirPin, INPUT);
}

void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  // Wait until the connection is established
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void connectMQTT() {
  Serial.print("Connecting to MQTT broker...");
  // Keep trying to connect to the MQTT broker
  while (!mqttClient.connect(mqtt_broker, mqtt_port)) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to MQTT broker");
}

void loop() {
  mqttClient.poll(); // Maintain the MQTT connection

  // Check the PIR sensor for animal detection
  int pirState = digitalRead(pirPin);
  if (pirState == HIGH) {  // If motion is detected
    sendIFTTTNotification();  // Send notification via IFTTT
    mqttClient.beginMessage(animalAlertTopic); // Start MQTT message
    mqttClient.print("Animal detected!"); // Send alert message
    mqttClient.endMessage(); // End MQTT message
    delay(5000); // Delay to avoid repeated alerts
  }

  // Check the soil moisture level
  int moistureLevel = analogRead(soilMoisturePin);  // Read soil moisture sensor
  Serial.print("Soil Moisture Level: ");
  Serial.println(moistureLevel);

  if (moistureLevel < moistureThreshold) {  // Check if moisture is below threshold
    mqttClient.beginMessage(soilMoistureTopic);
    mqttClient.print(moistureLevel);  // Send moisture level
    mqttClient.endMessage();
    Serial.println("Low moisture, data sent to Raspberry Pi");
  }
  delay(1000); // Small delay to prevent excessive readings
}

void sendIFTTTNotification() {
  // Construct URL for IFTTT webhook request
  String url = String("/trigger/") + iftttEvent + "/with/key/" + iftttKey;
  WiFiClient client;
  // Connect to IFTTT server
  if (client.connect("maker.ifttt.com", 80)) {
    client.println("POST " + url + " HTTP/1.1");
    client.println("Host: maker.ifttt.com");
    client.println("Connection: close");
    client.println();
    Serial.println("IFTTT notification sent");
  } else {
    Serial.println("Failed to connect to IFTTT");
  }
  client.stop();  // Close connection
}
