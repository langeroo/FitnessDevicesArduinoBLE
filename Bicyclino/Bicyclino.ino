#include "Arduino.h"
/* For the bluetooth funcionality */
#include <ArduinoBLE.h>

/* Device name which can be seen in BLE scanning software. */
#define BLE_DEVICE_NAME               "Bicyclino"
/* Local name which should pop up when scanning for BLE devices. */
#define BLE_LOCAL_NAME                "Bicyclino"

//https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Characteristics/org.bluetooth.characteristic.cycling_power_measurement.xml
BLEService CyclePowerService("1818");
BLECharacteristic CyclePowerFeature("2A65", BLERead, 4);
BLECharacteristic CyclePowerMeasurement("2A63", BLERead | BLENotify, 8);
BLECharacteristic CyclePowerSensorLocation("2A5D", BLERead, 1);

unsigned char bleBuffer[8];
unsigned char slBuffer[1];
unsigned char fBuffer[4];

short power;
unsigned short crank_revolutions = 0;
unsigned short timestamp = 0;
unsigned short flags = 0x20;
byte sensorlocation = 0x0D;


void setup() 
{
  Serial.begin(9600);
  if (!BLE.begin()) 
  {
    while (1);
    Serial.println("Waiting for connection...");   
  }
  else
  {
    BLE.setDeviceName(BLE_DEVICE_NAME);
    BLE.setLocalName(BLE_LOCAL_NAME);
    BLE.setAdvertisedService(CyclePowerService);
    CyclePowerService.addCharacteristic(CyclePowerFeature);
    CyclePowerService.addCharacteristic(CyclePowerMeasurement);
    CyclePowerService.addCharacteristic(CyclePowerSensorLocation);
    BLE.addService(CyclePowerService);
    BLE.advertise();
   }
  slBuffer[0] = sensorlocation & 0xff;
  fBuffer[0] = 0x00;
  fBuffer[1] = 0x00;
  fBuffer[2] = 0x00;
  fBuffer[3] = 0x08;
}

void loop() 
{
  BLEDevice central = BLE.central();
  if(central)
  {
    while(central.connected())
    {
        /* Instantaneous Power (Watts) */
        power = 123;

        /* Crank revolution (count; used to determine cadence) */
        crank_revolutions++;

        /* Timestamp in seconds w/ resolution of 1024 */
        timestamp = millis() / 1000 * 1024; 

        // Pack data to be broadcast
        bleBuffer[0] = flags & 0xff;
        bleBuffer[1] = (flags >> 8) & 0xff;
        bleBuffer[2] = power & 0xff;
        bleBuffer[3] = (power >> 8) & 0xff;
        bleBuffer[4] = crank_revolutions & 0xff;
        bleBuffer[5] = (crank_revolutions >> 8) & 0xff;
        bleBuffer[6] = timestamp & 0xff;
        bleBuffer[7] = (timestamp >> 8) & 0xff;
       
        CyclePowerFeature.writeValue(fBuffer, 4);
        CyclePowerMeasurement.writeValue(bleBuffer, 8);
        CyclePowerSensorLocation.writeValue(slBuffer, 1);
        delay(1000);
    }
  }
}
