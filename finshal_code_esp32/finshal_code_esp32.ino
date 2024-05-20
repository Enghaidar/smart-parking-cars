#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "A.S";
const char* password = "1967@1974@";
const char* mqtt_server = "192.168.0.88";

const char* topic = "parking/status";

const char* topic1 = "parking/told";

const int irPin1 = 13; // IR sensor pin
const int irPin2 = 12; // IR sensor pin
const int irPin3 = 14; // IR sensor pin
const int irPin4 = 27; // IR sensor pin
int parkingSpaceID1 = 1; // Change this for each ESP32
int parkingSpaceID2 = 2; // Change this for each ESP32
int parkingSpaceID3 = 3; // Change this for each ESP32
int parkingSpaceID4 = 4; // Change this for each ESP32

WiFiClient espClient;
PubSubClient client(espClient);

bool isSpaceBooked = false; // Variable to track booking status

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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

void callback(char* topic1, byte* payload, unsigned int length) {

  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  // Print the received message
  Serial.print("Received message: ");
  Serial.println(message);

  // Update booking status based on the received message
  if (message.equals("1:booked") || message.equals("2:booked") || message.equals("3:booked") || message.equals("4:booked")) {
     Serial.println("booked");
    digitalWrite(2, HIGH);
    delay(2000);
    digitalWrite(2, LOW);
    delay(500);
  } 
  else {
    // Handle other states here
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe(topic1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(irPin1, INPUT);
  pinMode(irPin2, INPUT);
  pinMode(irPin3, INPUT);
  pinMode(irPin4, INPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(2, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read IR sensor
  int sensorValue1 = digitalRead(irPin1);
  int sensorValue2 = digitalRead(irPin2);
  int sensorValue3 = digitalRead(irPin3);
  int sensorValue4 = digitalRead(irPin4);

  // Publish status to MQTT broker
  if (sensorValue1 == HIGH) {
    client.publish("parking/status", (String(parkingSpaceID1) + ":occupied").c_str());
  } else {
    client.publish("parking/status", (String(parkingSpaceID1) + ":vacant").c_str());
  }
  //delay(1000);

  // Publish status to MQTT broker
  if (sensorValue2 == HIGH) {
    client.publish("parking/status", (String(parkingSpaceID2) + ":occupied").c_str());
  } else {
    client.publish("parking/status", (String(parkingSpaceID2) + ":vacant").c_str());
  }
  //delay(1000);

  if (sensorValue3 == HIGH) {
    client.publish("parking/status", (String(parkingSpaceID3) + ":occupied").c_str());
  } else {
    client.publish("parking/status", (String(parkingSpaceID3) + ":vacant").c_str());
  }
  //delay(1000);

  if (sensorValue4 == HIGH) {
    client.publish("parking/status", (String(parkingSpaceID4) + ":occupied").c_str());
  } else {
    client.publish("parking/status", (String(parkingSpaceID4) + ":vacant").c_str());
  }
  //delay(1000);
}
