#ifndef __IOT_SWITCH__
#define __IOT_SWITCH__

/**
     WiFi Devices, firmware for connecting devices to already available
		android applicaion.

     Copyright (C) 2018  Abhishek Safui

     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <https://www.gnu.org/licenses/>
*/


#include "Time.h"
#include  "IotService.h"

#define ON_STATE "ON"
#define OFF_STATE "OFF"



/**
   Functions inherited from LED:
    state() to get the LED state, which is same as this IotSwitch's state
*/

class IotSwitch: public LED, public IotService {

    static ArduinoList<IotSwitch *> _iotSwitches;

    std::shared_ptr<Timer> timer;
    std::shared_ptr<ArduinoSwitch> sw;

  public:


    IotSwitch(int pin, uint8_t active_mode, int inPin, ActiveMode inPin_active_mode, String sw_name): LED(pin, active_mode), IotService(sw_name, "SWITCH")
    {

      /**
         Create the timer variable
      */
      timer = std::make_shared<Timer>(sw_name + "_Timer");

      _iotSwitches.push_back(this);
      sw = std::make_shared<ArduinoSwitch>(inPin, inPin_active_mode, hardwareSwitchCallback, this);

      /* set the timer callback only */
      /* Constructor resets the timer to 0. Later timer init will load any set value from SPIFFS file */
      timer->set(0, 0, 0, switchCallback, this);
    }

    static void hardwareSwitchCallback(void *arg) {

      IotSwitch *sPtr = (IotSwitch *)arg;
      sPtr->toggle();

    }


    void initIotSwitch() {
      timer->initTimer();
    }


    void ON() {

      LED::ON();
      timer->start();

    }

    void OFF() {
      LED::OFF();
      timer->stop();
    }


    /* timer callback function. When timer expires this operation is donw */

    static void switchCallback(void *arg) {
      IotSwitch *sw = (IotSwitch *)arg;
      sw->OFF();
    }

    /**
       Functions overloaded from IotService

    */

    void setState(JsonObject& serviceObject) {

      String value = serviceObject[SERVICE_STATE];
      if (value == ON_STATE) {
        ON();
      } else if (value == OFF_STATE) {
        OFF();
      }

    }

    void getState(JsonObject& serviceObject) {

      serviceObject[SERVICE_NAME] = getName();
      serviceObject[SERVICE_TYPE] = getType();
      /**
         Get the underlying LED state
      */
      if (state() == HIGH) {
        serviceObject[SERVICE_STATE] = ON_STATE;
      }
      else if (state() == LOW) {

        serviceObject[SERVICE_STATE] = OFF_STATE;

      }
    }


};

ArduinoList<IotSwitch *> IotSwitch::_iotSwitches;
#endif
