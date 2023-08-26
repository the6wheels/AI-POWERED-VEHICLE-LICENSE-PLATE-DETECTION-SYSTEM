//************************************************************
// this is a MqttBroker example that uses the painlessMesh library
//
// connect to a another network and relay messages from a MQTT broker to the nodes of the mesh network.
//
// - To send a message to a mesh node, you can publish it to "painlessMesh/to/NNNN" where NNNN equals the nodeId.
// - To broadcast a message to all nodes in the mesh you can publish it to "painlessMesh/to/broadcast".
// - When you publish "getNodes" to "painlessMesh/to/gateway" you receive the mesh topology as JSON
//
// - Every message from the mesh which is sent to the gateway node will be published to "painlessMesh/from/NNNN" where NNNN
//   is the nodeId from which the packet was sent.
//
//
//************************************************************
//#include <Arduino.h>
#include "painlessMesh.h"
#include "PubSubClient.h"
#include "DHT.h"
#define DHTPIN 6     // Digital pin connected to the DHT sensor
 
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// PainlessMesh credentials ( name, password and port ): You should change these
#define   MESH_PREFIX     "QS"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// WiFi credentials: should match your access point!
#define   STATION_SSID     "Base_Station_wifi"
#define   STATION_PASSWORD "ilikepotato"

#define   HOSTNAME         "MQTT_Bridge"


// define led according to pin diagram
int RED_LED = D1;
int YEL_LED = D0;

#define timeSeconds 10

// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;


// Checks if motion was detected, sets LED HIGH and starts a timer
void IRAM_ATTR detectsMovement() {
  digitalWrite(RED_LED, HIGH);
  startTimer = true;
  lastTrigger = millis();
}



// variables for the yellow LED
bool ledState_YEL = LOW;            // this holds the current state of the LED
unsigned long prevMillis_YEL = 0;   // this holds the start time of millis for computing the elapse time
unsigned long interval_YEL = 5000;  // this holds the interval of each toggle of LED


unsigned long currMillis_ALL = 0;   // this holds the current time captured by the millis() function
                                    // it is use in general for synchronization purpose
                                    
                                    
                                    
Scheduler userScheduler;   // to control your personal task

painlessMesh  mesh;
WiFiClient wifiClient;

// Prototypes
void receivedCallback( const uint32_t &from, const String &msg );
void mqttCallback(char* topic, byte* payload, unsigned int length);

IPAddress getlocalIP();
IPAddress myIP(0,0,0,0);

// hivemq pubblic broker address and port
char mqttBroker[]  = "10.42.0.1";
#define MQTTPORT 1883

// topic's suffix: everyone can publish/subscribe to this public broker,
// you have to change the following 2 defines
#define PUBPLISHSUFFIX             "painlessMesh/from/"
#define SUBSCRIBESUFFIX            "painlessMesh/to/"

#define PUBPLISHFROMGATEWAYSUFFIX  PUBPLISHSUFFIX "gateway"

#define CHECKCONNDELTA 60     // check interval ( seconds ) for mqtt connection

PubSubClient mqttClient;



bool calc_delay = false;
SimpleList<uint32_t> nodes;
uint32_t nsent=0;
char buff[512];
uint32_t nexttime=0;
uint8_t  initialized=0;


//Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );



// messages received from the mqtt broker
void mqttCallback(char* topic, uint8_t* payload, unsigned int length) 
  {
  char* cleanPayload = (char*)malloc(length+1);
  payload[length] = '\0';
  memcpy(cleanPayload, payload, length+1);
  String msg = String(cleanPayload);
  free(cleanPayload);

  Serial.printf("mc t:%s  p:%s\n", topic, payload);
  
  String targetStr = String(topic).substring(strlen(SUBSCRIBESUFFIX));
  if(targetStr == "gateway")
    {
    if(msg == "getNodes")
      {
      auto nodes = mesh.getNodeList(true);
      String str;
      for (auto &&id : nodes)
        str += String(id) + String(" ");
      mqttClient.publish(PUBPLISHFROMGATEWAYSUFFIX, str.c_str());
      }
    }
  else if(targetStr == "broadcast") 
    {
    mesh.sendBroadcast(msg);
    }
  else
    {
    uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    if(mesh.isConnected(target))
      {
      mesh.sendSingle(target, msg);
      }
    else
      {
      mqttClient.publish(PUBPLISHFROMGATEWAYSUFFIX, "Client not connected!");
      }
    }
  }





// Needed for painless library

// messages received from painless mesh network
void receivedCallback( const uint32_t &from, const String &msg ) 
  {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  String topic = PUBPLISHSUFFIX + String(from);
  mqttClient.publish(topic.c_str(), msg.c_str());
  }




void newConnectionCallback(uint32_t nodeId) 
  {
  Serial.printf("--> Start: New Connection, nodeId = %u\n", nodeId);
  Serial.printf("--> Start: New Connection, %s\n", mesh.subConnectionJson(true).c_str());
  }




void changedConnectionCallback() 
  {
  Serial.printf("Changed connections\n");

  nodes = mesh.getNodeList();
  Serial.printf("Num nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");
  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) 
    {
    Serial.printf(" %u", *node);
    node++;
    }
  Serial.println();
  calc_delay = true;

  sprintf(buff,"Nodes:%d",nodes.size());
  }




void nodeTimeAdjustedCallback(int32_t offset) 
  {
  Serial.printf("Adjusted time %u Offset = %d\n", mesh.getNodeTime(),offset);
  }




void onNodeDelayReceived(uint32_t nodeId, int32_t delay)
  {
  Serial.printf("Delay from node:%u delay = %d\n", nodeId,delay);
  }




void reconnect()
{
  //byte mac[6];
  char MAC[9];
  int i;

  // unique string
  //WiFi.macAddress(mac);
  //sprintf(MAC,"%02X",mac[2],mac[3],mac[4],mac[5]);
  sprintf(MAC, "%08X",(uint32_t)ESP.getEfuseMac());  // generate unique addresss.
  // Loop until we're reconnected
  while (!mqttClient.connected()) 
    {
    Serial.println("Attempting MQTT connection...");    
    // Attemp to connect
    if (mqttClient.connect(/*MQTT_CLIENT_NAME*/MAC)) 
      {
      Serial.println("Connected");  
      mqttClient.publish(PUBPLISHFROMGATEWAYSUFFIX,"Ready!");
      mqttClient.subscribe(SUBSCRIBESUFFIX "#");
      } 
    else
      {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
      mesh.update();
      mqttClient.loop();
      }
    }
}






IPAddress getlocalIP() 
  {
  return IPAddress(mesh.getStationIP());
  }





void setup() 
  {

      // Set LED to LOW
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);

    // Set the pin directions for LEDs
  pinMode(YEL_LED, OUTPUT);
  
  Serial.begin(115200);
  
  

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | MSG_TYPES | REMOTE ); // all types on except GENERAL
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  // Channel set to 1. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);

  // if you want your node to accept OTA firmware, simply include this line
  // with whatever role you want your hardware to be. For instance, a
  // mesh network may have a thermometer, rain detector, and bridge. Each of
  // those may require different firmware, so different roles are preferrable.
  //
  // MAKE SURE YOUR UPLOADED OTA FIRMWARE INCLUDES OTA SUPPORT OR YOU WILL LOSE
  // THE ABILITY TO UPLOAD MORE FIRMWARE OVER OTA. YOU ALSO WANT TO MAKE SURE
  // THE ROLES ARE CORRECT
  mesh.initOTAReceive("bridge");

  sprintf(buff,"Id:%d",mesh.getNodeId());
  
  mqttClient.setServer(mqttBroker, MQTTPORT);
  mqttClient.setCallback(mqttCallback);  
  mqttClient.setClient(wifiClient);
  
      Serial.println(F("DHTxx test!"));
      dht.begin();
          if(startTimer && (now - lastTrigger > (timeSeconds*100))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
    
  }






void loop() 
  {
	    float h = dht.readHumidity();
  float t = dht.readTemperature();
 
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.println("%");
  Serial.print(F("Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
  Serial.println("");
   if(startTimer && (now - lastTrigger > (timeSeconds*100))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
  
  
  
	    // Store the currently time
  // Notice: it use a single current time for synchonization with other function
  currMillis_ALL = millis();
  
    // Virtually process all the task
  process_YEL();
  
  
  // it will run the user scheduler as well
  mesh.update();
  mqttClient.loop();

  if(myIP != getlocalIP())
    {
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
    initialized = 1;
    }
  if ( ( millis() >= nexttime ) && ( initialized ) )
    {
    nexttime=millis()+CHECKCONNDELTA*1000;
    if (!mqttClient.connected()) 
      {reconnect();}
    }
  }





void process_YEL() {
  if (currMillis_ALL - prevMillis_YEL >= interval_YEL) {
    // Elapse time has reached the interval
    // Save the current time as previous time
    prevMillis_YEL = currMillis_ALL;

    // toggle LED state
    ledState_YEL = !ledState_YEL;
    
    digitalWrite(YEL_LED, ledState_YEL);
  }
}
