#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <VirtualWire.h>

//1-wire sensors
#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer = { 0x10, 0x16, 0x99, 0xFB, 0x00, 0x08, 0x00, 0xC5 };
DeviceAddress outsideThermometer = { 0x10, 0x4E, 0xDA, 0x02, 0x01, 0x08, 0x00, 0xA4 };

//Ethernet settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192,168,1,172);
IPAddress myDns(192,168,1,1);
EthernetClient client;

char server[] = "";

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 5*1000;  // delay between updates, in milliseconds

void setup() {
  Serial.begin(9600);
  
  // Start up the 1-wire library
  sensors.begin();
  // set the resolution to 10 bit (good enough?)
  sensors.setResolution(insideThermometer, 10);
  sensors.setResolution(outsideThermometer, 10);
  
  // give the ethernet module time to boot up:
  delay(1000);

  Ethernet.begin(mac, ip, myDns);

  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  
  //Start the rf link
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000);
  vw_set_rx_pin(3);
  vw_rx_start();
}

void loop() {
  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }

  if (!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    Serial.println("Getting temperatures...");
    sensors.requestTemperatures();
    sendTemperature(insideThermometer, "1");
    sendTemperature(outsideThermometer, "2");
    lastConnectionTime = millis();
  }
  
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message(buf, &buflen))
  {
    int i;
    digitalWrite(13, true); // Flash a light to show received good message

    Serial.print("Got: ");
        
    for (i = 0; i < buflen; i++)
    {
      Serial.print(char(buf[i]));
    }
    Serial.println("");
    digitalWrite(13, false);
    
    if (!client.connected()) {
      char sensorId[3];
      sensorId[0] = buf[0]; sensorId[1] = buf[1]; sensorId[2] = NULL;
      
      char temperature[7];
      if (buf[2] == ' ') {temperature[0] = '+';}
      else {temperature[0] = '-';}
      temperature[1] = buf[3]; temperature[2] = buf[4]; 
      temperature[3] = buf[5]; temperature[4] = buf[6]; temperature[5] = buf[7]; 
      temperature[6] = NULL;
      
      httpRequest(sensorId, temperature);
    }
  }
  
  lastConnected = client.connected();
}

void sendTemperature(DeviceAddress deviceAddress, String sensorId) {
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00 || tempC == 85.00) {
    Serial.println("Error getting temperature for sensor " + sensorId);
  } 
  else {
    char tempS[6];
    dtostrf(tempC, 6, 2, tempS);
    if (tempS[0] == ' ') {tempS[0] = '+';}
    else {tempS[0] = '-';}
    httpRequest(sensorId, tempS);
  }
}

void httpRequest(String sensorId, String temperature) {
  if (client.connect(server, 80)) {
    String message = "GET /index.php?action=add_temperature&hash=JKD5JG6509JEKC&sensor_id=" + sensorId + "&temperature=" + temperature + " HTTP/1.1";

    Serial.println("Sending... " + message);
    
    client.println(message);    
    client.println("Host: temperature.fotoni.org");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();
    client.stop();
  } 
  else {
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.stop();
  }
}




