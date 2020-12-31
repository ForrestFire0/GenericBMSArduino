/**
    This code will:
    - Read the BMS cell voltages, pack voltage, and error codes.
*/

#define BMS_RX_PIN 2 //pin the bms is receiving signals on, where we are sending.
#define BMS_TX_PIN 3 //pin the BMS is sending signals to, where we are receiving.
#define NUM_CELLS 21
#define NUM_TEMP_PROBES 4
#define BMS_DEBUG false
// Debug mode prints the hex of every byte coming in and leaving.
// You can compare the bytes you receive to what you should be receiving on the google sheet linked at the bottom of the readme.

#include <SoftwareSerial.h>
#include <BMS.h>
SoftwareSerial BMSSerial(BMS_TX_PIN, BMS_RX_PIN);
BMS bms;

void setup()
{
    Serial.begin(115200);

    BMSSerial.begin(9600);
    bms.begin(&BMSSerial);

    Serial.println("Finished setup");
}

void loop()
{

    bms.update();
    Serial.println("Voltages: (V)");
    for (byte i = 0; i < NUM_CELLS; i++)
    {
        Serial.print(i + 1);
        Serial.print(F("   "));
        if (i < 9)
            Serial.print(F(" "));
    }
    Serial.println();
    float *cells = bms.getCells();
    for (byte i = 0; i < NUM_CELLS; i++)
    {
        Serial.print(cells[i], 2);
        Serial.print(F(" "));
    }
    Serial.println();
    uint32_t balstate = bms.getBalanceState();
    for (byte i = 0; i < NUM_CELLS; i++)
    {
        Serial.print(bitRead(balstate, i));
        Serial.print(F("    "));
    }

    Serial.print("\nPack voltage: ");
    Serial.println(bms.getPackVoltage());
    Serial.println("\nMax  Min  Average");
    Serial.print(bms.getCellMaxVoltage());
    Serial.print("   ");
    Serial.print(bms.getCellMinVoltage());
    Serial.print("   ");
    Serial.println(bms.getPackVoltage() / NUM_CELLS);
    Serial.print("Cell Diff: ");
    Serial.println(bms.getCellDiff());

    Serial.print("Charge: ");
    Serial.println(bms.getMOSFETStatus().charge ? "Permitted" : "Prohibited");
    Serial.print("Discharge: ");
    Serial.println(bms.getMOSFETStatus().discharge ? "Permitted" : "Prohibited");

    Serial.println("Temps: (C)");
    for (byte i = 0; i < NUM_TEMP_PROBES; i++)
    {
        Serial.print(i + 1);
        Serial.print(F("     "));
    }
    Serial.println();
    float *probes = bms.getProbeData();
    for (byte i = 0; i < NUM_TEMP_PROBES; i++)
    {
        Serial.print(probes[i], 2);
        Serial.print(F(" "));
    }

    delay(2000);
}
