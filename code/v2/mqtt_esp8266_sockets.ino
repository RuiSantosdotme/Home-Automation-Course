/*****
 
 All the resources for this project:
 https://rntlab.com/
 
*****/

// Loading the ESP8266WiFi library and the PubSubClient library
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
// Loading the RCSwitch library to control the outlets
#include <RCSwitch.h>

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "YOUR_RPi_IP_Address";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

// Initializes the RCSwitch
RCSwitch mySwitch = RCSwitch();

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

  // If a message is received on the topic home/office/esp1/gpio2, you check if the message is either 1 or 0. Turns the ESP GPIO according to the message
  if(topic=="home/office/esp1/desk"){
      Serial.print("Changing Desk Light to ");
      if(messageTemp == "1"){
        // Sends binary code to turn on Desk Light
        // BINARY CODE EXAMPLE. REPLACE WITH YOUR BINARY CODE
        mySwitch.send("000101010101000101010101");
        Serial.print("On");
      }
      else if(messageTemp == "0"){
        // Sends binary code to turn off Desk Light
        // BINARY CODE EXAMPLE. REPLACE WITH YOUR BINARY CODE
        mySwitch.send("000101010101000101010100");
        Serial.print("Off");
      }
  }
  if(topic=="home/office/esp1/workbench"){
      Serial.print("Changing Workbench Light to ");
      if(messageTemp == "1"){
        // Sends binary code to turn on Workbench Light
        // BINARY CODE EXAMPLE. REPLACE WITH YOUR BINARY CODE
        mySwitch.send("000101010101010001010101");
        Serial.print("On");
      }
      else if(messageTemp == "0"){
        // Sends binary code to turn off Workbench Light
        // BINARY CODE EXAMPLE. REPLACE WITH YOUR BINARY CODE
        mySwitch.send("000101010101010001010100");
        Serial.print("Off");
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
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("home/office/esp1/desk");
      client.subscribe("home/office/esp1/workbench");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void setup() {

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
}
