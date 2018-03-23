
/**
    Functions for setting up the device
*/

void spiffsInit() {
  DEBUG.print("Mounting SPIFFS..");
  if (!SPIFFS.begin()) {
    DEBUG.println("[ERR]");
    return;
  }
  DEBUG.println("[OK]");
}

void otaSetup() {
  DEBUG.println("OTA hostname: " + DeviceSettings::instance()->getHostname());
  ArduinoOTA.setHostname(DeviceSettings::instance()->getHostname().c_str());
  ArduinoOTA.begin();
}

void iotSwitchInit(){
//  File timerFile = SPIFFS.open("/timer","r");
//  if(!timerFile){
//    DEBUG.println("Timer file does not exist..");
//    return;
//  }
//  
//  int size = timerFile.size();
//  std::unique_ptr<char[]> buf(new char[size+1] {});
//  timerFile.readBytes(buf.get(),size);
//  buf.get()[size] = 0;
//  
//
//  DEBUG.print("Timer file read: "); DEBUG.println(buf.get()); 
//  iotSwitch.setTimer(buf.get());
//  
//
//  timerFile.close();
}

void gpioInit() {

  DEBUG.println("Initializing gpios..");

  DEBUG.println("[OK]");

}

/* UDP server setup function */

void udpServerInit() {
  DEBUG.print("Starting UDP server on port " + WiFi.localIP());
  DEBUG.print(":" + String(UDP_SERVER_LISTEN_PORT));
  DEBUG.print(" ");
  if (udp.listen(UDP_SERVER_LISTEN_PORT)) {
    DEBUG.println("[OK]");
    udp.onPacket([](AsyncUDPPacket packet) {
      DEBUG.print("UDP Packet Type: ");
      DEBUG.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
      DEBUG.print(", From: ");
      DEBUG.print(packet.remoteIP());
      DEBUG.print(":");
      DEBUG.print(packet.remotePort());
      DEBUG.print(", To: ");
      DEBUG.print(packet.localIP());
      DEBUG.print(":");
      DEBUG.print(packet.localPort());
      DEBUG.print(", Length: ");
      DEBUG.print(packet.length());
      DEBUG.print(", Data: ");
      DEBUG.write(packet.data(), packet.length());
      DEBUG.println();

      IotService::process_command(packet);
      //packet.write((uint8_t *)"OK DONE\n",8);
      //udp.writeTo((uint8_t*)"OK\n",3,packet.remoteIP(),packet.remotePort());
    });


  } else {
    DEBUG.println("[ERR]");
  }
}

/**
   Wifi configuration functions
*/

void connect_to_ap(const String &ap, const String &pass) {

  DEBUG.println("Connecting to " + ap);
  WiFi.begin(ap.c_str(), pass.c_str());

}

void start_soft_ap(const String &ap, const String &pass) {
  WiFi.softAP(ap.c_str(), pass.c_str());
  DEBUG.println("Access Point Started, SSID: " + ap +
                " Password: " + pass);
}
void wifiInit(DeviceSettings* settings) {

  if ( settings->getMode() == "STA_AP") {
    /* Setup Hotspot(access point) */
    WiFi.mode(WIFI_AP_STA);
  } else if (settings->getMode() == "STA") {
    WiFi.mode(WIFI_STA);
  } else if (settings->getMode() == "AP") {
    WiFi.mode(WIFI_AP);
  }

  if (settings->getMode() == "STA" || settings->getMode() == "STA_AP") {
    connect_to_ap(settings->getApSSID(), settings->getApPasswd());
    /* Setup handlers to log important events */
    onConnectHandler = WiFi.onStationModeConnected(&onConnected);
    onDisconnectHandler = WiFi.onStationModeDisconnected(&onDisconnected);
  }

  if (settings->getMode() == "AP" || settings->getMode() == "STA_AP") {
    start_soft_ap(settings->getSoftApSSID(), settings->getSoftApPasswd());
  }
}

/**
   Wifi Event handlers
*/

void onDisconnected(const WiFiEventStationModeDisconnected& event)
{
  Serial.println("Wifi Disconnected from accesspoint " + event.ssid +
                 ". Reason [" + event.reason + "].");
  //connectWifi(ssid,passwd);
  AppConfig::wifi_disconnected = 1;
}



void onConnected(const WiFiEventStationModeConnected& event) {
  AppConfig::wifi_disconnected = 0;
  Serial.println("Wifi connected to accespoint.");
  Serial.println("  SSID: " + event.ssid);
  char bssid[100];
  sprintf(bssid, "%02x:%02x:%02x:%02x:%02x:%02x", event.bssid[0],
          event.bssid[1],
          event.bssid[2],
          event.bssid[3],
          event.bssid[4],
          event.bssid[5]);
  Serial.print("  BSSID: " ); Serial.println(bssid);
  Serial.println("  Channel: " + event.channel);
}

