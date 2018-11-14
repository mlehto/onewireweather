#include <OneWire.h>
#include <DallasTemperature.h>
#include <VirtualWire.h>

// Data wire is plugged into pin 3 on the Arduino
#define ONE_WIRE_BUS 3

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

DeviceAddress insideThermometer = { 0x10, 0x4A, 0xB4, 0x02, 0x01, 0x08, 0x00, 0xCB };
DeviceAddress outsideThermometer = { 0x10, 0xBA, 0xB5, 0x02, 0x01, 0x08, 0x00, 0x74 };

long measuringInterval = 5L * 1000;

void setup(void)
{
  Serial.begin(9600);

  sensors.begin();
  sensors.setResolution(insideThermometer, 10);
  sensors.setResolution(outsideThermometer, 10);
  
  vw_setup(2000);
  vw_set_tx_pin(8);
}

void sendTemperature(DeviceAddress deviceAddress, String sensorId)
{
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00 || tempC == 85.00) {
    Serial.print("Error getting temperature");
  } 
  else {
    char tempS[6];
    dtostrf(tempC, 6, 2, tempS);
    char tempMessage[9];
    tempMessage[0] = sensorId[0];
    tempMessage[1] = sensorId[1];
    tempMessage[2] = tempS[0];
    tempMessage[3] = tempS[1];
    tempMessage[4] = tempS[2];
    tempMessage[5] = tempS[3];
    tempMessage[6] = tempS[4];
    tempMessage[7] = tempS[5];
    tempMessage[8] = NULL;
    
    digitalWrite(13, HIGH);
    Serial.print("Sending... ");  
    Serial.print(tempMessage);
    vw_send((uint8_t *)tempMessage, strlen(tempMessage));
    vw_wait_tx(); // Wait until the whole message is gone
    digitalWrite(13, LOW);
    Serial.print(" ...done!");  
    Serial.print("\n\r");
  }
}

void loop(void)
{ 
  sensors.requestTemperatures();
  
  sendTemperature(insideThermometer, "10");
  Serial.print("\n\r");
  
  delay(measuringInterval);
  
  sendTemperature(outsideThermometer, "11");  
  Serial.print("\n\r\n\r");
  
  delay(measuringInterval);
}

