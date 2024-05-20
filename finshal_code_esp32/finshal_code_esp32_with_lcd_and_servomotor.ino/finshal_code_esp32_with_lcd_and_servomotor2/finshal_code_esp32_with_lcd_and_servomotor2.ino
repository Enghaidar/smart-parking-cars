#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>


const char* ssid = "A.S";
const char* password = "1967@1974@";
const char* mqtt_server = "192.168.0.88";

const char* topic = "parking/status1"; //topic to publish the parking state to broker
const char* topic1 = "parking/told1"; // topic to subscribe the parking state from broker

const int irPin1 = 13; // IR sensor pin
const int irPin2 = 12; // IR sensor pin
const int irPin3 = 14; // IR sensor pin
const int irPin4 = 27; // IR sensor pin
const int irPinout = 19; // IR sensor pin

int parkingSpaceID1 = 1; // Change this for each ESP32
int parkingSpaceID2 = 2; // Change this for each ESP32
int parkingSpaceID3 = 3; // Change this for each ESP32
int parkingSpaceID4 = 4; // Change this for each ESP32

static const int servoPin = 15;
static const int servoout = 2;
Servo servo1;
Servo servo2;


LiquidCrystal_I2C lcd(0x27, 16, 4); // I2C address 0x27, 16 column and 4 rows


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


bool openGateRequested = false;

void callback(char* topic1, byte* payload1, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload1[i];
  }
  // Print the received message
  Serial.print("Received message: ");
  Serial.println(message);
  // Update booking status based on the received message
  if (message.equals("open gate")) 
  {
    Serial.println("open gate");
    openGateRequested = true; // تعيين الطلب على فتح البوابة
  } 
  
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe(topic1);
    } 
    else 
    {
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
  pinMode(irPinout, INPUT);

  lcd.init(); // initialize the lcd
  lcd.backlight(); // to clear lcd

  Serial.begin(115200);
  servo1.attach(servoPin);
  servo2.attach(servoout);

  setup_wifi();
  client.setServer(mqtt_server, 1883); // define my broker and message port
  client.setCallback(callback); // define the connecting void

}

void loop() {

  int sensorValueOUT = digitalRead(irPinout);
   if (sensorValueOUT==LOW)
  {
    Serial.println("xxx");
      servo2.write(100);
      delay(2000);
      servo2.write(0);
      delay(2000);
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read IR sensor
  int sensorValue1 = digitalRead(irPin1);
  int sensorValue2 = digitalRead(irPin2);
  int sensorValue3 = digitalRead(irPin3);
  int sensorValue4 = digitalRead(irPin4);
  int posDegrees=90;
   if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // قم بفتح البوابة إذا كانت هناك طلبات لفتح البوابة
  if (openGateRequested) 
  {
    // تحريك محرك السيرفو إلى 180 درجة
    
      servo1.write(100);
      delay(2000);
      //delay(2000); // انتظار 2 ثانية

    // إعادة توجيه محرك السيرفو إلى 0 درجة
      servo1.write(0);
      delay(2000);

    // استجابة لطلب فتح البوابة
    openGateRequested = false; // إعادة تعيين الطلب
  }

  // Publish status to MQTT broker
  if (sensorValue1 == HIGH) 
  {

                                  // clear display
    lcd.setCursor(0, 0);         // move cursor to   (0, 0)
    lcd.print("P1=occupied");
    client.publish("parking/status1", (String(parkingSpaceID1) + ":vacant").c_str());
  } 
  else 
  {  
   //lcd.clear();
    lcd.setCursor(0, 0);         // move cursor to   (0, 0)
    lcd.print("P1=  vacant");
    client.publish("parking/status1", (String(parkingSpaceID1) + ":occupied").c_str());
  }
  //delay(1000);

  // Publish status to MQTT broker
  if (sensorValue2 == HIGH)
   {
     
    lcd.setCursor(0, 1);         // move cursor to   (6, 0)
    lcd.print("P2=occupied");
    client.publish("parking/status1", (String(parkingSpaceID2) + ":vacant").c_str());
  } 
  else
  {
    //lcd.clear();
    lcd.setCursor(0, 1);         // move cursor to   (6, 0)
    lcd.print("P2=  vacant");
    client.publish("parking/status1", (String(parkingSpaceID2) + ":occupied").c_str());
  }
  //delay(1000);

  if (sensorValue3 == HIGH) 
  {
    
    lcd.setCursor(0, 2);         // move cursor to   (0, 1)
    lcd.print("P3=occupied");
    client.publish("parking/status1", (String(parkingSpaceID3) + ":vacant").c_str());
  } 
  else 
  {
    //lcd.clear();
    lcd.setCursor(0, 2);         // move cursor to   (0, 1)
    lcd.print("P3=  vacant");
    client.publish("parking/status1", (String(parkingSpaceID3) + ":occupied").c_str());
  }
  //delay(1000);

  if (sensorValue4 == HIGH) 
  {
    
    lcd.setCursor(0,3);         // move cursor to   (6, 1)
    lcd.print("P4=occupied");
    client.publish("parking/status1", (String(parkingSpaceID4) + ":vacant").c_str());
  } 
  else 
  {
    
    //lcd.clear();                 // clear display
    lcd.setCursor(0, 3);         // move cursor to   (6, 1)
    lcd.print("P4=  vacant");
    client.publish("parking/status1", (String(parkingSpaceID4) + ":occupied").c_str());
  }

    
}
