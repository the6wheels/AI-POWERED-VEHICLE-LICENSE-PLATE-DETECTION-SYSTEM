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

#define   MESH_PREFIX     "QS"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;




#define timeSeconds 10

// Set GPIOs PIR Motion Sensor
const int motionSensor = D6;

// define led according to pin diagram
int RED_LED = D0;
int YEL_LED = D1;


// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;
boolean motion = false;

// Checks if motion was detected, sets LED HIGH and starts a timer
void IRAM_ATTR detectsMovement() {
  digitalWrite(RED_LED, HIGH);
  startTimer = true;
  lastTrigger = millis();
}




// variables for the yellow LED
bool ledState_YEL = LOW;            // this holds the current state of the LED
unsigned long prevMillis_YEL = 0;   // this holds the start time of millis for computing the elapse time
unsigned long interval_YEL = 50;  // this holds the interval of each toggle of LED


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

void setup() {

    // Set the pin directions for LEDs
  pinMode(YEL_LED, OUTPUT);



    // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Set LED to LOW
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);


  Serial.begin(115200);

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




   // Current time
  now = millis();
  if((digitalRead(RED_LED) == HIGH) && (motion == false)) {
    Serial.println("MOTION DETECTED!!!");
    motion = true;
  }
  // Turn off the LED after the number of seconds defined in the timeSeconds variable
  if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
    Serial.println("Motion stopped...");
    digitalWrite(RED_LED, LOW);
    startTimer = false;
    motion = false;
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

