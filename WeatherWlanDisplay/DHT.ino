void readDHTSensors() {
  readDHTSensor(DHT, DHT22_PIN, &dhtLastMeasured, &humidity, &temperature, &humidityText, &temperatureText);
  readDHTSensor(DHT2, DHT22_2_PIN, &dht2LastMeasured, &humidity2, &temperature2, &humidity2Text, &temperature2Text);
}
  
void readDHTSensor(dht sensor, int DHT_PIN, unsigned long *lastMeasured, double *hum, double *temp, String *humText, String *tempText) {
  if ((millis() - *lastMeasured > dhtMeasurementInterval) || (millis() - *lastMeasured < 0)) {
    int chk = sensor.read22(DHT_PIN);
    switch (chk)
    {
      case DHTLIB_OK:  
                  *hum = sensor.humidity;
                  *temp = sensor.temperature;
                  *humText = String(doubleToString(*hum, 1)) + "\x25";    
                  *tempText = String(doubleToString(*temp, 1)) + "\xb0" "c";                  
                  break;
      case DHTLIB_ERROR_CHECKSUM: 
                  //logger("DHT checksum error"); 
                  *tempText = "(checksum error)";
                  *temp = -127;
                  break;
      case DHTLIB_ERROR_TIMEOUT: 
                  //logger("DHT time out error"); 
                  *tempText = "(timeout)";
                  *temp = -127;
                  break;
      default: 
                  //logger("DHT unknown error"); 
                  *tempText = "(error)";
                  *temp = -127;
                  break;
    }
    
    *lastMeasured = millis();
  }
}
