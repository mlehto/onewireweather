void readOneWire() {
  if ((millis() - saunaLastMeasured > dhtMeasurementInterval) || (millis() - saunaLastMeasured < 0) || saunaLastMeasured == 0) {
    sensors.requestTemperatures();
    saunaTemperature = sensors.getTempC(saunaThermometer);
    if (saunaTemperature == -127.00 || saunaTemperature == 85.00) {
      logger("Error getting temperature for sauna.");
      saunaText = "-";
    } 
    else {
      char tempS[5];
      dtostrf(saunaTemperature, 3, 0, tempS);
      saunaText = tempS;
      saunaText = saunaText + "\xb0" "c";
           
      for (int i = 179 ; i > 0 ; i--) {
        saunaHistory[i] = saunaHistory[i-1];
      }
      saunaHistory[0] = saunaTemperature;
      
      if (saunaHistory[2] != 0) {
        saunaTemperatureChange = (saunaHistory[2] - saunaHistory[0])*-1;
      }

      showSaunaTemperature = (saunaTemperatureChange > bigChange) || saunaTemperature > 45;
    }
    
    saunaLastMeasured = millis();
  }
}

