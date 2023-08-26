
//************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 2. prints anything it receives to Serial.print
//
//
//************************************************************
#include "painlessMesh.h"
//#include "TaskScheduler.h"
#include "Arduino.h"
#include <humanstaticLite.h>
#include <HardwareSerial.h>


#define   MESH_PREFIX     "QS"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555



Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;



// define led according to pin diagram
int RED_LED = D1;
int YEL_LED = D0;


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





#define timeSeconds 10






// variables for the yellow LED
bool ledState_YEL = LOW;            // this holds the current state of the LED
unsigned long prevMillis_YEL = 0;   // this holds the start time of millis for computing the elapse time
unsigned long interval_YEL = 5000;  // this holds the interval of each toggle of LED


unsigned long currMillis_ALL = 0;   // this holds the current time captured by the millis() function
                                    // it is use in general for synchronization purpose




// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  String msg = "Hello from node PIR sensor node ID = ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}


/*   Multiple Serial test for XIAO ESP32C3 */
HardwareSerial mySerial(0);   //Create a new HardwareSerial class.



// we'll be using software serial
HumanStaticLite radar = HumanStaticLite(&mySerial);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  mySerial.begin(115200);

  while(!Serial);   //When the serial port is opened, the program starts to execute.

  Serial.println("Ready");
  
  
      // Set the pin directions for LEDs
  pinMode(YEL_LED, OUTPUT);



  // Set LED to LOW
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);



//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
  
}

void loop() {
	
	
	  // it will run the user scheduler as well
  mesh.update();

  // Store the currently time
  // Notice: it use a single current time for synchonization with other function
  currMillis_ALL = millis();

  // Virtually process all the task
  process_YEL();
  
  // put your main code here, to run repeatedly:
  radar.HumanStatic_func(true);    //Turn on printing of human movement sign parameters
  if(radar.radarStatus != 0x00){
    switch(radar.radarStatus){
      Serial.println(radar.radarStatus);
      case SOMEONE:
        Serial.println("Someone is here.");
        Serial.println("---------------------------------");
         // Turn off the LED after the number of seconds defined in the timeSeconds variable
          digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
        break;
      case NOONE:
        Serial.println("Nobody here.");
        Serial.println("---------------------------------");
         // Turn off the LED after the number of seconds defined in the timeSeconds variable
          digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
        break;
      case NOTHING:
        Serial.println("No human activity messages.");
        Serial.println("---------------------------------");
         // Turn off the LED after the number of seconds defined in the timeSeconds variable
          digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
        break;
      case SOMEONE_STOP:
        Serial.println("Someone stop");
        Serial.println("---------------------------------");
         // Turn off the LED after the number of seconds defined in the timeSeconds variable
          digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
        break;
      case SOMEONE_MOVE:
        Serial.println("Someone moving");
        Serial.println("---------------------------------");
         // Turn off the LED after the number of seconds defined in the timeSeconds variable
          digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
        break;
      case HUMANPARA:
        Serial.print("The parameters of human body signs are: ");
        Serial.println(radar.bodysign_val, DEC);
        Serial.println("---------------------------------");
         // Turn off the LED after the number of seconds defined in the timeSeconds variable
          digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
        break;
      case SOMEONE_CLOSE:
        Serial.println("Someone is closing");
        Serial.println("---------------------------------");
         // Turn off the LED after the number of seconds defined in the timeSeconds variable
          digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
        break;
      case SOMEONE_AWAY:
        Serial.println("Someone is staying away");
        Serial.println("---------------------------------");
         // Turn off the LED after the number of seconds defined in the timeSeconds variable
          digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
        break;
      case DETAILMESSAGE:
        Serial.print("Spatial static values: ");
         // Turn off the LED after the number of seconds defined in the timeSeconds variable
          digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
        Serial.println(radar.static_val);
        Serial.print("Distance to stationary object: ");
         // Turn off the LED after the number of seconds defined in the timeSeconds variable
          digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
        Serial.print(radar.dis_static);
        Serial.println(" m");
         // Turn off the LED after the number of seconds defined in the timeSeconds variable
          digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }

        Serial.print("Spatial dynamic values: ");
        Serial.println(radar.dynamic_val);
         // Turn off the LED after the number of seconds defined in the timeSeconds variable
          digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }

        Serial.print("Distance from the movement object: ");
        Serial.print(radar.dis_move);
        Serial.println(" m");
         // Turn off the LED after the number of seconds defined in the timeSeconds variable
          digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
        
        Serial.print("Speed of moving object: ");
        Serial.print(radar.speed);
        Serial.println(" m/s");
        Serial.println("---------------------------------");
         // Turn off the LED after the number of seconds defined in the timeSeconds variable
          digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
        break;
    }
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

