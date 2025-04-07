#include <Wire.h>
#include <ESP32Servo.h>
#include <RTClib.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <FirebaseESP32.h>

// ✅ Define Pin Assignments
#define RED_LIGHT_1 26
#define YELLOW_LIGHT_1 27
#define GREEN_LIGHT_1 14
#define RED_LIGHT_2 12

#define TRIG1 4
#define ECHO1 5
#define TRIG2 18
#define ECHO2 19

#define IR_SENSOR1 33
#define IR_SENSOR2 32

#define SERVO_PIN 13
Servo servo;

#define GSM_TX 17
#define GSM_RX 16
HardwareSerial gsmSerial(2);

RTC_DS3231 rtc;

// ✅ WiFi Configuration
const char* ssid = "Aniii";
const char* password = "987654321";

// ✅ MQTT Configuration
#define MQTT_BROKER "mqtt.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "traffic/test"

WiFiClient espClient;
PubSubClient client(espClient);

// ✅ Firebase Configuration
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;
FirebaseData firebaseData;

// ✅ Function to Setup WiFi
void setupWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi Connection Failed! Check credentials or router.");
  }
}

// ✅ Function to Setup MQTT
void setupMQTT() {
  client.setServer(MQTT_BROKER, MQTT_PORT);
  Serial.print("Connecting to MQTT...");
  if (client.connect("ESP32_Client")) {
    Serial.println("\n✅ MQTT Connected!");
    client.publish(MQTT_TOPIC, "ESP32 is online!");
  } else {
    Serial.println("\n❌ MQTT Connection Failed! Check broker and network.");
  }
}

// ✅ Corrected Firebase Initialization
void setupFirebase() {
  firebaseConfig.host = "traffic-new-90b5d-default-rtdb.firebaseio.com";
  firebaseConfig.signer.tokens.legacy_token = "yRKiQPJpbkDOWWZdCsjr32KsWiNDdw6ofTvLpuac";

  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);
  Serial.println("✅ Firebase Initialized Successfully!");
}

// ✅ Function to Send Test Data to Firebase
void testFirebase() {
  Serial.print("Sending test data to Firebase... ");
  if (Firebase.setString(firebaseData, "/test", "ESP32 is connected!")) {
    Serial.println("✅ Data Sent to Firebase Successfully!");
  } else {
    Serial.println("❌ Firebase Error: " + firebaseData.errorReason());
  }
}

// ✅ Function to Test Ultrasonic Sensors
long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return -1;
  
  return duration * 0.034 / 2;
}

void testUltrasonicSensors() {
  long distance1 = getDistance(TRIG1, ECHO1);
  long distance2 = getDistance(TRIG2, ECHO2);
  Serial.print("Ultrasonic 1: "); Serial.print(distance1); Serial.println(" cm");
  Serial.print("Ultrasonic 2: "); Serial.print(distance2); Serial.println(" cm");
}

// ✅ Function to Test IR Sensors
void testIRSensors() {
  int ir1 = digitalRead(IR_SENSOR1);
  int ir2 = digitalRead(IR_SENSOR2);
  Serial.print("IR Sensor 1: "); Serial.println(ir1 ? "NO Obstacle" : "Obstacle Detected");
  Serial.print("IR Sensor 2: "); Serial.println(ir2 ? "NO Obstacle" : "Obstacle Detected");
}

// ✅ Setup Function
void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);
  pinMode(IR_SENSOR1, INPUT_PULLUP);
  pinMode(IR_SENSOR2, INPUT_PULLUP);
  
  servo.attach(SERVO_PIN);
  
  if (!rtc.begin()) {
    Serial.println("❌ RTC not found!");
  } else {
    Serial.println("✅ RTC Initialized!");
  }

  gsmSerial.begin(9600, SERIAL_8N1, GSM_RX, GSM_TX);

  Serial.println("\n🔹 Running Full Hardware Test...\n");

  setupWiFi();
  setupMQTT();
  setupFirebase();

  testFirebase();
  testUltrasonicSensors();
  testIRSensors();

  Serial.println("\n✅ All Tests Completed!\n");
}

// ✅ Loop Function
void loop() {
  if (!client.connected()) setupMQTT();
  client.loop();
  
  testUltrasonicSensors();
  testIRSensors();

  delay(5000);
}
