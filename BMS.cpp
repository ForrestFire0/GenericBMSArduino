#ifndef BMS_CPP
#define BMS_CPP

#include "Arduino.h"
#include "BMS.h"

BMS::BMS()
{
}

void BMS::begin(SoftwareSerial *serialPort)
{
    BMSSerial = serialPort;
    clear();
    indata.data = new byte[50];
}

bool BMS::requestResponse(uint16_t maxWait)
{
    // Make sure any remaining data is flusheed out.
    clear();

    // Send the request
    BMSSerial->write(header, 2);
    BMSSerial->write(outdata.command);
    BMSSerial->write(outdata.len);
    if (outdata.len)
        BMSSerial->write(outdata.data, outdata.len);
    BMSSerial->write(outdata.checksumA);
    BMSSerial->write(outdata.checksumB);
    BMSSerial->write(end);
    BMSSerial->flush();
    Serial.println("\nFinished sending request...");
    delay(10);

    auto start = millis();
    while (BMSSerial->peek() != 0xDD)
    {
        if (millis() > maxWait + start)
            return false;
        delay(10);
    }

// Second byte echos command. Lets make sure it matches the original byte.
#if BMS_DEBUG
    Serial.print("Start: ");
    printHex(next());
    Serial.println();
#endif

    indata.command = next();
#if BMS_DEBUG
    Serial.print("Command: ");
    printHex(indata.command);
    Serial.println();
#endif
    if (indata.command != outdata.command)
    {
        Serial.println(F("\nError: Command byte does not match."));
        return false;
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
    Serial.println("\nData:");
#endif
    // Now lets load the payload into the... well payload.
    for (int i = 0; i < indata.len; i++)
    {
        indata.data[i] = next();
#if BMS_DEBUG
        printHex(indata.data[i]);
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
        Serial.print(F("BMS incorrect last byte: 0x77 expected, received 0x"));
        return false;
    }

    return true;
}

boolean BMS::update(uint16_t maxWait)
{
    //Request cell data.
    outdata.command = 0x04;
    outdata.len = 0x00;
    outdata.data = nullptr;
    outdata.checksumA = 0xFF;
    outdata.checksumB = 0xFC;
    bool cellSuccess = requestResponse(maxWait);
    if (indata.len != NUM_CELLS * 2)
    {
        Serial.println("Data length should be 42. length: ");
        Serial.println(indata.len);
    }
    else if (cellSuccess)
    {
        cellMax = 0, cellMin = 100;
        for (int i = 0; i < (indata.len) / 2; i++)
        {
            byte highbyte = indata.data[2 * i];
            byte lowbyte = indata.data[2 * i + 1];

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
        Serial.print(F("Error getting cell data."));
    }
    clear();
    //Request pack data.
    outdata.command = 0x03;
    outdata.len = 0x00;
    outdata.data = nullptr;
    outdata.checksumA = 0xFF;
    outdata.checksumB = 0xFD;
    bool mainSuccess = requestResponse(maxWait);

    if (cellSuccess)
    {
        balanceState = ((uint32_t)indata.data[13]) << 24 | ((uint32_t)indata.data[14]) << 16 | ((uint32_t)indata.data[15]) << 8 | indata.data[16];

        uint16_t temp;

        temp = (indata.data[0] << 8) | indata.data[1];
        packVoltage = temp / 100.0f; // convert to float and leave at 2 dec places

        temp = (indata.data[2] << 8) | indata.data[3];
        packCurrent = temp / 100.0f; // convert to float and leave at 2 dec places

        if (indata.data[22] != NUM_TEMP_PROBES)
        {
            Serial.print("Error: wrong number of temp probes found. Found: ");
            Serial.print(indata.data[22]);
        }

        for (byte i = 0; i < indata.data[22]; i++)
        {
            temp = indata.data[23 + 2 * i] << 8 | indata.data[24 + 2 * i];
            probes[i] = (temp - 2731) / 10.00f;
        }

        MOSFETStatus.charge = indata.data[20] & (1 << 7);
        MOSFETStatus.discharge = indata.data[20] & (1 << 6);
    }
    else
    {
        Serial.print(F("Error getting cell data."));
    }

    return cellSuccess && mainSuccess;
}

boolean BMS::checkStatus(byte status)
{
    switch (status)
    {
    case 0x80:
        Serial.println("BMS returned fail code...");
        return false;
    case 0x00:
        return true;
    default:
        Serial.println("BMS returned unknown code:");
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