/*
Code for dummy actuator. It checks for a message from the leader
and turns on the LED for a few seconds
*/
#include <SPI.h>
 //Radio Head Library:
#include <RH_RF95.h>
#include <ArduinoJson.h>

#define PACKET_SIZE 128
#define FREQUENCY 915
#define LEADER_NODE 1

#define LED 4

// We need to provide the RFM95 module's chip select and interrupt pins to the
// rf95 instance below.On the SparkFun ProRF those pins are 12 and 6 respectively.
RH_RF95 rf95(12, 6);

bool receivePacket(){
  // Receive data from the leader
  StaticJsonDocument<200> doc;
  DeserializationError error;

  if (rf95.available()){
    uint8_t buf[PACKET_SIZE];
    uint8_t len = PACKET_SIZE;
    if (rf95.recv(buf, &len)){
        SerialUSB.println((char *)buf);
        error = deserializeJson(doc, buf);
        if(!error){
          SerialUSB.println("Received JSON data");
          if((int)doc["node_id"]==LEADER_NODE){
            SerialUSB.println("it's from leader!!!!!");
          }
          SerialUSB.println((int)doc["node_id"]);
          return true;
        }
      }else{
        SerialUSB.println("Recieve failed");
      }
  }
  return false;
}

void setupLora(){
    if (rf95.init() == false) {
        SerialUSB.println("Radio Init Failed - Freezing");
        while (1);
    } else {
        //An LED inidicator to let us know radio initialization has completed.    
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
    }
    // Set frequency
    rf95.setFrequency(FREQUENCY);
    // Transmitter power can range from 14-20dbm.
    rf95.setTxPower(20, false);
}


void setup() {
    SerialUSB.begin(9600);
    while(!SerialUSB);
    setupLora();
}

// The following 2 functions are there to keep led off and on for 5 minuites
int ledTurnedOnAt;
void turnOnLed(){
  digitalWrite(LED_BUILTIN, HIGH);
  ledTurnedOnAt = millis();
  
}
void turnOffLedIfTimeComplete(int timeLimit){
    if(millis() - ledTurnedOnAt > timeLimit){
      digitalWrite(LED_BUILTIN, LOW);
    }
}

void loop() {
  bool receivedCommand = receivePacket();
  if(receivedCommand){
    turnOnLed();
  }
  turnOffLedIfTimeComplete(5000);

}
