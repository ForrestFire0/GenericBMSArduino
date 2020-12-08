#include "Arduino.h"
#include <SoftwareSerial.h>
#ifndef BMS_H
#define BMS_H

// Input how many cells you expect to have
#define NUM_CELLS 21
#define NUM_TEMP_PROBES 4
#define BMS_DEBUG true

typedef struct
{
    byte command;
    byte len; //Length of the payload. Does not include cls, id, or checksum bytes
    byte *data;
    byte checksumA; //Given to us from module. Checked against the rolling calculated A/B checksums.
    byte checksumB;
} BMSPacket;

typedef struct
{
    boolean charge;
    boolean discharge;
} MOSFET_STATUS;

const byte header[2] = {0xDD, 0xA5};
const byte end = 0x77;

class BMS
{
public:
    BMS();
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
    BMSPacket outdata;
    BMSPacket indata;
    bool checkStatus(byte status);
    bool requestResponse(uint16_t maxWait);

    float cells[NUM_CELLS];
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