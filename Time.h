#ifndef __TIME_H__
#define __TIME_H__
  
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


#include "IotService.h"


enum class TimerState {
  RUNNING,
  STOPPED,
};

class Timer: public IotService {

    static ArduinoList<Timer*> timer_list;
    static int update_ts_ms;

    /**
        timer set values
    */

    int         set_hours;
    int         set_minutes;
    int         set_seconds;

    /**
        timer running values
    */

    int         hours;
    int         minutes;
    int         seconds;
    TimerState  _state;


    std::function<void(void *)> callback;
    void *arg;

  public:

    Timer(String& name) : IotService(name, "TIMER")
    {

      timer_list.push_back(this);
      _state = TimerState::STOPPED;
    }

    /**
       Function to load all timer's states sequentially.
    */
    static void init() {
      for (auto it = timer_list.begin(); it != timer_list.end(); it++) {
        (*it)->initTimer();
      }
    }

    /**
       Function to load timer value from file.

    */
    void initTimer() {

      String fileName = getFileName();

      File timerFile = SPIFFS.open(fileName.c_str(), "r");
      if (!timerFile) {
        DEBUG.println(fileName + ": Timer file does not exist..");
        return;
      }

      int size = timerFile.size();
      if (size > 256 || size <= 0) {
        /**
           Size can never be more than 256
        */
        DEBUG.print("Unexpected size: "); DEBUG.println(size);
        return;
      }

      DEBUG.print(fileName + " Size: "); DEBUG.println(size);
      std::unique_ptr<char[]> buf(new char[size + 1] {});
      timerFile.readBytes(buf.get(), size);
      buf.get()[size] = 0;


      DEBUG.print("Timer file read: "); DEBUG.println(buf.get());
      setTimerState(buf.get());
      timerFile.close();

    }
    /**
       Set timer value and register timer pop callback.
       Usefull for initializing a timer.
    */

    void set(int hour, int minute, int second, std::function<void(void *)> call, void * data = NULL)
    {

      callback = call;
      arg = data;

      set(hour, minute, second);

    }

    /**
       Set timer values.
       Usefull for seting timer after timer has been initialized.
    */
    void set(int hour, int minute, int second) {

      set_hours = hour;
      set_minutes = minute;
      set_seconds = second;

      if(hour == 0 && minute == 0 && second == 0)
      {
        _state = TimerState::STOPPED;
      }

      if (state() == TimerState::RUNNING) {
        /** If timer is running, update the running state also
        */
        hours = hour;
        minutes = minute;
        seconds = second;
      }
    }

    /**
       Start the timer into running state
    */
    void start() {

      if ((set_hours + set_minutes + set_seconds) == 0) {
        DEBUG.println("Timer not set. So cannot start..");
        return;
      }

      hours = set_hours;
      minutes = set_minutes;
      seconds = set_seconds;

      _state = TimerState::RUNNING;

    }

    /**
       Set the timer to STOPPED state
    */
    void stop() {
      _state = TimerState::STOPPED;
    }

    void decrementSecond() {

      if ( seconds <= 0 ) {
        decrementMinute();
      }

      seconds--;
    }

    void decrementMinute() {

      if (minutes < 0) {  
        decrementHour();
      }

      seconds = 59;
      minutes--;

    }

    void decrementHour() {

      if (hours < 0) {
        return;
      }

      minutes = 59;
      hours--;

    }

    void check() {

      if (hours <= 0 && minutes <= 0 && seconds <= 0) {

        _state = TimerState::STOPPED;
        DEBUG.println("Timer popped. Calling callback and marking it STOPPED.");
        callback(arg);

      }
    }

    /**
       Should be called in loop, frequently
    */
    static void update() {

      if (millis() - update_ts_ms >= 1000) {
        /**
            Update every second
        */
        update_ts_ms = millis();

        for (auto it = timer_list.begin(); it!= timer_list.end(); it++) {

          if ((*it)->state() == TimerState::RUNNING) {

            /* Decrement Second and check if timer has popped */
            (*it)->decrementSecond();
            (*it)->check();
          }

        }
      }
    }




    int  addSecond() {
      seconds++;
      if (seconds == 60) {
        seconds = 0;
        addMinute();
      }
    }

    int addMinute() {

      minutes++;
      if (minutes == 60) {
        minutes = 0;
        addHour();
      }
      return minutes;

    }

    int addHour() {
      return hours++;
    }

    int getSeconds() {
      return seconds;
    }

    int getMinutes() {
      return minutes;
    }

    int getHours() {
      return hours;
    }

    /**
       Overloaded virtual functions
    */

    void getState(JsonObject &serviceObject) {


      serviceObject[SERVICE_NAME] = getName();
      serviceObject[SERVICE_TYPE] = getType();
      serviceObject[SERVICE_STATE] = toString();
    }

    void setState(JsonObject& serviceObject) {
      /**
         State is of the form HH:MM:SS
      */
      String state = serviceObject[SERVICE_STATE];
      setTimerState(state);

      /**
         Save state in file
      */
      String fileName = getFileName();
      File timerFile = SPIFFS.open(fileName.c_str(), "w");
      if (!timerFile) {
        DEBUG.println("Error opening timer file..");
      } else {
        timerFile.println(state);
        timerFile.close();
      }
    }



  private:

    void setTimerState(String state) {
      char buffer[100];
      strncpy(buffer, state.c_str(), sizeof(buffer));
      int hour;
      int minute;
      int second;

      char *hr_ptr = strtok(buffer, ":");
      char *min_ptr = strtok(NULL, ":");
      char *sec_ptr = strtok(NULL, " \0");

      hour = atoi(hr_ptr);
      minute = atoi(min_ptr);
      second = atoi(sec_ptr);

      DEBUG.println("Setting iot timer->.");
      DEBUG.print("Hours = "); DEBUG.println(hour);
      DEBUG.print("Minutes = "); DEBUG.println(minute);
      DEBUG.print("Seconds = "); DEBUG.println(second);

      /* Set the timer requested time */
      /* It will be loaded when turned on */
      set(hour, minute, second);
    }

    String toString() {
      char buffer[100];
      if (state() == TimerState::RUNNING) {
        snprintf( buffer, sizeof(buffer), "%d:%d:%d RUNNING %d:%d:%d", set_hours,
                  set_minutes,
                  set_seconds,
                  hours,
                  minutes,
                  seconds);
      } else {
        snprintf( buffer, sizeof(buffer), "%d:%d:%d IDLE", set_hours,
                  set_minutes,
                  set_seconds);
      }
      return buffer;
    }

    String getFileName() {
      /**
         Every IotSwitch has a file named /<name>_timer that stores the value of timer
      */
      String fileName("/" + getName());
      return fileName;
    }

    TimerState state() {
      return _state;
    }

    void load(int hour, int minute, int second, std::function<void(void *)> call, void * data = NULL)
    {

      callback = call;
      arg = data;

      set(hour, minute, second);

      start();

    }
};

ArduinoList<Timer*>  Timer::timer_list;
int     Timer::update_ts_ms = 0;

#endif
