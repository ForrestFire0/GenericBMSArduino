/**
    This code will:
    - Read the BMS cell voltages, pack voltage, and error codes.
*/


#define BMS_RX_PIN 2 //pin the bms is reciving signals on, where we are sending.
#define BMS_TX_PIN 3 //pin the BMS is sending signals to, where we are recieving.
#define NUM_CELLS 21
#define NUM_TEMP_PROBES 4

#include <SoftwareSerial.h>
#include <BMS.h>
SoftwareSerial BMSSerial(BMS_TX_PIN, BMS_RX_PIN);
BMS bms;

void setup() {
    Serial.begin(115200);
    
    BMSSerial.begin(9600);
    bms.begin(&BMSSerial);
    
    Serial.println("Finished setup");
}

void loop() {
    
    bms.update();

    Serial.println("Voltages: (V)");
    for (byte i = 0; i < NUM_CELLS; i++) {
        Serial.print(i + 1);
        Serial.print(F("   "));
        if (i < 9) Serial.print(F(" "));
    }
    Serial.println();
    float* cells = bms.getCells();
    for (byte i = 0; i < NUM_CELLS; i++) {
        Serial.print(cells[i], 2);
        Serial.print(F(" "));
    }
    Serial.print("\nPack voltage: "); Serial.println(bms.getPackVoltage());
    Serial.print("Min cell voltage: "); Serial.println(bms.getCellMinVoltage());
    Serial.print("Max cell voltage: "); Serial.println(bms.getCellMaxVoltage());
    Serial.print("Cell Diff: "); Serial.println(bms.getCellDiff());

    Serial.print("Balance State: "); Serial.println(bms.getBalanceState(), BIN);
    Serial.println("Charge: "); Serial.println(bms.getMOSFETStatus().charge ? "Permitted" : "Prohibited");
    Serial.println("Discharge: "); Serial.println(bms.getMOSFETStatus().discharge ? "Permitted" : "Prohibited");

    Serial.println("Temps: (C)");
    for (byte i = 0; i < NUM_TEMP_PROBES; i++) {
        Serial.print(i + 1);
        Serial.print(F("     "));
    }
    Serial.println();
    float* probes = bms.getProbeData();
    for (byte i = 0; i < NUM_TEMP_PROBES; i++) {
        Serial.print(probes[i], 2);
        Serial.print(F(" "));
    }
}
