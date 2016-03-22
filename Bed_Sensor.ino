//Push 2
// Include external libraries
//#include <SPI.h>
#include <WiFi.h>
#include <WifiIPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>
#include <BMA222.h>

WiFiClient client1;
char server[] = "miniproject.eu-gb.mybluemix.net";
// Wireless network parameters
char ssid[] = "WIFI";          // Your wireless network name also called SSID
char password[] = "PASSWORD";       // Your wireless network password
unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 1L * 1000L;
// IBM IoT Foundation Cloud Settings
/* When adding a device on internetofthings.ibmcloud.com the following information will be generated:
    org=<org>
    type=iotsample-ti-energia
    id=<mac>
    auth-method=token
    auth-token=<password>
*/

#define MQTT_MAX_PACKET_SIZE 4096       // Maximum length of MQTT message in bytes
#define IBMSERVERURLLEN  64
#define IBMIOTFSERVERSUFFIX "messaging.internetofthings.ibmcloud.com"

char organization[] = "l7lvu1";     // Your BlueMix Organisation ID
char typeId[] = "sensor";            // Type of device
char pubtopic[] = "iot-2/evt/status/fmt/json";    // MQTT publication topic
char subTopic[] = "iot-2/cmd/+/fmt/json";         // MQTT subscribe topic
char deviceId[] = "d4f51303ee72";//"d4f51303f020";//"20F0313F5D4";                 // Unique device identifier typically the MAC address of the IoT device
char clientId[64];

char mqttAddr[IBMSERVERURLLEN];
int mqttPort = 1883;                              // Port for MQTT connection

// Authentication method. Should be use-token-auth when using authenticated mode
char authMethod[] = "use-token-auth";          
char authToken[] = "ofPcD9N!+R8xDF@&BD";          // The authentication token generated by your BlueMix application

//const char MQTTSTRING[] = "{\"d\":{\"myName\":\"TILaunchPad\",\"temperature\":%.2f,\"ambient\":%.2f,\"X\":%i,\"Y\":%i,\"Z\":%i,\"RSSI\":%i}}";
const char MQTTSTRING[] = "{\"d\":{\"Xvalue\":%i}}";
// Create MAC address and WiFiIPStack objects
MACAddress mac;
  
WifiIPStack ipstack;  
MQTT::Client<WifiIPStack, Countdown, MQTT_MAX_PACKET_SIZE> client(ipstack);

// Define external sensors, inputs and outputs.
#define LEDPIN RED_LED       // Red LED on CC3200 Launchpad
BMA222 accSensor;            // Three axis acceleration sensor

#include <Wire.h>
#include "Adafruit_TMP006.h"
Adafruit_TMP006 tmp006(0x41);

// Function prototypes to call when a message arrives
void callback(char* topic, byte* payload, unsigned int length);
void messageArrived(MQTT::MessageData& md);

 int n = 0;


// Setup function runs once when microprocessor is powered up
void setup() {
  uint8_t macOctets[6];
  
  Serial.begin(115200);          //Initialise serial port for local monitoring on the Serail Monitor via USB
  Serial.print("Attempting to connect to Network named: ");
  pinMode(11, INPUT);
  Serial.println(ssid);
  
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print(".");         // print dots while we wait to connect
    delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while (WiFi.localIP() == INADDR_NONE) {
    Serial.print(".");        // print dots while we wait for an ip addresss
    delay(300);
  }

  // We are connected and have an IP address.
  Serial.print("\nIP Address obtained: ");
  Serial.println(WiFi.localIP());

  mac = WiFi.macAddress(macOctets);
  Serial.print("MAC Address: ");
  Serial.println(mac);
  
  // Use MAC Address as deviceId
  sprintf(deviceId, "%02x%02x%02x%02x%02x%02x", macOctets[0], macOctets[1], macOctets[2], macOctets[3], macOctets[4], macOctets[5]);
  Serial.print("deviceId: ");
  Serial.println(deviceId);

  sprintf(clientId, "d:%s:%s:%s", organization, typeId, deviceId);
  sprintf(mqttAddr, "%s.%s", organization, IBMIOTFSERVERSUFFIX);

  if (!tmp006.begin()) {
    Serial.println("No sensor found");
    //while (1);
  }
  
  // start the accel sensor
  accSensor.begin();
  uint8_t chipID = accSensor.chipID();
  Serial.print("ChipID: ");
  Serial.println(chipID);
  

}

// Main loop. Runs continuously
void sendValue(int value) {
  int rc = -1;
  
  // If the MQTT service is not connected then open connection
  if (!client.isConnected()) {
    Serial.print("Connecting to ");
    Serial.print(mqttAddr);
    Serial.print(":");
    Serial.println(mqttPort);
    Serial.print("With client id: ");
    Serial.println(clientId);
    
    while (rc != 0) {
      rc = ipstack.connect(mqttAddr, mqttPort);
    }

    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    connectData.MQTTVersion = 3.1;
    connectData.clientID.cstring = clientId;
    connectData.username.cstring = authMethod;
    connectData.password.cstring = authToken;
    connectData.keepAliveInterval = 10;
    
    rc = -1;
    while ((rc = client.connect(connectData)) != 0);
    Serial.println("Connected\n");
    
    Serial.print("Subscribing to topic: ");
    Serial.println(subTopic);
    
    // Unsubscribe the topic, if it had subscribed it before.
    client.unsubscribe(subTopic);
    // Try to subscribe for commands from IBM Bluemix MQTT cloud server
    // messageArrived - function that is called when a MQTT message is received
    rc = client.subscribe(subTopic, MQTT::QOS0, messageArrived);
    if (rc != 0) {
      Serial.print("Subscribe failed with return code : ");
      Serial.println(rc);
    } else {
      Serial.println("Subscribe success\n");
    }
  }

// Poll Sensors and Publish JSON data to MQTT service
  Serial.print("Publishing: ");
  Serial.print(n++);
  char string[64]; // = aJson.print(json);   // Convert JSON data to MQTT string
  sprintf(string, MQTTSTRING, value);
  Serial.println(string);             // Print MQTT string to serial terminal
  
  // Create MQTT message
  MQTT::Message message;              
  message.qos = MQTT::QOS0; 
  message.retained = false;
  message.payload = string; 
  message.payloadlen = strlen(string);
  // Publish MQTT message
  Serial.println("Got to end of messages");
  rc = client.publish(pubtopic, message);
  Serial.println("Published");
  if (rc != 0) {
    Serial.print("Message publish failed with return code : ");
    Serial.println(rc);
  }
  
  // Wait for one second before publishing again
  // This will also service any incoming messages
  client.yield(2000);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message has arrived");
  
  char * msg = (char *)malloc(length * sizeof(char));   // Allocate memory for incoming message
  int count = 0;
  for(count = 0 ; count < length ; count++) {
    msg[count] = payload[count];
  }
  msg[count] = '\0';
  Serial.println(msg);
  
  if(length > 0) {
    digitalWrite(LEDPIN, HIGH);
    delay(1000);
    digitalWrite(LEDPIN, LOW);  
  }

  free(msg);                     //Free up memory allocated to message
}

void messageArrived(MQTT::MessageData& md) {
  Serial.print("Message Received\t");
    MQTT::Message &message = md.message;
    int topicLen = strlen(md.topicName.lenstring.data) + 1;
//    char* topic = new char[topicLen];
    char * topic = (char *)malloc(topicLen * sizeof(char));
    topic = md.topicName.lenstring.data;
    topic[topicLen] = '\0';
    
    int payloadLen = message.payloadlen + 1;
//    char* payload = new char[payloadLen];
    char * payload = (char*)message.payload;
    payload[payloadLen] = '\0';
    
    String topicStr = topic;
    String payloadStr = payload;
    
    //Command topic: iot-2/cmd/blink/fmt/json

    if(strstr(topic, "/cmd/blink") != NULL) {
      Serial.print("Command IS Supported : ");
      Serial.print(payload);
      Serial.println("\t.....");
      
      pinMode(LEDPIN, OUTPUT);
      
      //Blink twice
      for(int i = 0 ; i < 2 ; i++ ) {
        digitalWrite(LEDPIN, HIGH);
        delay(250);
        digitalWrite(LEDPIN, LOW);
        delay(250);
      }
    } else {
      Serial.println("Command Not Supported:");            
    }
}

int getStatus(){
  WiFiClient client1;
  char server[] = "miniproject.eu-gb.mybluemix.net";
  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client1.connect(server, 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client1.println("GET /test HTTP/1.0");
    client1.println("Host: miniproject.eu-gb.mybluemix.net");
    client1.println("Connection: close");
    client1.println();
  }
  char c;
  while (client1.available()) {
    c = client1.read();
    Serial.write(c);
  }
  Serial.println((char)c);
  Serial.println(c=='0');
  // if the server's disconnected, stop the client:
  if (!client1.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client1.stop();

    // do nothing forevermore:
  }
  return (c=='0');

}

int x=0;
int first=0;
void loop(){
  if (x==0){
  /*while (getStatus()){
    delay(500);
  }
  sendValue(accSensor.readXData());
  delay(1000);*/
  char c;
  while (client1.available()) {
    c = client1.read();
    Serial.write(c);
  }
  Serial.print("\nThe value is");
  Serial.println(c=='0');
  if (c=='1') x=1;
  else if(c=='2') x=2;
  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (x==0 && millis() - lastConnectionTime > postingInterval) {
    httpRequest();
    delay(1040);
  }
 
}
  else if(x==2) {
  int isPressed = digitalRead(11);
  Serial.print("Is pressed: ");
  Serial.println(isPressed);
  if(!isPressed)
  {
    Serial.println("Turn off alarm");
    sendValue(0);
    while(1);
  }
  delay(500);
}
  else x=0;
}
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client1.stop();

  // if there's a successful connection:
  if (client1.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client1.println("GET /test HTTP/1.0");
    client1.println("Host: miniproject.eu-gb.mybluemix.net");
    client1.println("User-Agent: ArduinoWiFi/1.1");
    client1.println("Connection: close");
    client1.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}





