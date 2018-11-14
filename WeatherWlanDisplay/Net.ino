void sendSensorData() {
  //Sisä
  if ((millis() - sensor1LastUpdated > sensorUpdateInterval) || (millis() - sensor1LastUpdated < 0)) {
    if (temperature != -127) {
      sendData("04", "add_temperature", temperature);
      sendData("05", "add_humidity", humidity);
    }
    sensor1LastUpdated = millis();
  }
  
  //Ulko
  if ((millis() - sensor2LastUpdated > sensorUpdateInterval) || (millis() - sensor2LastUpdated < 0)) {
    if (temperature2 != -127) {
      sendData("13", "add_temperature", temperature2);
      sendData("12", "add_humidity", humidity2);
    }
    sensor2LastUpdated = millis();
  }
  //Sauna
  if ((millis() - sensor3LastUpdated > sensorUpdateInterval) || (millis() - sensor3LastUpdated < 0)) {
    if (saunaTemperature != -127.00 && saunaTemperature != 85.00) {
      sendData("09", "add_temperature", saunaTemperature);
    }
    sensor3LastUpdated = millis();    
  }
}

void sendData(String sensorId, String action, double value) {
  digitalWrite(WLAN_LED, HIGH);
  
  unsigned long startTime, t = 0L;
  uint32_t ip;
  
  char dataToSend[10];
  dtostrf(value, 2, 2, dataToSend);

  ip = 0;
  startTime = millis();
  while (ip == 0 && ((millis() - startTime) < connectTimeout)) {
    if (! cc3000.getHostByName("fotoni.org", &ip)) {
      Serial.println(F("Couldn't resolve!"));
      conn = false;
    }
    delay(500);
  }
  if(conn) {
    startTime = millis();
    do {
      client = cc3000.connectTCP(ip, 80);
    } while((!client.connected()) &&
            ((millis() - startTime) < connectTimeout));

    if(client.connected()) {

      String messageString = "/index.php?action=" + action + "&hash=JKD5JG6509JEKC&sensor_id=" + sensorId + "&value=" + dataToSend;
      char message[100];
      messageString.toCharArray(message, 100);
      Serial.println(message);
      
      client.fastrprint(F("GET "));
      client.fastrprint(message);
      client.fastrprint(F(" HTTP/1.1\r\n"));
      client.fastrprint(F("Host: ")); client.fastrprint("temperature.fotoni.org"); client.fastrprint(F("\r\n"));
      client.fastrprint(F("\r\n"));
      client.println();
    }
    else {
      conn = false;
      cc3000.disconnect();
    }
    
    unsigned long lastRead = millis();
    while (client.connected() && (millis() - lastRead < 3000)) {
      while (client.available()) {
        char c = client.read();
        Serial.print(c);
        lastRead = millis();
      }
    }
  
    client.close();
  }
  
  digitalWrite(WLAN_LED, LOW);
}
