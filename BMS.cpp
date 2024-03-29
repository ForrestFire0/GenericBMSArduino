#ifndef BMS_CPP
#define BMS_CPP

//This is to define how you wish the code to send errors. Usually you would want it to send to the Serial Monitor.
//For me I wanted it to send it to the Serial Monitor but in JSON format.
#ifndef debug_msg
#define debug_msg(error) Serial.println(F("{\"s\":\"bms_error\", \"error\": \"" error "\"}"));
// \/\/\/\/ Enable to get errors on Serial Monitor. Make sure to comment out the line above, too.
//#define debug_msg(error) Serial.println(F(error));
#endif

#include "Arduino.h"
#include "BMS.h"

BMS::BMS()
{
}

void BMS::begin(SoftwareSerial *serialPort)
{
    BMSSerial = serialPort;
    clear();
#if BMS_DEBUG
    Serial.println("BMS DEBUG ENABLED");
#endif
}

bool BMS::requestResponse(uint16_t maxWait)
{
    // Make sure any remaining data is flushed out.
    clear();
    BMSSerial->listen();
#if BMS_DEBUG
    Serial.println("Starting request...");
#endif
    // Send the request
    BMSSerial->write(header, 2);
    BMSSerial->write(outdata.command);
    BMSSerial->write(outdata.len);
    if (outdata.len)
        BMSSerial->write(buffer, outdata.len);
    BMSSerial->write(outdata.checksumA);
    BMSSerial->write(outdata.checksumB);
    BMSSerial->write(end);
    BMSSerial->flush();
#if BMS_DEBUG
    Serial.println("Finished sending request...");
#endif
    delay(10);

    auto start = millis();
    while (BMSSerial->peek() != 0xDD)
    {
        if (millis() > maxWait + start)
        {
#if BMS_DEBUG
            Serial.println("Got no data from the BMS. Returning nothing.");
#endif
            return false;
        }
        delay(10);
    }

#if BMS_DEBUG
    Serial.print("Start: ");
    printHex(next());
    Serial.println();
#else
    next();
#endif

    // Second byte echos command. Lets make sure it matches the original byte.
    indata.command = next();
#if BMS_DEBUG
    Serial.print("Command: ");
    printHex(indata.command);
    Serial.println();
#endif
    if (indata.command != outdata.command)
    {
        debug_msg("Ignorable error: Command byte does not match.") return false;
    }

    //Next byte is status.
    byte status = next();
#if BMS_DEBUG
    Serial.print("Status: ");
    printHex(status);
    Serial.println();
#endif

    if (!checkStatus(status))
        return false;

    //Next byte is length
    indata.len = next();
#if BMS_DEBUG
    Serial.print("Length: ");
    printHex(indata.len);
    Serial.print("\nData:");
#endif
    // Now lets load the payload into the... well payload.
    for (int i = 0; i < indata.len; i++)
    {
        buffer[i] = next();
#if BMS_DEBUG
        printHex(buffer[i]);
#endif
    }

    indata.checksumA = next();
    indata.checksumB = next();
#if BMS_DEBUG
    printHex(indata.checksumA);
    printHex(indata.checksumB);
#endif

    // make sure last byte is end byte
    byte end = next();

#if BMS_DEBUG
    printHex(end);
#endif

    if (end != 0x77)
    {
        debug_msg("Fatal error: End byte does not match.");
        return false;
    }

    return true;
}

boolean BMS::update(uint16_t maxWait)
{
    //Request cell data.
    outdata.command = 0x04;
    outdata.len = 0x00;
    outdata.checksumA = 0xFF;
    outdata.checksumB = 0xFC;
    bool cellSuccess = requestResponse(maxWait);

    if (indata.len != NUM_CELLS * 2)
        debug_msg("Fatal error: BMS returned no or incorrect data!")

    else if (cellSuccess)
    {
        cellMax = 0, cellMin = 100;
        for (int i = 0; i < (indata.len) / 2; i++)
        {
            byte highbyte = buffer[2 * i];
            byte lowbyte = buffer[2 * i + 1];

            uint16_t cellnow = (highbyte << 8) | lowbyte;
            cells[i] = cellnow / 1000.0f;
            if (cells[i] > cellMax)
                cellMax = cells[i];
            if (cells[i] < cellMin)
                cellMin = cells[i];
        }
        cellDiff = cellMax - cellMin;
    }
    else
    {
        debug_msg("Some error getting cell data.");
    }
    clear();
    //Request pack data.
    outdata.command = 0x03;
    outdata.len = 0x00;
    outdata.checksumA = 0xFF;
    outdata.checksumB = 0xFD;
    bool mainSuccess = requestResponse(maxWait);

    if (mainSuccess)
    {
        balanceState = ((uint32_t)buffer[14]) << 24 | ((uint32_t)buffer[15]) << 16 | ((uint16_t)buffer[12]) << 8 | buffer[13];

        uint16_t temp;

        temp = (buffer[0] << 8) | buffer[1];
        packVoltage = temp / 100.0f; // convert to float and leave at 2 dec places

        temp = (buffer[2] << 8) | buffer[3];
        packCurrent = temp / 100.0f; // convert to float and leave at 2 dec places

        if (buffer[22] != NUM_TEMP_PROBES)
        {
            Serial.print(F("Error: num of temp probes. Found: "));
            Serial.print(buffer[22]);
        }

        for (byte i = 0; i < buffer[22]; i++)
        {
            temp = buffer[23 + 2 * i] << 8 | buffer[24 + 2 * i];
            probes[i] = (temp - 2731) / 10.00f;
        }

        MOSFETStatus.charge = (buffer[20] % 2) == 1;
        MOSFETStatus.discharge = buffer[20] >= 2;
    }
    else
    {
        debug_msg("Some error getting pack data.");
    }

    return cellSuccess && mainSuccess;
}

boolean BMS::checkStatus(byte status)
{
    switch (status)
    {
    case 0x80:
        debug_msg("BMS returned fail code...");
        return false;
    case 0x00:
        return true;
    default:
        debug_msg("BMS returned unknown code: ");
        Serial.println(status);
        return false;
    }
    return false;
}

void BMS::printHex(byte x)
{
    if (x < 16)
    {
        Serial.print("0");
    }
    Serial.print(x, HEX);
    Serial.print(" ");
}

//Robust way to return the next byte from the port.
byte BMS::next()
{
    while (!BMSSerial->available())
        ;
    return BMSSerial->read();
}
void BMS::clear()
{
    while (BMSSerial->available())
    {
        BMSSerial->read();
    }
}
#endif