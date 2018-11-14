

void refreshTime() {
  if (RTC.read(date)) {
    h_string = String(date.Hour);
    if (date.Hour < 10) {
      h_string = "0" + h_string;
    }
    
    m_string = String(date.Minute);
    if (date.Minute < 10) {
      m_string = "0" + m_string;
    }
    
    d_string = String(date.Day);
    mon_string = String(date.Month);
    y_string = String(date.Year);
    
    switch (date.Wday) {
      case 2: day_string = "maanantai"; break;
      case 3: day_string = "tiistai"; break;
      case 4: day_string = "keskiviikko"; break;
      case 5: day_string = "torstai"; break;
      case 6: day_string = "perjantai"; break;
      case 7: day_string = "lauantai"; break;
      case 1: day_string = "sunnuntai"; break;
      default: day_string = "..."; break;
    }
    
    if (date.Hour == 4 && date.Minute == 15 && date.Second == 05) {
      timeToUpdateTime = true;
      delay(2000);
    }
  }
}

void syncTimeToNTP() {
  if (RTC.read(date)) {
    time_t clockTime = makeTime(date);
    long ntpTime = getTime();
    time_t local = FI.toLocal(ntpTime, &tcr);
    
    logger(String(hour(local)) + ":" + String(minute(local)) + "." + String(second(local)) + " adjusting time by: " + String(local - clockTime));
    
    RTC.set(local);
  }     
  timeToUpdateTime = false;
}

unsigned long getTime(void) {
  uint8_t       buf[48];
  unsigned long ip, startTime, t = 0L;

  logger("Locating time server...");

  // Hostname to IP lookup; use NTP pool (rotates through servers)
  if(cc3000.getHostByName("pool.ntp.org", &ip)) {
    static const char PROGMEM
      timeReqA[] = { 227,  0,  6, 236 },
      timeReqB[] = {  49, 78, 49,  52 };

    logger("Attempting connection...");
    startTime = millis();
    do {
      client = cc3000.connectUDP(ip, 123);
    } while((!client.connected()) &&
            ((millis() - startTime) < connectTimeout));

    if(client.connected()) {
      logger("connected! Issuing request...");

      // Assemble and issue request packet
      memset(buf, 0, sizeof(buf));
      memcpy_P( buf    , timeReqA, sizeof(timeReqA));
      memcpy_P(&buf[12], timeReqB, sizeof(timeReqB));
      client.write(buf, sizeof(buf));

      logger("Awaiting response...");
      memset(buf, 0, sizeof(buf));
      startTime = millis();
      while((!client.available()) &&
            ((millis() - startTime) < responseTimeout));
      if(client.available()) {
        client.read(buf, sizeof(buf));
        t = (((unsigned long)buf[40] << 24) |
             ((unsigned long)buf[41] << 16) |
             ((unsigned long)buf[42] <<  8) |
              (unsigned long)buf[43]) - 2208988800UL;
        logger("OK, got time");
      }
      client.close();
    }
  }
  if(!t) logger("Error getting time");
  return t;
}  
