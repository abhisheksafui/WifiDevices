#ifndef __SWITCH_H__
#define __SWITCH_H__
  
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


#define MAX_SWITCHES 3
#define DEBOUNCE_TIME_MS 100
#define REPEATED_PRESS_INTERVAL_MS 700

#if 1

enum class ActiveMode {

  ACTIVE_HIGH,
  ACTIVE_LOW,

};

enum class SwitchFsmState {

  IDLE,
  DEBOUNCE_START,
  PRESSED,

};


class Switch {

    static Switch*   _switches[MAX_SWITCHES];
    static int      _switchCount;
    static bool     _init;

    /**
        Switch initialisation variables
    */
    ActiveMode      _mode;
    int _pin;
    std::function<void(void *)> _callback;
    void *_callbackArg;

    /**
       Runtime variables
    */
    SwitchFsmState  _state;
    unsigned int _state_timestamp;

    /**
       Switch private methods
    */

    SwitchFsmState state() {

      return _state;
    }

    unsigned int stateUpdateTime() {
      return _state_timestamp;
    }

  public:
    Switch(int pin, ActiveMode mode, std::function<void(void *)> call, void *arg = nullptr): _callback(call), _callbackArg(arg), _pin(pin), _mode(mode){

      if ( _init == false ) {
        _switchCount = 0;
        _init = true;
      }

      _switches[_switchCount] = this;
      _switchCount++;

      pinMode(_pin,INPUT_PULLUP);
      attachInterrupt(_pin, interrupt, (mode == ActiveMode::ACTIVE_HIGH) ? RISING : FALLING);

      DEBUG.print("Switch CONSTRUCTOR called. Count = ");
      DEBUG.println(_switchCount);

    }

    bool pressed() {
      int state = digitalRead(_pin);
      //DEBUG.print("State = "); DEBUG.println(state);
      if (_mode == ActiveMode::ACTIVE_HIGH ) {

        return state == HIGH;

      } else {

        return state == LOW;
      }

      
    }

    static void interrupt() {

      DEBUG.println("Interrupted");
      for (int i = 0; i < _switchCount; i++) {
        if (_switches[i]->state() == SwitchFsmState::IDLE &&
            _switches[i]->pressed() ) {

          DEBUG.println("Switch debounce started.. ");
          _switches[i]->_state = SwitchFsmState::DEBOUNCE_START;
          _switches[i]->_state_timestamp = millis();

        }

      }

    }

    static void update() {
      /* Check if any switch needs to be checked */
      //DEBUG.print("Switch update called. Count = ");      DEBUG.println(_switchCount);

      for (int i = 0; i < _switchCount; i++) {

        if (_switches[i]->pressed()) {
          if ( (_switches[i]->state() == SwitchFsmState::DEBOUNCE_START) &&
               ( millis() - _switches[i]->stateUpdateTime() > DEBOUNCE_TIME_MS ) ) {

            /* Switch pressed so call callback */
            _switches[i]->_state = SwitchFsmState::PRESSED;
            _switches[i]->_state_timestamp = millis();
            _switches[i]->_callback(_switches[i]->_callbackArg);

          } else if ( (_switches[i]->state() == SwitchFsmState::PRESSED) &&
                      ( millis() - _switches[i]->stateUpdateTime() > REPEATED_PRESS_INTERVAL_MS) ) {

            /* Switch is kept pressed */
            _switches[i]->_state_timestamp = millis();
            _switches[i]->_callback(_switches[i]->_callbackArg);
          }
        } else {

          _switches[i]->_state = SwitchFsmState::IDLE;
          _switches[i]->_state_timestamp = millis();
        }

      }

    }
};

bool Switch::_init = false;
Switch*   Switch::_switches[MAX_SWITCHES];
int       Switch::_switchCount = 0;

#endif
#endif
