// DTH Sensor dependencies
#include <SPI.h>
#include <DHT.h>
#include <DHT_U.h>

//Radio Head Library:
#include <RH_RF95.h>

#include <ArduinoJson.h>

// Initialize DHT Sensor
#define DHTPIN 3 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22 // DHT 22
DHT_Unified dht(DHTPIN, DHTTYPE);

#define WDT_FLAG 1 << 17 // First bit of the 3rd byte

#define PACKET_SIZE 150
#define MAX_RETRY 4
#define WAIT_INTERVALS 300000

// We need to provide the RFM95 module's chip select and interrupt pins to the
// rf95 instance below.On the SparkFun ProRF those pins are 12 and 6 respectively.
RH_RF95 rf95(12, 6);

int gLed = 13; //Status LED is on pin 13

// Communication Gloabl Varaibles
int gNodeId = 4;
int gNextPacketId = 0; // Counts the number of packets sent
float gFrequency = 915; // Broadcast frequency
int gPacketSize = 128;

// raw sensor values struct
int gMoistPin = A0;
const float randomMoisture = 0.6;
int gLightPin = A1;

struct SensorValues {
    float temp;
    float humidity;
    float light;
    float moisture;
};

int gWaitedFor= 50000; // Large Value so transmission can begin first
int gWaitStartedAt = 0;

/* Functions for Communication Start*/
void createPacket(StaticJsonDocument<PACKET_SIZE> jsonPayload, char* packet)
{
    serializeJson(jsonPayload, packet, PACKET_SIZE);
}

StaticJsonDocument<PACKET_SIZE> parsePacket(char* packet)
{
    StaticJsonDocument<PACKET_SIZE> jsonPayload;
    deserializeJson(jsonPayload, packet);
    return jsonPayload;
}

void sendPacket(int errorCode, float temp, float light,
    float moisture, float humidity, bool mockRainPrediction)
{
    char packet[PACKET_SIZE];
    StaticJsonDocument<PACKET_SIZE> jsonPayload;
    int retryCount = 0;
    int randomWait;
    int ackRetries;

    jsonPayload["node_id"] = gNodeId;
    jsonPayload["packet_id"] = gNextPacketId++;
    jsonPayload["error_code"] = errorCode;
    jsonPayload["temp"] = temp;
    jsonPayload["light"] = light;
    jsonPayload["moisture"] = moisture;
    jsonPayload["humidity"] = humidity;
    jsonPayload["mock_rain"] = mockRainPrediction;

    createPacket(jsonPayload, packet);
    SerialUSB.println(packet);

    while (1) {
        // The delay is there before 1st send to sovlve some wierd issues
        // The ack was never received
        randomWait = rand() % 1000;
        SerialUSB.print("Waiting for: ");
        SerialUSB.println(randomWait);

        ackRetries = 10000;
        delay(randomWait);
        
        rf95.send((uint8_t*)packet, sizeof(packet));
        rf95.waitPacketSent();
        SerialUSB.println(packet);
        
        while (ackRetries > 0) {
            if (ackRecevied()) {
                SerialUSB.println("ACK REVEIVED!!!");
                return;
            }
            ackRetries-=1;
        }
        SerialUSB.println("ACK NOT RECEIVED");
        retryCount+=1;      
        if(retryCount > MAX_RETRY){ 
          SerialUSB.println("Stopping retransmission");
          break;
        }
    }
    SerialUSB.println("Transmission Failed");
  }

bool ackRecevied()
{
     StaticJsonDocument<PACKET_SIZE> receivedJson;
     DeserializationError error;
    if (rf95.available()) {
        uint8_t buf[PACKET_SIZE];
        uint8_t len = PACKET_SIZE;
        if (rf95.recv(buf, &len)) {
            // Verifying this results in missing ACK :( 
            // So for now, leaving this as it is.
        }
        else {
            return false;
        }
    }
}

void setupLora()
{
    if (rf95.init() == false) {
        SerialUSB.println("Radio Init Failed - Freezing");
        while (1)
            ;
    }
    else {
        //An LED inidicator to let us know radio initialization has completed.
        digitalWrite(gLed, HIGH);
        delay(500);
        digitalWrite(gLed, LOW);
    }
    // Set frequency
    rf95.setFrequency(gFrequency);
    // Transmitter power can range from 14-20dbm.
    rf95.setTxPower(20, false);
}

/* Function for Error code start*/

void setupWDT()
{
    // Generic clock generator 2, divisor = 32 (2^(DIV+1))
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(4);
    // Enable clock generator 2 using low-power 32KHz oscillator.
    // With /32 divisor above, this yields 1024Hz(ish) clock.
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_DIVSEL;
    while (GCLK->STATUS.bit.SYNCBUSY)
        ;
    // WDT clock = clock gen 2
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_WDT | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2;

    // Enable WDT early-warning interrupt
    NVIC_DisableIRQ(WDT_IRQn);
    NVIC_ClearPendingIRQ(WDT_IRQn);
    NVIC_SetPriority(WDT_IRQn, 0);
    NVIC_EnableIRQ(WDT_IRQn);

    WDT->INTENSET.bit.EW = 1; // Enable early warning interrupt
    WDT->EWCTRL.bit.EWOFFSET = 0x8; // Early Warning Interrupt Time Offset
    WDT->CONFIG.bit.PER = 0x9; // Set period for chip reset
    WDT->CTRL.bit.WEN = 0; // Disable window mode
    while (WDT->STATUS.bit.SYNCBUSY)
        ; // Sync CTRL write
    WDT->CTRL.bit.ENABLE = 1; // Start watchdog now!
    while (WDT->STATUS.bit.SYNCBUSY)
        ;
}

int getErrorCode(bool wdt_interrupt)
{
    int errorCode = REG_DSU_STATUSA | REG_DSU_STATUSB << 8;
    if (wdt_interrupt) {
        errorCode = errorCode | WDT_FLAG;
    }
    return errorCode;
}

void WDT_Handler(void)
{ // ISR for watchdog early warning, DO NOT RENAME!
    SerialUSB.println("WDT_Handler");
    WDT->CLEAR.reg = 0xFF;
}

// Joshua Dotto
SensorValues getSensorData()
{
    sensors_event_t eventTemp;

    sensors_event_t eventHumid;

    SensorValues sensorValue;

    dht.temperature().getEvent(&eventTemp);

    dht.humidity().getEvent(&eventHumid);

    sensorValue.temp = eventTemp.temperature;

    sensorValue.humidity = eventHumid.relative_humidity;

    sensorValue.light = (100.0 / 1023) * (analogRead(gLightPin));

    sensorValue.moisture = 0.1011 * (analogRead(gMoistPin)) - 1.1374;
    return sensorValue;
}

/* 1s interrupt handler*/
void setup()
{
    pinMode(gLed, OUTPUT);
    SerialUSB.begin(9600);
    SerialUSB.println("RFM Client!");
    SerialUSB.println("Transmitter up!");
    dht.begin();
    setupLora();
//    setupWDT();
}

void loop()
{
    // Keep checking for a time sync signal until it is received
    boolean triggered = false;
    boolean mockRainPrediction = false;
    String str;
    if(SerialUSB.available() > 0){
        str = SerialUSB.readString();
        SerialUSB.println(str);
        
        if (str.startsWith("rain")){
          mockRainPrediction = true;
        }
        SerialUSB.println("Manually triggered sending");
        triggered = true;
        
    }
    if( gWaitedFor < WAIT_INTERVALS && !triggered){
      gWaitedFor = millis() - gWaitStartedAt;
      return;
    }
    gWaitStartedAt = millis();
    gWaitedFor = 0;
    SensorValues s = getSensorData();
    sendPacket(0, s.temp, s.light, s.moisture, s.humidity, mockRainPrediction);
    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
}
