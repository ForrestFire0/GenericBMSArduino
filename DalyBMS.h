#include "Arduino.h"
#include <SoftwareSerial.h>
#ifndef DalyBMS_H
#define DalyBMS_H

// Input how many cells you expect to have
#ifndef NUM_CELLS
#define NUM_CELLS 21
#endif

#ifndef NUM_TEMP_PROBES
#define NUM_TEMP_PROBES 4
#endif

#ifndef BMS_DEBUG
#define BMS_DEBUG false
#endif

typedef struct
{
    byte command;
    byte checksum; //Given to us from module. Checked against the rolling calculated A/B checksums.
} DalyBMSPacket;

typedef struct
{
    boolean charge;
    boolean discharge;
} MOSFET_STATUS;

const byte header[2] = {0xA5, 0x40};
const byte end = 0x77;

class DalyBMS
{
public:
    DalyBMS();
    void begin(SoftwareSerial *serialPort);
    boolean update(uint16_t maxWait = 1000);

    float *getCells() { return cells; }
    uint32_t getBalanceState() { return balanceState; }
    float getPackVoltage() { return packVoltage; }
    float getPackCurrent() { return packCurrent; }
    float getCellMaxVoltage() { return cellMax; }
    float getCellMinVoltage() { return cellMin; }
    float getCellDiff() { return cellDiff; }
    byte getNumProbes() { return NUM_TEMP_PROBES; }
    float *getProbeData() { return probes; }
    MOSFET_STATUS getMOSFETStatus() { return MOSFETStatus; }

private:
    SoftwareSerial *BMSSerial;
    DalyBMSPacket outdata;
    DalyBMSPacket indata;
    bool checkStatus(byte status);
    bool requestResponse(uint16_t maxWait);

    float cells[NUM_CELLS];
    byte buffer[8];
    uint32_t balanceState;
    float packVoltage;
    float packCurrent;
    float cellMax;
    float cellMin;
    float cellDiff;
    MOSFET_STATUS MOSFETStatus;
    float probes[NUM_TEMP_PROBES];
    void printHex(byte x);
    void clear();
    byte next();
};
#endif