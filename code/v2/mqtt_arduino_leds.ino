/*****
 
 All the resources for this project:
 https://rntlab.com/
 
*****/

// Loading the libraries
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// This MAC addres can remain the same
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };

/* 
 *  IMPORTANT!
 *  YOU MUST UPDATE THESE NEXT TWO VARIABLES WITH VALUES SUITABLE TO YOUR NETWORK!
 *  
*/

// Replace with an IP that is suitable for your network. I could use any IP in this range: 192.168.1.X
// I've used a tool http://angryip.org/ to see an available IP address and I ended up using 192.168.1.99
IPAddress ip(192, 168, 1, 99);

// Replace with your Raspberry Pi IP Address. In my case, the RPi IP Address is 192.168.1.76
IPAddress server(192, 168, 1, 76);

// Initializes the clients
EthernetClient ethClient;
PubSubClient client(ethClient);

// Connect two LEDs to your Arduino. One to pin 6 and the other to pin 7
const int ledPin6 = 6;
const int ledPin7 = 7;

// This functions is executed when some device publishes a message to a topic that your Arduino is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your Arduino is subscribed you can actually do something
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  // Feel free to add more if statements to control more Pins with MQTT

  // If a message is received on the topic home/livingroom/arduino/ledPin6 
  // It's a value between 0 and 255 to adjust the LED brightness
  if(String(topic)=="home/livingroom/arduino/ledPin6"){
      Serial.print("Changing Digital Pin 6 Brithness to ");
      Serial.print(messageTemp);
       analogWrite(ledPin6, messageTemp.toInt());
  }
  // If a message is received on the topic home/livingroom/arduino/ledPin7, 
  //you check if the message is either 1 or 0. Turns the Arduino Digital Pin according to the message
  if(String(topic)=="home/livingroom/arduino/ledPin7"){
      Serial.print("Changing Digital Pin 7 to ");
      if(messageTemp == "1"){
        digitalWrite(ledPin7, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "0"){
        digitalWrite(ledPin7, LOW);
        Serial.print("Off");
      }
  }
  Serial.println();
}


// This functions reconnects your Arduino to your MQTT broker
// Change the function below if you want to subscribe to more topics with your Arduino 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("home/livingroom/arduino/ledPin6");
      client.subscribe("home/livingroom/arduino/ledPin7");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// The setup function sets your Arduino pins as Outputs, starts the serial communication at a baud rate of 57600
// Sets your mqtt broker and sets the callback function. The callback function is what receives messages and
// actually controls the LEDs. Finally starts the Ethernet communication
void setup()
{
  pinMode(ledPin6, OUTPUT);
  pinMode(ledPin7, OUTPUT);
  
  Serial.begin(57600);

  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);
}

// For this project, you don't need to change anything in the loop function. 
// Basically it ensures that you Arduino is connected to your broker
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ethClient");
}
