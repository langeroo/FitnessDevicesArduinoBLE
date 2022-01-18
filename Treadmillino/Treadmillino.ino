#include "Arduino.h"
/* For the bluetooth funcionality */
#include <ArduinoBLE.h>

/* Device name which can be seen in BLE scanning software. */
#define BLE_DEVICE_NAME               "Treadmilino"
/* Local name which should pop up when scanning for BLE devices. */
#define BLE_LOCAL_NAME                "Treadmilino"

// https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Characteristics/org.bluetooth.characteristic.rsc_measurement.xml
BLEService RunningSpeedAndCadenceService("1814");
BLECharacteristic RscFeature("2A54", BLERead, 4);
BLECharacteristic RscMeasurement("2A53", BLERead | BLENotify, 8);
BLECharacteristic RscSensorLocation("2A5D", BLERead, 1);

unsigned char bleBuffer[4];
unsigned char slBuffer[1];
unsigned char fBuffer[4];

unsigned short flags = 0x04;
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
    BLE.setAdvertisedService(RunningSpeedAndCadenceService);
    RunningSpeedAndCadenceService.addCharacteristic(RscFeature);
    RunningSpeedAndCadenceService.addCharacteristic(RscMeasurement);
    RunningSpeedAndCadenceService.addCharacteristic(RscSensorLocation);
    BLE.addService(RunningSpeedAndCadenceService);
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
        /* Instantaneous Speed (meters per second) */
        float       inst_speed_mps = 3.3528; // 7.5mph
        
        /* Instantaneous Cadence (steps per min) */
        uint8_t     inst_cadence_steps_per_min = 20;

        /* 
         *  You can add inst_stride_length and total_distance but they're not required.
         *  If you do add these, you'll need to modify the flags to add the corresponding
         *  bits to activate these values, and you'll need to put them into bleBuffer at
         *  the correct indices. Refer to the documentation link at the top of the sketch.
        */
        //uint16_t    inst_stride_length = 1;   
        //uint32_t    total_distance = 10;

        // Unit is in m/s with a resolution of 1/256 s
        uint16_t  inst_speed_ble = uint16_t(inst_speed_mps * 256);

        // Unit is in 1/minute (or RPM) with a resolutions of 1 1/min (or 1 RPM)
        // 1 Revolution is 2 steps
        uint8_t   inst_cadence_cycles_per_min = inst_cadence_steps_per_min/2;
        
        // Pack data to be broadcast
        bleBuffer[0] = flags & 0xff;
        bleBuffer[1] = inst_speed_ble & 0xff;
        bleBuffer[2] = (inst_speed_ble >> 8) & 0xff;
        bleBuffer[3] = inst_cadence_cycles_per_min & 0xff;
       
        RscFeature.writeValue(fBuffer, 4);
        RscMeasurement.writeValue(bleBuffer, 4);
        RscSensorLocation.writeValue(slBuffer, 1);
        delay(1000);
    }
  }
}
