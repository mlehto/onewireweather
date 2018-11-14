void refreshDisplay() {
  setBacklightLevel();
  
  u8g.firstPage();  
  do {
    if (digitalRead(debugButton)) {
      draw();
    } 
    else {
      drawlogger();
    }
  } 
  while(u8g.nextPage());
}

void draw(void) {
  u8g.setFont(u8g_font_helvR18);
  u8g.setPrintPos(3,18);
  u8g.print(h_string + ":" + m_string);
  
  String date = d_string + "." + mon_string + ".";
  drawCentered(date, 28, u8g_font_chikita);
  drawCentered(day_string, 36, u8g_font_chikita);
  
  u8g.drawBox(5, 40, 54, 1);
  
  if (showSaunaTemperature) {
    drawCentered("sauna", 49, u8g_font_chikita);
    drawCentered(saunaText, 71, u8g_font_helvB18);
    drawSaunaArrow(62);
    
    drawSaunaGraph(97);
    
    drawCentered("ulko", 103, u8g_font_chikita);
    drawCentered(humidity2Text, 128, u8g_font_chikita);
    drawCentered(temperature2Text, 120, u8g_font_helvR14);  
  }
  else {
    drawCentered("sis" "\xe4", 55, u8g_font_chikita);
    drawCentered(humidityText, 80, u8g_font_chikita);
    drawCentered(temperatureText, 72, u8g_font_helvR14);
    
    drawCentered("ulko", 97, u8g_font_chikita);
    drawCentered(humidity2Text, 122, u8g_font_chikita);
    drawCentered(temperature2Text, 114, u8g_font_helvR14);  
  }  
}

int widthOfText;
char widthCharHelper[64];
void drawCentered(String text, int ypos, const u8g_fntpgm_uint8_t *font)
{
  text.toCharArray(widthCharHelper,64);
  u8g.setFont(font);
  u8g.setPrintPos(32 - (u8g.getStrWidth(widthCharHelper)/2), ypos);
  u8g.print(text);
}

void drawSaunaArrow(int ypos) {
  if (saunaTemperatureChange > smallChange && saunaTemperatureChange < bigChange) {
    u8g.drawLine(0, ypos+1, 2, ypos-1);
    u8g.drawLine(2, ypos-1, 4, ypos+1);
    u8g.drawLine(0, ypos+1, 4, ypos+1);
    u8g.drawLine(2, ypos-1, 2, ypos+1);
  }
  else if (saunaTemperatureChange >= bigChange) {
    u8g.drawLine(1, ypos+2, 1, ypos-1);
    u8g.drawLine(0, ypos-1, 2, ypos-3);
    u8g.drawLine(2, ypos-3, 4, ypos-1);
    u8g.drawLine(3, ypos-1, 3, ypos+2);
    u8g.drawLine(1, ypos+2, 3, ypos+2);
    u8g.drawLine(2, ypos-3, 2, ypos+2);
  }
  else if (saunaTemperatureChange < -smallChange && saunaTemperatureChange > -bigChange) {
    u8g.drawLine(0, ypos-1, 2, ypos+1);
    u8g.drawLine(2, ypos+1, 4, ypos-1);
    u8g.drawLine(0, ypos-1, 4, ypos-1);
    u8g.drawLine(2, ypos+1, 2, ypos-1);
  }
  else if (saunaTemperatureChange < -bigChange) {
    u8g.drawLine(1, ypos-2, 1, ypos+1);
    u8g.drawLine(0, ypos+1, 2, ypos+3);
    u8g.drawLine(2, ypos+3, 4, ypos+1);
    u8g.drawLine(3, ypos+1, 3, ypos-2);
    u8g.drawLine(1, ypos-2, 3, ypos-2);
    u8g.drawLine(2, ypos+3, 2, ypos-2);
  }
  else {
      //u8g.drawLine(2, ypos, 4, ypos);
  }
}

void drawlogger(void) {
  u8g.setFont(u8g_font_04b_03r);
  int currentLine = printLogLine(0, l1);
  currentLine = printLogLine(currentLine, l2);
  currentLine = printLogLine(currentLine, l3);
  currentLine = printLogLine(currentLine, l4);
  currentLine = printLogLine(currentLine, l5);
  currentLine = printLogLine(currentLine, l6);
  currentLine = printLogLine(currentLine, l7);
  currentLine = printLogLine(currentLine, l8);
  currentLine = printLogLine(currentLine, l9);
  currentLine = printLogLine(currentLine, l10); 
 
  //Light level
  u8g.setColorIndex(0);
  u8g.drawBox(0, 120, 64, 8);
  u8g.setColorIndex(1);
  u8g.setFont(u8g_font_04b_03r);
  String lightLevelString = String(lightLevel) + " " + String(displayBacklightLevelTarget) + " " + String(displayBacklightLevel);
  char luxWidthChar[60];
  lightLevelString.toCharArray(luxWidthChar, 60);
  int luxWidth = u8g.getStrWidth(luxWidthChar);
  u8g.setPrintPos(32 - (luxWidth/2), 128);
  u8g.print(lightLevelString);
}

int printLogLine(int currentLine, String text) {
  int lineHeight = 5;
  int horPosition = 0;
  for (int i = 0; i < text.length(); i++) {
    u8g.setPrintPos(horPosition, currentLine+lineHeight);
    u8g.print(text.charAt(i));
    if (text.charAt(i) == ' ') {
      horPosition += 3;
    }
    else {
      char character[2];
      text.substring(i,i+1).toCharArray(character, 2);
      horPosition += u8g.getStrWidth(character);
    }
    if (horPosition > 60) {
      horPosition = 0;
      currentLine += lineHeight + 1; 
    }
  }
  return currentLine + lineHeight + 4;
}

void drawSaunaGraph(int ypos) {
  int pixelY = 0;
  float saunaLow = 24;
  float saunaMax = 85;
  int graphHeight = 25;

  for (int i = 62 ; i > 0 ; i--) {
    float temperature = saunaHistory[(62-i)*2];
    if (temperature != 0) {
      pixelY = map(temperature, saunaLow, saunaMax, 0, graphHeight);
      if (temperature < saunaLow) pixelY = 0;
      if (temperature > saunaMax) pixelY = graphHeight;
      u8g.drawPixel(i, ypos-pixelY);
    }
  }
  
  u8g.drawLine(1, ypos-graphHeight, 0, ypos-graphHeight);
  u8g.drawLine(0, ypos-graphHeight, 0, ypos-graphHeight+2);
  u8g.drawLine(0, ypos-2, 0, ypos);
  u8g.drawLine(0, ypos, 1, ypos);
}

