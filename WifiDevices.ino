


/**
*    WiFi Devices, firmware for connecting devices to already available 
*		android applicaion.
*
*    Copyright (C) 2018  Abhishek Safui
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <https://www.gnu.org/licenses/>
*/
#include <ArduinoSwitch.h>
#include <FS.h>
#include<ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "global.h"
#include "Led.h"
#include<ESP8266mDNS.h>
#include <ESPAsyncUDP.h>
#include "IotSwitch.h"
#include "DeviceSettings.h"
#include "Time.h"

/**
 *  Global variables
 */
 
/* Wifi event handlers are called as long as handler objects are in scope: hence global */
WiFiEventHandler onConnectHandler;
WiFiEventHandler onDisconnectHandler;

AsyncUDP udp;



LED system_led(2,ACTIVE_LOW);

/** Iot switches will be created dynamically, only after reading their names from the file **/
std::shared_ptr<IotSwitch> iotSwitch1;
std::shared_ptr<IotSwitch> iotSwitch2;

int led_blink_time = 1000;



void interrupt(){
  DEBUG.println("Interupt");
}

void wifiModeSwitchPressCallback(void *){
  /**
   * Toggle the wifi mode and reset
   */
   if(DeviceSettings::getWifiMode() == "STA"){
      DeviceSettings::setWifiMode("AP");
   }else if (DeviceSettings::getWifiMode() == "AP"){
      DeviceSettings::setWifiMode("STA");
   }else if (DeviceSettings::getWifiMode()== "STA_AP")
   {
     DeviceSettings::setWifiMode("AP");
   }
   DeviceSettings::commit();
   
}

/**
 * Switch used to toggle between AP and STA modes
 */
ArduinoSwitch wifiModeControlSwitch(0,ActiveMode::ACTIVE_LOW,wifiModeSwitchPressCallback);

/**
 *  Setup function
 */
void setup() {
  
  /* Enable Serial Logging First */
  DEBUG.begin(115200);

  DEBUG.println("\n\n\n\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\n");
  DEBUG.println("Esp8266 Application booting..");

  ArduinoSwitch::init(&DEBUG);

  /* Read Configuration Second */
  spiffsInit();

  /** Device settings should be initialised at very begining **/
  DeviceSettings::instance()->init();

  if(DeviceSettings::instance()->getMode() == "AP")
  {
    /** Dont blink remain on **/
    led_blink_time = 0;
  }
  else if(DeviceSettings::instance()->getMode() = "STA")
  {
    led_blink_time = 1000;
  }
  else
  {
    led_blink_time = 200;
  }

  iotSwitch1 = std::make_shared<IotSwitch> (13, ACTIVE_HIGH,4,ActiveMode::ACTIVE_LOW,  "SW1");
  iotSwitch2 = std::make_shared<IotSwitch> (12, ACTIVE_HIGH,5,ActiveMode::ACTIVE_LOW,  "SW2");

  iotSwitch1->initIotSwitch();
  iotSwitch2->initIotSwitch();
  
  /* Setup Wifi based on configuration */
  wifiInit(DeviceSettings::instance());



  /* Init gpios before opening udp command channel */
  gpioInit();
  /* Start UDP server for commands */
  udpServerInit();

  /* Setup ota based on read configuration */
  otaSetup();


  /* Advertise yourself as iot device on the network */
  MDNS.addService("iot","udp",UDP_SERVER_LISTEN_PORT);
  MDNS.addServiceTxt("iot", "udp", "name", DeviceSettings::instance()->getHostname());
  DEBUG.println("Esp8266 Boot complete..");

}

void loop() {
  // put your main code here, to run repeatedly:
  
  if(led_blink_time && millis()-AppConfig::led_update_ts > led_blink_time){
    AppConfig::led_update_ts = millis();
    system_led.toggle(); 
  }else if(led_blink_time == 0){
    system_led.ON();
  }

  
  Timer::update();
  ArduinoSwitch::update();
  IotService::update(udp);
  
  ArduinoOTA.handle();
}
