#include "Arduino.h"
#include <HardwareSerial.h>
#include "painlessMesh.h"


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
HardwareSerial MySerial(0);   //Create a new HardwareSerial class.

void setup() {
  // initialize both serial ports:
  Serial.begin(115200);
  MySerial.begin(115200, SERIAL_8N1, RX, TX); // at CPU Freq is 40MHz, work half speed of defined.

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
	
	
  // read from port 1, send to port 0:
  if (MySerial.available()) {
    String str  = MySerial.readStringUntil('\r');
    Serial.println(str);
              digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
        }
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    String str = Serial.readStringUntil('\r');
    MySerial.println(str);
              digitalWrite(RED_LED, HIGH);
        if(startTimer && (now - lastTrigger > (timeSeconds*10))) {
          digitalWrite(RED_LED, LOW);
          startTimer = false;
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
