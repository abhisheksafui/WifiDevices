  
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

class DeviceSettings: public IotService {

  String softApSSID;
  String softApPasswd;
  String hostname;
  String apSSID;
  String apPasswd;
  String mode; 
  String fileName;



/**
   * Constructed with name of file on SPIFFS that store the devce configuration
   */
  DeviceSettings(String file): IotService("Setting", "DEVICE_SETTING")
  { 
    fileName = file;
  }

  static DeviceSettings* instancePtr;

  bool parse() {
    
    File configFile = SPIFFS.open(fileName, "r");

    if (!configFile) {
      DEBUG.println("Failed to open config file");
      return false;
    } else {
      DEBUG.println("Config file loaded.");
    }

    size_t size = configFile.size();
    DEBUG.print("Size of config file: "); DEBUG.println((int)size);
    if (size > 1024)
    {
      DEBUG.println("ERROR: Config file too large.");
      return false;
    }

    std::unique_ptr<char[]> buf(new char[size] {});
    configFile.readBytes(buf.get(), size);
    DynamicJsonBuffer j_buff;
    JsonObject &root = j_buff.parseObject(buf.get());

    if (!root.success()) {
      DEBUG.println("ERROR parsing configuration.");
    }
    DEBUG.println("Configuration parsed sucessfully..");


    mode = root["mode"].as<char*>();
    softApSSID = root["softApSSID"].as<char*>();
    softApPasswd = root["softApPasswd"  ].as<char*>();
    apSSID = root["apSSID"].as<char*>();
    apPasswd = root[AP_PASSWD_TAG].as<char*>();
    hostname = root[HOSTNAME_TAG].as<char*>();

    dumpSettings();
    configFile.close();
    return true;

  }

   void createDefaultSettingsFile(){
    softApSSID = String("Device_") + ESP.getChipId();
      hostname = softApSSID;
      softApPasswd = "85128512";
      mode = "AP";    
      save();
  }
  
public:
  
  static DeviceSettings *instance(){
  
    if(instancePtr == NULL){
      instancePtr = new DeviceSettings(CONFIG_FILE);
    }

    return instancePtr;
    
    
  }

 

  void init()
  {    
    DEBUG.println("Device settings inittialising..");
    /** Read the device file if it exists otherwise create the file **/
    File configFile = SPIFFS.open(fileName,"r");
    if(!configFile)
    {
      DEBUG.println("Device settings file not found: " + fileName + " . Creating file");
      createDefaultSettingsFile();
    }
    else 
    {
      /** Read the settings **/
      if(parse() != true){
        DEBUG.println("Error parsing Device settings file. Creating default file..");
        createDefaultSettingsFile();   
      }
    }

    IotService::setHostname(hostname);
  }

  
  
  void save()
  {
    
    File configFile = SPIFFS.open(fileName, "w");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    root[SOFT_AP_SSID_TAG] = softApSSID;
    root[SOFT_AP_PASSWD_TAG] = softApPasswd; 
    root[AP_SSID_TAG] = apSSID;
    root[AP_PASSWD_TAG] = apPasswd; 
    root[HOSTNAME_TAG] = hostname;
    root[WIFI_MODE_TAG] = mode; 

    char buffer[512];
    root.printTo(buffer,sizeof(buffer));
    DEBUG.print("Saving configuration to file("); DEBUG.print(fileName); DEBUG.print(")");
    DEBUG.println(buffer);

    configFile.print(buffer);
    configFile.close();

    DEBUG.println("Saved new Device Settings..Restarting device..");

    ESP.reset();
  }

   void dumpSettings() {
    DEBUG.println("======= CONFIGURATION =======");
    DEBUG.println("  mode : " + mode);
    DEBUG.println("  softApSSID: " + softApSSID);
    DEBUG.println("  softApPasswd: " + softApPasswd);
    DEBUG.println("  apSSID: " + apSSID);
    DEBUG.println("  apPasswd: " + apPasswd);
    DEBUG.println("  hostname: " + hostname);
  }

  void setState(JsonObject& item)
  {
       
      JsonObject& stateObject = item[SERVICE_STATE];
      DEBUG.print("hostname: "); DEBUG.println(stateObject[HOSTNAME_TAG].as<char *>());
      instance()->setSoftApSSID(stateObject[SOFT_AP_SSID_TAG]);
      instance()->setSoftApPasswd(stateObject[SOFT_AP_PASSWD_TAG]);
      instance()->setApSSID(stateObject[AP_SSID_TAG]);
      instance()->setApPasswd(stateObject[AP_PASSWD_TAG]);
      instance()->setHostname(stateObject[HOSTNAME_TAG]);
      instance()->setMode(stateObject[WIFI_MODE_TAG]);

      commit();
  }

  void getState(JsonObject& serviceObject)
  {
     /**
      * Fill the json object
      */
      serviceObject[SERVICE_NAME] = getName();
      serviceObject[SERVICE_TYPE] = getType();

      JsonObject& stateObject = serviceObject.createNestedObject(SERVICE_STATE);
      stateObject[AP_SSID_TAG] = apSSID;
      stateObject[AP_PASSWD_TAG] = apPasswd;
      stateObject[SOFT_AP_SSID_TAG] = softApSSID;
      stateObject[SOFT_AP_PASSWD_TAG] = softApPasswd;
      stateObject[HOSTNAME_TAG] = hostname;
      stateObject[WIFI_MODE_TAG] = mode;
  }

  /** GETTERS **/
  String& getSoftApSSID(){
    return softApSSID;
  }

  String& getSoftApPasswd(){
    return softApPasswd;
  }

  String& getApSSID(){
    return apSSID;
  }

  String& getApPasswd(){
    return apPasswd;
  }

  String& getMode(){
    return mode;
  }

  String& getHostname(){
    return hostname;
  }

  /** SETTERS **/
  void setSoftApSSID(String &value){
    softApSSID = value; 
  }
  void setSoftApPasswd(String &value){
    softApPasswd = value; 
  }
  void setApSSID(String &value){
    apSSID = value; 
  }
  void setApPasswd(String &value){
    apPasswd = value;
  }
  void setMode(String &value){
    mode = value;
  }
  void setHostname(String &value){
    hostname = value;
  }

  void setSoftApSSID(const char *value){
    softApSSID = value; 
  }
  void setSoftApPasswd(const char *value){
    softApPasswd = value; 
  }
  void setApSSID(const char *value){
    apSSID = value; 
  }
  void setApPasswd(const char *value){
    apPasswd = value;
  }
  void setMode(const char *value){
    mode = value;
  }
  void setHostname(const char *value){
    hostname = value;
  }


  String& getHostname(String &value){
    return hostname;
  }


  static const String getWifiMode(){
    if(instance()){
      return instance()->getMode();
    }else{
      return String("");
    }
  }

  static void setWifiMode(String &m){
    if(instance()){
      instance()->setMode(m);
    }
  }

  static void setWifiMode(String &&m){
    if(instance()){
      instance()->setMode(m);
    }
  }

  static void commit(){
    if(instance()){
      instance()->save();
    }
  }

  constexpr static const char * SOFT_AP_SSID_TAG = "softApSSID";
  constexpr static const char * SOFT_AP_PASSWD_TAG = "softApPasswd";
  constexpr static const char * AP_SSID_TAG = "apSSID";
  constexpr static const char * AP_PASSWD_TAG = "apPasswd";
  constexpr static const char * HOSTNAME_TAG = "hostname";
  constexpr static const char * WIFI_MODE_TAG = "mode";
  
};

DeviceSettings* DeviceSettings::instancePtr = NULL;

