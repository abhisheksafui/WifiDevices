


/*
 * Class representing notification from Device.
 * Other devices may register for Device Notification.
 */

class Notification {

  char *_message;
  uint16_t _len;

  public:
  typedef std::shared_ptr<Notification> Ptr;
  const uint8_t * getMessage(){
    return (uint8_t *)_message;
  }
  uint16_t getLength(){
    return _len;
  }
};


class NotificationListener {


  IPAddress _ip;
  uint16_t _port; 
  uint32_t _timestamp;
  String   _device_id;
  
  static const uint32_t MAX_TIME  = 50000;//60 seconds  
  public:

  typedef std::shared_ptr<NotificationListener> Ptr;
  NotificationListener(String id, IPAddress ip, uint16_t port)
  : _device_id(id), _ip(ip), _port(port)
  {
    _timestamp = millis();
  }

  IPAddress& getIP(){
    return _ip;
  }
  
  uint16_t getPort(){
    return _port;
  }

  void notify(Notification *notif, AsyncUDP &udp){
    AsyncUDPMessage message;
    message.write(notif->getMessage(), notif->getLength());
    udp.sendTo(message, getIP(), getPort());
  }

  bool expired(){
    uint32_t now = millis();
    if(now > _timestamp){
      if((now - _timestamp) > MAX_TIME ){
        return true;
      }
    }else {
      if(~(_timestamp - now) > MAX_TIME){
        return true;
      }
    }

    return false;
  }

  void updateTimestamp(){
    _timestamp = millis();
  }

  String& getDeviceID(){
    return _device_id;
  }


  static uint32_t getTimeout(){
    return MAX_TIME;
  }
  
};
