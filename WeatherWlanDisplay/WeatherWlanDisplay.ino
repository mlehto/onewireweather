#include "U8glib.h"
#include <DS1307RTC.h>
#include <Time.h>
#include <Wire.h>
#include <Timezone.h>
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <VirtualWire.h>
#include <dht.h>

//WLAN
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER);
#define WLAN_SSID       ""
#define WLAN_PASS       ""
#define WLAN_SECURITY   WLAN_SEC_WPA2
Adafruit_CC3000_Client client;
bool conn = false;
bool timeToUpdateTime = true;

//WLAN time
const unsigned long
  connectTimeout  = 15L * 1000L, // Max time to wait for server connection
  responseTimeout = 15L * 1000L; // Max time to wait for data from server
int
  countdown       = 0;  // loop() iterations until next time server query
TimeChangeRule FIST = {"FIST", Last, Sun, Mar, 2, 180};
TimeChangeRule FIT = {"FIT ", Last, Sun, Oct, 3, 120};
Timezone FI(FIST, FIT);
TimeChangeRule *tcr;        //pointer to the time change rule, use to get the TZ abbrev
tmElements_t date;

//1-wire sensors
#define ONE_WIRE_BUS 30
#define ONE_WIRE_VCC 28
#define ONE_WIRE_GND 26
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress saunaThermometer = { 0x28, 0x95, 0xEB, 0x55, 0x04, 0x00, 0x00, 0x29 };
String saunaText; float saunaTemperature = -127; 
unsigned long saunaLastMeasured = 0;
float saunaHistory[180];
float saunaTemperatureChange = 0;
float smallChange = 0.5;
float bigChange = 1.4;
boolean showSaunaTemperature = false;
  
//DHT sensors
#define DHT22_PIN 37  
#define DHT22_2_PIN 36 
#define DHT_2_VCC 38 
#define DHT_2_NC 36 
#define DHT_2_GND 32
String humidityText, temperatureText;
String humidity2Text, temperature2Text;
const unsigned long dhtMeasurementInterval = 20*1000L; unsigned long dhtLastMeasured = 0, dht2LastMeasured = 0;
double humidity, temperature, humidity2, temperature2;
dht DHT;
dht DHT2;

//Sensor info to DB
const unsigned long sensorUpdateInterval = 60*1000L; 
unsigned long sensor1LastUpdated = 0;
unsigned long sensor2LastUpdated = 10000L;
unsigned long sensor3LastUpdated = 20000L;
  
int debugButton = 7;
int displayBacklight = 46, displayBacklightLevel = 255, displayBacklightLevelTarget = 255;

String h_string = ""; String m_string = ""; 
String d_string = ""; String mon_string = ""; String y_string = "";
String day_string = "...";
U8GLIB_ST7920_128X64_1X u8g(40, 42, 44);
#define LCD_LED 48
#define WLAN_LED 49
String l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;

//Light level sensor
int BH1750_address = 0x23;
byte luxBuf[2];
const unsigned long luxMeasurementInterval = 1*1000L; unsigned long lastMeasuredLux = 0;
String lightLevel = "0 lx";
double lightLevelLux = 0;

void setup() {                
  pinMode(displayBacklight, OUTPUT);
  pinMode(debugButton, INPUT);
  
  pinMode(DHT_2_GND, OUTPUT); digitalWrite(DHT_2_GND, LOW);
  pinMode(DHT_2_NC, OUTPUT); digitalWrite(DHT_2_NC, LOW);
  pinMode(DHT_2_VCC, OUTPUT); digitalWrite(DHT_2_VCC, HIGH);
  
  pinMode(ONE_WIRE_VCC, OUTPUT); digitalWrite(ONE_WIRE_VCC, HIGH);
  pinMode(ONE_WIRE_GND, OUTPUT); digitalWrite(ONE_WIRE_GND, LOW);
  
  pinMode(LCD_LED, OUTPUT); digitalWrite(LCD_LED, LOW);
  pinMode(WLAN_LED, OUTPUT); digitalWrite(WLAN_LED, LOW);
  
  SPI.begin();
  
  Serial.begin(9600);
  Serial.println("Booting...");
  
  logger("Serial up");
   
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {u8g.setColorIndex(255);}
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {u8g.setColorIndex(3);}
  else if ( u8g.getMode() == U8G_MODE_BW ) {u8g.setColorIndex(1);}
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {u8g.setHiColorByRGB(255,255,255);}
  u8g.setRot270();
  
  Wire.begin();
  logger("Wire up");
  
  logger("Initialising the CC3000...");
  if (!cc3000.begin()) {
    logger("Unable to initialise the CC3000!");
  }
  logger("CC3000 is up");
  
  sensors.begin();
  sensors.setResolution(saunaThermometer, 12);
  logger("1-wire is up");
}

void loop() {
  refreshTime();
  
  refreshDisplay();
  
  readDHTSensors();
  
  readOneWire();
  
  if (!conn) {
    connectToWlan();
  }
  
  if (timeToUpdateTime) {
    syncTimeToNTP();
  }
  
  sendSensorData();
}

void connectToWlan() {
  logger("Deleting old connection profiles");
  if (!cc3000.deleteProfiles()) {
    logger("Failed deleting conns!");
    while(1);
  }
  conn = false;

  char *ssid = WLAN_SSID;
  char myConcatenation[80];
  sprintf(myConcatenation,"%s %s","Attempting connect to " ,WLAN_SSID);
  logger(myConcatenation);
  
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    logger("Connection failed!");
  }
   
  logger("WLAN Connected!");
  
  logger("Request DHCP");
  while (!cc3000.checkDHCP()) {
    delay(100);
  }
  
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    logger("Unable to retrieve the IP Address!");
  }
  else
  {
    logger("Got IP");
    conn = true;
  }
}

void logger(String message) {
  l10 = l9; l9 = l8; l8 = l7; l7 = l6; l6 = l5; l5 = l4; l4 = l3; l3 = l2; l2 = l1;
  l1 = message;
}


  
