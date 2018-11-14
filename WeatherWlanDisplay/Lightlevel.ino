void setBacklightLevel() {
  if ((millis() - lastMeasuredLux > luxMeasurementInterval) || (millis() - lastMeasuredLux < 0)) {
    updateLightLevel();
    
    displayBacklightLevelTarget = map(lightLevelLux, 1, 100, 2, 255);
  
    if (lightLevelLux > 70) {
      displayBacklightLevelTarget = 255;
    }
    if (displayBacklightLevelTarget < 2) displayBacklightLevelTarget = 2;
    
    lastMeasuredLux = millis();
  }  

  displayBacklightLevel = (displayBacklightLevel + displayBacklightLevelTarget) / 2;
  analogWrite(displayBacklight, displayBacklightLevel);
}

void BH1750_Init(int address){
  Wire.beginTransmission(address);
  Wire.write(0x10); // 1 [lux] aufloesung
  Wire.endTransmission();
}

int BH1750_Read(int address) //
{
  int i=0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while(Wire.available()) //
  {
    luxBuf[i] = Wire.read();  // receive one byte
    i++;
  }
  Wire.endTransmission();  
  return i;
}

void updateLightLevel() {
  BH1750_Init(BH1750_address);
  delay(200);
  if(2 == BH1750_Read(BH1750_address)) {
    lightLevelLux = ((luxBuf[0] << 8) | luxBuf[1]) / 1.96;
    lightLevel = doubleToString(lightLevelLux,1) + "lx";
  }
}
