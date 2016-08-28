/*****
 
 All the resources for this project:
 https://rntlab.com/
 
*****/

// Loading the required libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <RCSwitch.h>
#include "DHT.h"

// Uncomment one of the lines bellow for whatever DHT sensor type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "YOUR_RPi_IP_Address";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

RCSwitch mySwitch = RCSwitch();

// Smoke Sensor
int smokePin = A0;
// DHT Sensor
const int DHTPin = 5;

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Smoke Threshold
int smokeThres = 60;

// Control Variables
boolean armMotion = false;
boolean armSmoke = false;
boolean smokeTriggered = false;
boolean motionTriggered = false;

// PIR Motion Sensor
const int motionSensor = 4;

// Status LEDs
const int smokeLED = 13;
const int motionLED = 12;

// Buzzer
const int buzzerPin = 14;

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;
long lastSmokeCheck = 0;

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic home/office/esp1/desk, you check if the message is either 1 or 0. Turns the Desk outlet according to the message
  if(topic=="home/office/esp1/desk"){
      Serial.print("Changing Desk light to ");
      if(messageTemp == "1"){
        mySwitch.send("000101010101000101010101");
        Serial.print("On");
      }
      else if(messageTemp == "0"){
        mySwitch.send("000101010101000101010100");
        Serial.print("Off");
      }
  }
  if(topic=="home/office/esp1/workbench"){
      Serial.print("Changing Workbench light to ");
      if(messageTemp == "1"){
        mySwitch.send("000101010101010001010101");
        Serial.print("On");
      }
      else if(messageTemp == "0"){
        mySwitch.send("000101010101010001010100");
        Serial.print("Off");
      }
  }
  if(topic=="home/office/esp1/smoke"){
      Serial.print("SMOKE SENSOR STATUS CHANGE");
      if(messageTemp == "1"){
        Serial.print("Smoke Sensor Armed");
        client.publish("home/office/esp1/smoke/status", "Armed");      
        client.publish("home/office/esp1/smoke/notification", "NO SMOKE");
        armSmoke = true;
        smokeTriggered = false;
        digitalWrite(smokeLED, HIGH);
      }
      else if(messageTemp == "0"){
        Serial.print("Smoke Sensor Not Armed");
        client.publish("home/office/esp1/smoke/status", "Not Armed");      
        client.publish("home/office/esp1/smoke/notification", "NO SMOKE");
        armSmoke = false;
        smokeTriggered = false;
        digitalWrite(smokeLED, LOW);
      }
   }
   if(topic=="home/office/esp1/motion"){
      Serial.print("MOTION SENSOR STATUS CHANGE");
      if(messageTemp == "1"){
        Serial.print("Motion Sensor Armed");
        client.publish("home/office/esp1/motion/status", "Armed");
        client.publish("home/office/esp1/motion/notification", "NO MOTION");
        armMotion = true;
        motionTriggered = false;
        digitalWrite(motionLED, HIGH);
      }
      else if(messageTemp == "0"){
        Serial.print("Motion Sensor Not Armed");
        client.publish("home/office/esp1/motion/status", "Not Armed");
        client.publish("home/office/esp1/motion/notification", "NO MOTION");
        armMotion=false;
        motionTriggered = false;
        digitalWrite(motionLED, LOW);
      }
   }
  Serial.println();
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    /*
     YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
     To change the ESP device ID, you will have to give a new name to the ESP8266.
     Here's how it looks:
       if (client.connect("ESP8266Client")) {
     You can do it like this:
       if (client.connect("ESP1_Office")) {
     Then, for the other ESP:
       if (client.connect("ESP2_Garage")) {
      That should solve your MQTT multiple connections problem
    */
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
      // Once connected, publish an announcement...
      client.publish("home/office/esp1/smoke/status", "Not Armed");
      client.publish("home/office/esp1/motion/status", "Not Armed");
      client.publish("home/office/esp1/smoke/notification", "NO SMOKE");
      client.publish("home/office/esp1/motion/notification", "NO MOTION");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("home/office/esp1/desk");
      client.subscribe("home/office/esp1/workbench");
      client.subscribe("home/office/esp1/smoke");
      client.subscribe("home/office/esp1/motion");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Checks motion
void detectsMovement() {
  if (armMotion && !motionTriggered) {
    Serial.println("MOTION DETECTED!!!");
    motionTriggered = true;
    client.publish("home/office/esp1/motion/notification", "MOTION DETECTED");
  }
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void setup() {
  pinMode(smokeLED, OUTPUT);
  pinMode(motionLED, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  pinMode(smokePin, INPUT);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);
  dht.begin();
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  mySwitch.enableTransmit(16);
    
  // SET YOUR PULSE LENGTH
  mySwitch.setPulseLength(REPLACE_WITH_YOUR_PULSE_LENGTH);
  
  // SET YOUR PROTOCOL (default is 1, will work for most outlets)
  mySwitch.setProtocol(REPLACE_WITH_YOUR_PROTOCOL);
  
  // Set number of transmission repetitions.
  mySwitch.setRepeatTransmit(15);
}

// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("espClient");
  
  now = millis();
  // Publishes new temperature and humidity every 30 seconds
  if (now - lastMeasure > 30000) {
    lastMeasure = now;
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Computes temperature values in Celsius
    float hic = dht.computeHeatIndex(t, h, false);
    static char temperatureTemp[6];
    dtostrf(hic, 6, 2, temperatureTemp);
    
    // Uncomment to compute temperature values in Fahrenheit 
    // float hif = dht.computeHeatIndex(f, h);
    // static char temperatureTemp[6];
    // dtostrf(hic, 6, 2, temperatureTemp);
    
    static char humidityTemp[6];
    dtostrf(h, 6, 2, humidityTemp);

    // Publishes Temperature and Humidity values
    client.publish("home/office/esp1/temperature", temperatureTemp);
    client.publish("home/office/esp1/humidity", humidityTemp);
    
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t Heat index: ");
    Serial.print(hic);
    Serial.println(" *C ");
    // Serial.print(hif);
    // Serial.println(" *F");
  }
  // Checks smoke
  if (now - lastSmokeCheck > 200) {
    lastSmokeCheck = now;  
    int smokeValue = analogRead(smokePin);
    if (smokeValue > smokeThres && armSmoke){
      Serial.print("Pin A0: ");
      Serial.println(smokeValue);
      tone(buzzerPin, 1000, 200);  
      if(!smokeTriggered){
        Serial.println("SMOKE DETECTED!!!");
        smokeTriggered = true;
        client.publish("home/office/esp1/smoke/notification", "SMOKE DETECTED");
      }
    }
  }
}
