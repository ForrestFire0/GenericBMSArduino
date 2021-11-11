#ifndef DalyBMS_CPP
#define DalyBMS_CPP

//This is to define how you wish the code to send errors. Usually you would want it to send to the Serial Monitor.
//For me I wanted it to send it to the Serial Monitor but in JSON format.
#ifndef debug_msg
//#define debug_msg(error) Serial.println(F("{\"s\":\"bms_error\", \"error\": \"" error "\"}"));
#define debug_msg(error) Serial.println(F(error));
#endif

#include "Arduino.h"
#include "DalyBMS.h"

DalyBMS::DalyBMS()
{
}

void DalyBMS::begin(SoftwareSerial *serialPort)
{
    BMSSerial = serialPort;
    clear();
#if BMS_DEBUG
    Serial.println("BMS DEBUG ENABLED");
#endif
}

bool DalyBMS::requestResponse(uint16_t maxWait)
{
    // Make sure any remaining data is flushed out.
    clear();
    BMSSerial->listen();
#if BMS_DEBUG
    Serial.println("Starting request...");
#endif
    wBuff[0] = BMS_START_ADDR; //fixed start byte 0xA5
    wBuff[1] = BMS_HOST_ADDR; //fixed Host byte 0x40
    wBuff[2] = outdata.command; //request ID
    wBuff[3] = "0x08"; //fixed length of the following
    memset(wBuff+4, 0, 8); //place Zeros
    wBuff[12] = 0;
      // calculate checksum
  for(i = 0; i <= 11; i++) {
    wBuff[12] += wBuff[i];
    //Serial.println(mbuf[i], HEX);
  }
    //The goal is to send out something like "a5 40 97 08 00 00 00 00 00 00 00 00 84" (Taken from the first line
    // Send the header. This is the "Frame Head/Start Flag" and also the "Communication Module Address" (a5 and 40)
    BMSSerial->write(wBuff, 13);
    // Send the command. This is "message id" in the "Communications content information" table. (you pick, something between 90 and 97)
 //   BMSSerial->write(outdata.command);
    // Looks like this is always 8.
 //   BMSSerial->write(8);
 //   for(byte i = 0; i < 8; i++) //From the dump.txt, looks like we need to send 8 zeros.
   //     BMSSerial->write((byte) 0);
   // BMSSerial->write(outdata.checksum); //The last number, the checksum.
//    BMSSerial->write(end); //No end byte for DalyBMS
    BMSSerial->flush();
#if BMS_DEBUG
    Serial.println("Finished sending request...");
#endif
    delay(5);

    auto start = millis();
    while (BMSSerial->peek() != header[1]) //Wait for the BMS to return a 0xA5
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
    next(); //Remove that 0xA5
#endif

    next(); //Next byte should be 0x01. No need to check it, however.

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

    //Next byte is length
    if(next() != 8) {
        //The length of data should always be 8. Something is wrong if this isn't true.
        debug_msg("Error: Data length not 8 bytes. (Line 91)") return false;
    }

    // Now lets load the payload into the... well payload.
    for (int i = 0; i < 8; i++)
    {
        buffer[i] = next();
#if BMS_DEBUG
        printHex(buffer[i]);
#endif
    }

    indata.checksum = next();
#if BMS_DEBUG
    printHex(indata.checksum);
#endif

    // make sure last byte is end byte
//    byte end = next(); No end byte for DalyBMS.
    return true;
}

boolean DalyBMS::update(uint16_t maxWait)
{
    //Ok so DalyBMS does this a lot differently than the other BMS. This is where you should experiment with making requests.
    //Here is an example request. Lets get the data at 0x90.
    outdata.command = 0x90;
    outdata.checksum = 0x7D; //Stole this out of dump.txt. IT WILL CHANGE BASED ON EACH REQUEST.
    requestResponse(1000);
    Serial.println("Response: ");
    for(byte i = 0; i < 8; i++) {
        printHex(buffer[i]);
    }

    //Note - to get cell voltages you might have to make a new function like requestResponse.
    //This is because to get cell voltages you have to send a request and then read up to 96 bytes of data.
    //If you need help, of course ask, but try it yourself first.
}

void DalyBMS::printHex(byte x)
{
    if (x < 16)
    {
        Serial.write('0');
    }
    Serial.print(x, HEX);
    Serial.write(' ');
}

//Robust way to return the next byte from the port.
byte DalyBMS::next()
{
    while (!BMSSerial->available())
        ;
    return BMSSerial->read();
}
void DalyBMS::clear()
{
    while (BMSSerial->available())
    {
        BMSSerial->read();
    }
}
#endif
