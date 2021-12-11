#include <SPI.h>
 //Radio Head Library:
#include <RH_RF95.h>
#include <TemperatureZero.h>
//Joshua Dotto
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 3      // Digital pin connected to the DHT sensor 
#define DHTTYPE DHT22 // DHT 22
DHT_Unified dht(DHTPIN, DHTTYPE);
// Joshua Dotto

#define PACKET_SIZE 28
#define WDT_FLAG 1<<17 // First bit of the 3rd byte
TemperatureZero TempZero = TemperatureZero();

// We need to provide the RFM95 module's chip select and interrupt pins to the
// rf95 instance below.On the SparkFun ProRF those pins are 12 and 6 respectively.
RH_RF95 rf95(12, 6);

int LED = 13; //Status LED is on pin 13

// Communication Gloabl Varaibles
int nodeId = 4;
int leaderNodeId = 2;
int nextPacketId =  0; // Counts the number of packets sent
float frequency = 915; // Broadcast frequency
unsigned long leaderOffset=1; // Millis time of leader - this node
bool timeSynced = false; // Initally not time synced

// raw sensor values struct

int gMoistPin = A0;
const float randomMoisture = 0.6;
int gLightPin = A1;

struct SensorValues{
  float temp;
  float humidity;
  float light;
  float moisture;
};

SensorValues getAverageSensorValue(int iterations = 5, int delayTime=200);

SensorValues s;

struct Payload {
  int nodeId;
  int packetId;
  int errorCode;
  unsigned long timeStamp;
  float temp;
  float light;
  float moisture;
  float humidity;
};


/* Functions for Communication Start*/

void createPacket(Payload payload,  uint8_t *packet) {
  memcpy(packet, &payload.nodeId, 4);
  memcpy(packet + 4, &payload.packetId, 4);
  memcpy(packet + 8, &payload.errorCode, 4);
  memcpy(packet + 12, &payload.temp, 4);
  memcpy(packet + 16, &payload.light, 4);
  memcpy(packet + 20, &payload.moisture, 4);
  memcpy(packet + 24, &payload.humidity, 4);
}

Payload parsePacket(uint8_t *packet) {
  Payload payload;
  memcpy(&payload.nodeId, packet, 4);
  memcpy(&payload.packetId, packet + 4, 4);
  memcpy(&payload.errorCode, packet + 8, 4);
  memcpy(&payload.timeStamp, packet + 12, 4);
  memcpy(&payload.light, packet + 16, 4);
  memcpy(&payload.moisture, packet + 20, 4);
  memcpy(&payload.humidity, packet + 24, 4);
  return payload;
}

void printPayload(Payload payload){    
    SerialUSB.print("Node ID = ");
    SerialUSB.print(payload.nodeId);
    SerialUSB.print(", ");

    SerialUSB.print("Packet ID = ");
    SerialUSB.print(payload.packetId);
    SerialUSB.print(", ");

    SerialUSB.print("Time Stamp = ");
    SerialUSB.print(payload.timeStamp);
    SerialUSB.print(", ");

    SerialUSB.print( "Light = ");
    SerialUSB.print(payload.light);
    SerialUSB.print(", ");

    SerialUSB.print( "Humidity = ");
    SerialUSB.print(payload.humidity);
    SerialUSB.print(", ");

    SerialUSB.print( "Moisture = ");
    SerialUSB.print(payload.moisture);
    SerialUSB.print(", ");

 
    SerialUSB.print("Temperature = ");
    SerialUSB.print(payload.temp);
    SerialUSB.print(", ");

    SerialUSB.print( "Error Code = ");
    SerialUSB.print(payload.errorCode);
    SerialUSB.print("\n");

}

void sendPacket(int errorCode, float temp, float light, 
                float moisture, float humidity) {                  
  Payload payload;
  uint8_t packet[PACKET_SIZE];

  payload.nodeId = nodeId;
  payload.packetId = nextPacketId++;
  payload.temp = temp; // TODO Populate it correctly
  payload.errorCode = errorCode; // Zero for no error code
  payload.light = light;
  payload.moisture = moisture;
  payload.humidity = humidity;

  SerialUSB.println("Sending Message with");
  printPayload(payload);
   
  createPacket(payload, packet);
  
  rf95.send((uint8_t * ) packet, sizeof(packet));
}

Payload receivePacket(){  
  Payload payload;
  payload.nodeId=-1;
  
  if (rf95.available()){
    uint8_t buf[PACKET_SIZE];
    uint8_t len = PACKET_SIZE;
    if (rf95.recv(buf, &len)){
        SerialUSB.print("buf: ");
        SerialUSB.println((char *)buf);
        digitalWrite(LED, HIGH); //Turn on status LED
        payload = parsePacket(buf);
        digitalWrite(LED, LOW); //Turn off status LED
      }else{
        SerialUSB.println("Recieve failed");
      }
  }
  return payload;
}

void setupLora(){
    pinMode(LED, OUTPUT);
    if (rf95.init() == false) {
        SerialUSB.println("Radio Init Failed - Freezing");
        while (1);
    } else {
        //An LED inidicator to let us know radio initialization has completed.    
        digitalWrite(LED, HIGH);
        delay(500);
        digitalWrite(LED, LOW);
    }
    // Set frequency
    rf95.setFrequency(frequency);
    // Transmitter power can range from 14-20dbm.
    rf95.setTxPower(20, false);
}


/* Function for Error code start*/

void setupWDT() {
  // Generic clock generator 2, divisor = 32 (2^(DIV+1))
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(4);
  // Enable clock generator 2 using low-power 32KHz oscillator.
  // With /32 divisor above, this yields 1024Hz(ish) clock.
  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) |
                      GCLK_GENCTRL_GENEN |
                      GCLK_GENCTRL_SRC_OSCULP32K |
                      GCLK_GENCTRL_DIVSEL;
  while(GCLK->STATUS.bit.SYNCBUSY);
  // WDT clock = clock gen 2
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_WDT |
                      GCLK_CLKCTRL_CLKEN |
                      GCLK_CLKCTRL_GEN_GCLK2;

  // Enable WDT early-warning interrupt
  NVIC_DisableIRQ(WDT_IRQn);
  NVIC_ClearPendingIRQ(WDT_IRQn);
  NVIC_SetPriority(WDT_IRQn, 0);
  NVIC_EnableIRQ(WDT_IRQn);

  WDT->INTENSET.bit.EW   = 1;      // Enable early warning interrupt
  WDT->EWCTRL.bit.EWOFFSET = 0x8;  // Early Warning Interrupt Time Offset
  WDT->CONFIG.bit.PER    = 0x9;   // Set period for chip reset 
  WDT->CTRL.bit.WEN      = 0;      // Disable window mode
  while(WDT->STATUS.bit.SYNCBUSY); // Sync CTRL write
  WDT->CTRL.bit.ENABLE = 1; // Start watchdog now!
  while(WDT->STATUS.bit.SYNCBUSY);
}


int getErrorCode(bool wdt_interrupt){
  int errorCode =  REG_DSU_STATUSA|REG_DSU_STATUSB<<8;
  if(wdt_interrupt){
      errorCode=errorCode|WDT_FLAG;
  }
  return errorCode;
}

void WDT_Handler(void) {  // ISR for watchdog early warning, DO NOT RENAME!
  SerialUSB.println("WDT_Handler");
  //sendPacket(TempZero.readInternalTemperature(), getErrorCode(true));
  sendPacket(getErrorCode(true), s.temp, s.light, s.moisture, s.humidity);
  WDT->CLEAR.reg = 0xFF;
}


/* 1s interrupt handler*/
void setup() {
    SerialUSB.begin(9600);
    SerialUSB.println("RFM Client!");
    SerialUSB.println("Transmitter up!");
    dht.begin();
    setupLora();
    setupWDT();
}
 
void loop() {
  // Keep checking for a time sync signal until it is received
  s = getSensorData();
  delay(2000);
  sendPacket(0, s.temp, s.light, s.moisture, s.humidity);
  WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;

  /*if(!timeSynced){
     WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
     timeSynced = syncTimeWithLeader();
     return;
  }*/
}


// Joshua Dotto

SensorValues getSensorData() {
  sensors_event_t eventTemp;
 
  sensors_event_t eventHumid;

  SensorValues sensorValue;

  dht.temperature().getEvent(&eventTemp);

  dht.humidity().getEvent(&eventHumid);


  sensorValue.temp = eventTemp.temperature;

  sensorValue.humidity = eventHumid.relative_humidity;

  sensorValue.light = (100.0/1023)*(analogRead(gLightPin));

  sensorValue.moisture = 0.1011*(analogRead(gMoistPin))-1.1374;
  return sensorValue;
}
