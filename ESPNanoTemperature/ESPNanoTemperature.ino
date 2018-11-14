#include <OneWire.h>
#include <DallasTemperature.h>

#define SSID ""
#define PASS ""
#define DST_IP ""
#define LED 13

#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer = { 0x28, 0x3F, 0x90, 0x99, 0x00, 0x00, 0x00, 0x12 };
DeviceAddress outsideThermometer = { 0x28, 0x6B, 0xDE, 0x3D, 0x00, 0x00, 0x00, 0x0C };

unsigned long lastMeasured = 0, measurementInterval = 60L * 1000; 
  
boolean connected = false;

void setup()
{
  pinMode(LED, OUTPUT);
  flashLed(LED, 1, 1000);
  
  sensors.begin();
  sensors.setResolution(insideThermometer, 12);
  sensors.setResolution(outsideThermometer, 12);
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial.setTimeout(5000);
}
   
void loop()
{
  if (!connected) {
    connectToNetwork();
  }
 
   if ((millis() - lastMeasured > measurementInterval) || (millis() - lastMeasured < 0) || lastMeasured == 0) {
     readOneWire("10", insideThermometer);
     readOneWire("11", outsideThermometer);
     lastMeasured = millis();
   }
}

void sendData(String sensorId, String value) {
  value.trim();
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += DST_IP;
  cmd += "\",80";
  Serial.println(cmd);
  if (Serial.find("Error")) return;
  
  cmd = "GET /index.php?action=add_temperature&hash=JKD5JG6509JEKC&sensor_id=" + sensorId + "&value=" + value + " HTTP/1.1\r\nHost: temperature.fotoni.org\r\n\r\n";
  Serial.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  if(Serial.find(">")) {
    //
  }
  else {
    Serial.println("AT+CIPCLOSE");
    connected = false;
    flashLed(LED, 5, 50);
    return;
  }
     
  Serial.print(cmd);
  delay(2000);

  while (Serial.available())
  {
    char c = Serial.read();
  }
  Serial.println("AT+CIPCLOSE");
  flashLed(LED, 10, 50);
}

void connectToNetwork() {
  resetWifi();
  
  for(int i=0; i<5; i++)
  {
    if(connectWiFi())
    {
      connected = true;
      break;
    }
  }
  if (connected) {
    flashLed(LED, 5, 200);
  }
  else {
    digitalWrite(LED, HIGH);
    delay(1000);
    resetWifi();
  }
     
  delay(5000);
  Serial.println("AT+CIPMUX=0");
}

void resetWifi() {
  Serial.println("AT+RST");
  delay(1000);
  if(Serial.find("Ready"))
  {
    flashLed(LED, 3, 200);
  }
  else
  {
    digitalWrite(LED, HIGH);
    while(1);
  }
  connected = false;
}

boolean connectWiFi()
{
  Serial.println("AT+CWMODE=1");
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  Serial.println(cmd);

  delay(2000);
  if(Serial.find("OK"))
  {
    return true;
  }
  else {
    return false;
  }
}

void flashLed(int ledPin, int times, int flashDuration) {
  digitalWrite(ledPin, LOW);
  for (int i = 0 ; i < times ; i++) {
    digitalWrite(ledPin, HIGH);
    delay(flashDuration);
    digitalWrite(ledPin, LOW);
    if (i < times-1)
      delay(flashDuration);
  }
}

void readOneWire(String sensorId, DeviceAddress device) {
    sensors.requestTemperatures();
    float temperature = sensors.getTempC(device);
    if (temperature == -127.00 || temperature == 85.00) {
      flashLed(LED, 5, 1000);
    } 
    else {
      char tempS[5];
      dtostrf(temperature, 3, 2, tempS);
      String tempText = tempS;
      sendData(sensorId, tempText);
      flashLed(LED, 5, 100);
      flashLed(LED, 1, 1000);
    }
}
