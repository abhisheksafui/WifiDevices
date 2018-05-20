#include <List.h>

#ifndef _IOT_SERVICE_H_
#define _IOT_SERVICE_H_

#include "notification.h"

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


/**
   IOT Service base class for all types of Iot Services like iotSwitch.

   It implements getState, setState, process_message command methods
*/



class IotService {

  public:

    /* JSON MESSAGE KEYS */

    constexpr static const char * const MESSAGE_TYPE_TAG = "MSG_TYPE";
    constexpr static const char * const SERVICE_NAME =     "SERVICE_NAME";
    constexpr static const char * const SERVICE_TYPE =     "SERVICE_TYPE";
    constexpr static const char * const SERVICE_ARRAY =    "SERVICES";
    constexpr static const char * const SERVICE_STATE =    "SERVICE_STATE";
    constexpr static const char * const DEVICE_ID_TAG =  "DEVICE_ID";
  private:

    /* COMMMAND TYPES */
    constexpr static const char *MESSAGE_TYPE_SERVICE_REQUEST = "SERVICES_REQUEST";
    constexpr static const char *MESSAGE_TYPE_SERVICE_REPLY = "SERVICES_REPLY";
    constexpr static const char *MESSAGE_TYPE_GET_REQUEST = "GET_REQUEST";
    constexpr static const char *MESSAGE_TYPE_GET_RESPONSE = "GET_RESPONSE";
    constexpr static const char *MESSAGE_TYPE_SET_REQUEST = "SET_REQUEST";
    constexpr static const char *MESSAGE_TYPE_SET_RESPONSE = "SET_RESPONSE";
    constexpr static const char *MESSAGE_TYPE_REGISTER_NOTIFICATION_REQUEST  = "REGISTER_NOTIFICATION_REQUEST";
    constexpr static const char *MESSAGE_TYPE_REGISTER_NOTIFICATION_RESPONSE  = "REGISTER_NOTIFICATION_RESPONSE";
    constexpr static const char *MESSAGE_TYPE_REGISTER_NOTIFICATION_REMOVED  = "REGISTER_NOTIFICATION_REMOVED";
    constexpr static const char *NOTIF_TIMEOUT_TAG = "NOTIF_TIMEOUT";




  public:


    IotService(String name, String type): _type(type), _name(name) {
      iotServices.push_back(this);
    }

    ~IotService(){
      iotServices.remove(this);
    }
    
    String& getType() {
      return _type;
    }

    String& getName() {
      return _name;
    }

    /**
       Must be overridden.
    */
    /** getState is expected to fill SERVICE_NAME, SERVICE_TYPE and SERVICE_STATE objects **/
    virtual void getState(JsonObject& serviceObject) = 0;

    /** setState gets the full serviceState object extracted from array in SET_REQUEST **/
    virtual void setState(JsonObject& serviceObject) = 0;

    /**
       Common service array function to reply to MESSAGE_TYPE_SERVICE_REQUEST
    */
    virtual void appendService(JsonArray& serviceArray) {
      JsonObject& serviceObject = serviceArray.createNestedObject();

      serviceObject[SERVICE_NAME] = getName();
      serviceObject[SERVICE_TYPE] = getType();

    }

    virtual void appendServiceState(JsonArray& serviceArray) {
      JsonObject& serviceObject = serviceArray.createNestedObject();
      getState(serviceObject);
    }

    static void process_command(AsyncUDPPacket &packet) {

      char cmd[MAX_COMMAND_LENGTH + 1];
      char buffer[MAX_REPLY_LENGTH + 1];

      memcpy(cmd, packet.data(), packet.length());
      cmd[packet.length()] = 0;

      /**
         Parse the Json message.
         Get the message type.
         Get the services array.
         For each service match the type and name and call getState or setState based on message type
      */


      DynamicJsonBuffer jsonBuffer;
      DynamicJsonBuffer replyBuffer;
      JsonObject& root = jsonBuffer.parseObject(cmd);
      JsonObject& reply = replyBuffer.createObject();

      if (!root.success())
      {
        DEBUG.println("Parsing .. [ERROR]");
        return;
      }
      else {
        DEBUG.println("Parsing .. [OK]. Continuing");
      }

      const char *msgType = root[MESSAGE_TYPE_TAG];
      DEBUG.println(msgType);

      reply[DEVICE_ID_TAG] = hostname;

      if (!strcmp(msgType, MESSAGE_TYPE_SERVICE_REQUEST))
      {
        DEBUG.println("Request for Services");
        reply[MESSAGE_TYPE_TAG] = MESSAGE_TYPE_SERVICE_REPLY;

        /** Create the service array **/
        JsonArray& serviceArray = reply.createNestedArray(SERVICE_ARRAY);

        for(auto it = iotServices.begin(); it != iotServices.end(); it++)
        {
          (*it)->appendService(serviceArray);
        }
        
        reply.printTo(buffer, sizeof(buffer));
        DEBUG.print("Replying message: "); DEBUG.println(buffer);
        packet.write((const uint8_t *)buffer, strlen(buffer));
      }
      else if (!strcmp(msgType, MESSAGE_TYPE_GET_REQUEST))
      {
        reply[MESSAGE_TYPE_TAG] = MESSAGE_TYPE_GET_RESPONSE;

        /** Get the array of services to respond to **/
        JsonArray& reqArray = root[SERVICE_ARRAY];
        JsonArray& serviceArray = reply.createNestedArray(SERVICE_ARRAY);

        for (auto item : reqArray)
        {
          for(auto it = iotServices.begin(); it != iotServices.end(); it++)
          {
            if (!strcmp(item[SERVICE_NAME], (*it)->getName().c_str()) &&
                !strcmp(item[SERVICE_TYPE], (*it)->getType().c_str()))
            {
              (*it)->appendServiceState(serviceArray);
            }
          }
        }
        reply.printTo(buffer, sizeof(buffer));
        DEBUG.print("Replying message: "); DEBUG.println(buffer);
        packet.write((const uint8_t *)buffer, strlen(buffer));
      }
      else if (!strcmp(msgType, MESSAGE_TYPE_SET_REQUEST))
      {
        reply[MESSAGE_TYPE_TAG] = MESSAGE_TYPE_SET_RESPONSE;

        /** Get the array of services to respond to **/
        //DEBUG.print("State = "); DEBUG.println(state);
        JsonArray& reqArray = root[SERVICE_ARRAY];
        JsonArray& serviceArray = reply.createNestedArray(SERVICE_ARRAY);

        for (auto item : reqArray)
        {
          for(auto it = iotServices.begin(); it != iotServices.end(); it++)
          {
            if (!strcmp(item[SERVICE_NAME], (*it)->getName().c_str()) &&
                !strcmp(item[SERVICE_TYPE], (*it)->getType().c_str()))
            {
              (*it)->setState(item);
              (*it)->appendServiceState(serviceArray);
            }
          }
        }
        reply.printTo(buffer, sizeof(buffer));
        DEBUG.print("Replying message: "); DEBUG.println(buffer);
        packet.write((const uint8_t *)buffer, strlen(buffer));

      } else if (!strcmp(msgType, MESSAGE_TYPE_REGISTER_NOTIFICATION_REQUEST)) {
        /* just store the ip and port to deliver notification to this device*/
        

        bool listenerExists = false;
        notificationListenerList.for_each([&listenerExists, &reply, &packet](NotificationListener::Ptr ptr){
          if(ptr->getIP() == packet.remoteIP() && ptr->getPort() == packet.remotePort() && ptr->getDeviceID() == reply[DEVICE_ID_TAG]){
            DEBUG.println("Listener already exists. Upating timestamp.");
            ptr->updateTimestamp();
            listenerExists =true;
          }
        });

        if(listenerExists == false){
          std::shared_ptr<NotificationListener> listener = std::make_shared<NotificationListener>(reply[DEVICE_ID_TAG], packet.remoteIP(), packet.remotePort());
          notificationListenerList.push_back(listener);
        }
        
        
        reply[MESSAGE_TYPE_TAG] = MESSAGE_TYPE_REGISTER_NOTIFICATION_RESPONSE;
        reply[NOTIF_TIMEOUT_TAG] = NotificationListener::getTimeout();
        reply.printTo(buffer, sizeof(buffer));
        DEBUG.print("Replying message: "); DEBUG.println(buffer);
        packet.write((const uint8_t *)buffer, strlen(buffer));
      }


    }

    static void setHostname(String& name)
    {
      hostname = name;
    }

    static void update(AsyncUDP &udp) {
      /** check if notifications pending. If so post to all registered receivers **/
      while (!notificationList.isEmpty()) {
        auto notif = notificationList.pop_front();

        for (auto it = notificationListenerList.begin(); it != notificationListenerList.end(); it++) {
          (*it)->notify(notif.get(), udp);
        }
      }

      /** check for expired Listeners. Remove them **/
      notificationListenerList.remove_if([&udp](NotificationListener::Ptr ptr) {
        
        if (ptr->expired()) {
          DEBUG.println("notificationListenerExpired. Removing notificationListener and informing it");
          DynamicJsonBuffer replyBuffer;
          char buffer[512];
          JsonObject& reply = replyBuffer.createObject();
          
          reply[MESSAGE_TYPE_TAG] = MESSAGE_TYPE_REGISTER_NOTIFICATION_REMOVED;
          reply[DEVICE_ID_TAG] = hostname;
          
          reply.printTo(buffer, sizeof(buffer));
          DEBUG.print("Sending message: "); DEBUG.println(buffer);
          
          udp.writeTo((uint8_t *)buffer, strlen(buffer), ptr->getIP(), ptr->getPort());
          return true;
        }

        return false;
      });
    }

  private:

    static const uint32_t MAX_COMMAND_LENGTH = 1024;
    static const uint32_t MAX_REPLY_LENGTH = 1024;
    static const uint32_t MAX_IOT_SERVICES = 20;

    static ArduinoList<IotService *> iotServices;

    static String hostname;
    static uint32_t iotServiceCount;

    static ArduinoList<std::shared_ptr<Notification>> notificationList;
    static ArduinoList<std::shared_ptr<NotificationListener>> notificationListenerList;

    String _type;
    String _name;
};

ArduinoList<IotService *> IotService::iotServices;
String IotService::hostname;
ArduinoList<std::shared_ptr<Notification>> IotService::notificationList;
ArduinoList<std::shared_ptr<NotificationListener>> IotService::notificationListenerList;

#endif
