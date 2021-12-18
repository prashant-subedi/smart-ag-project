/*
  Code for gateway upload into SAMD21. The SAMD21 is connected via USB to a PI
  The SAMD21 just listens for data from SerialUSB and transmits it
  and listens to LoRA messages and puts it into SerialUSB
*/
#include <SPI.h>
 //Radio Head Library:
#include <RH_RF95.h>

#define PACKET_SIZE 128
#define FREQUENCY 915

// We need to provide the RFM95 module's chip select and interrupt pins to the
// rf95 instance below.On the SparkFun ProRF those pins are 12 and 6 respectively.
RH_RF95 rf95(12, 6);

void receivePacket(){
  // Any packet received from LoRA is dumped into SerialUSB
  if (rf95.available()){
    uint8_t buf[PACKET_SIZE];
    uint8_t len = PACKET_SIZE;
    if (rf95.recv(buf, &len)){
        SerialUSB.println((char *)buf);
      }else{
        SerialUSB.println("Recieve failed");
      }
  }
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

String gSerialMessage;
void loop() {
  receivePacket();
  if(SerialUSB.available() > 0){
    gSerialMessage = SerialUSB.readString();
    rf95.send((uint8_t * )gSerialMessage.c_str(), gSerialMessage.length());
  }
}
