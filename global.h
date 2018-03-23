#ifndef GLOBAL_H
#define GLOBAL_H
/**
*    WiFi Devices, firmware for connecting devices to already available 
*    android applicaion.
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

/**
    Global Definitions
*/
#define DEBUG Serial
#define UDP_SERVER_LISTEN_PORT 1234
#define SWITCH_PIN 4

#define CONFIG_FILE "/config.conf"

/*  Global application data */
struct AppConfig {


  static bool wifi_disconnected;

  static unsigned int led_update_ts;

};

bool AppConfig::wifi_disconnected = true;
unsigned int AppConfig::led_update_ts = 0;



/**
   Global variables
*/

extern WiFiEventHandler onConnectHandler;
extern WiFiEventHandler onDisconnectHandler;

class Config;

/**
   Function Declarations
*/

void spiffsInit();
void wifiInit(const Config& conf);
void udpServerInit();
void gpioInit();
void otaSetup();

#endif
