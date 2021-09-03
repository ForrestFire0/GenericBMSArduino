# Generic BMS Reader for Arduino
Want to access all that juicy battery data from your BMS without having to use [computer software](https://www.lithiumbatterypcb.com/Computer%20operation%20software.rar) to read the info?
Want to display all that data on an Arduino driven LCD or OLED? You have come to the right place.

This is an Arduino library that communicates data from Generic Chinese BMSs.
Might work with:
<ul>
<li>BMSs from lithiumbatterypcb.com aka LLT Power</li>
<li>'Xiaoxiang' BMS</li>
<li>Some BMS's Labelled "Smart BMS"</li>
</ul>
Aka, if your board looks like this:

![Image of Generic BMS](https://www.lithiumbatterypcb.com/wp-content/uploads/2018/01/60V-Lithium-Ion-Battery-PCB.jpg)

It is probably going to work. Mine was a 21s so it had two of the 16 pin JST connectors at the base. All cell counts should work. You can always check if the UART protocol matches the protocol linked at the bottom of the page.

<h1>Step 1: Wiring</h1>

1. Wire your BMS to the cells and to the load as normal.
1. Connect the Arduino
  1. Connect the ground of the arduino to the ground of the BMS
  1. Connect the RX and TX to any two pins on the Arduino
  1. DO NOT CONNECT THE <fix me> PIN TO VCC. You may connect this pin (which supplies ~10.5 volts) to VIN if you wish to power the Arduino from the BMS.
  
# Step 2: Setting up the code
*Just copy the example code!*
1. Set the number of cells with "#define NUM_CELLS"
1. Set the number of temperature probes with "#define NUM_TEMP_PROBES"
1. Create a Software Serial object.
1. Inside void setup(), begin the Software Serial port at 9600 baud.
1. Inside void setup(), pass the software serial object to the BMS object.

```cpp
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
```
# Step 3: Reading the Data
1. Call "bms.update()"
1. Call the appropriate methods to return the relevant data:
```cpp
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
 ```
 Note about using the MOSFETStatus struct: just do "bms.getMOSFETStatus().discharge" to get whether discharge is permitted and "bms.getMOSFETStatus().charge" to get whether charger is permitted.


Sources and Helpful Links
1. Another [Smart BMS Reader](https://github.com/bres55/Smart-BMS-arduino-Reader) designed for a similar purpose. I would have used this version if it had been in the traditional Arduino Library format (import, instantiate, start, call methods...), or easier to understand.
1. My [copy of the spreadsheet](https://docs.google.com/spreadsheets/d/1XLCGFEuwLSbxoiXvt5f8QokEm06ZbBmTDPy41bfProQ/edit?usp=sharing) defining the BMS Protocol that explains the protocol and how to communicate with the BMS. I also color coded one of the responses so you can see what bytes mean what.
1. [Any other information you might ever need](https://wnsnty.xyz/entry/jbd-xiaoyang-smart-bluetooth-bms-information) plus a TON of helpful links that I won't duplicate.





