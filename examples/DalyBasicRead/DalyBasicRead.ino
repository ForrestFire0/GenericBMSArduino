/**
    This code will:
    - Read the BMS cell voltages, pack voltage, and error codes
    - Work on DalyBMS
*/

#define BMS_RX_PIN 2 //pin the bms is receiving signals on, where we are sending.
#define BMS_TX_PIN 3 //pin the BMS is sending signals to, where we are receiving.
#define NUM_CELLS 21
#define NUM_TEMP_PROBES 4
#define BMS_DEBUG false
// Debug mode prints the hex of every byte coming in and leaving.
// You can compare the bytes you receive to what you should be receiving on the google sheet linked at the bottom of the readme.

#include <SoftwareSerial.h>
#include <DalyBMS.h>
SoftwareSerial BMSSerial(BMS_TX_PIN, BMS_RX_PIN);
DalyBMS bms;

void setup()
{
    Serial.begin(115200);

    BMSSerial.begin(9600);
    bms.begin(&BMSSerial);

    Serial.println("Finished setup");

    bms.update(); //Put this loop eventually. For testing run it once.
}

void loop()
{
}
