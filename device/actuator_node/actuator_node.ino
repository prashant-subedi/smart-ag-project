#include <SPI.h>
 //Radio Head Library:
#include <RH_RF95.h>
#include <ArduinoJson.h>

#define PACKET_SIZE 128
#define FREQUENCY 915
#define LEADER_NODE 1

RH_RF95 rf95(12, 6);

bool receivePacket(){
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
