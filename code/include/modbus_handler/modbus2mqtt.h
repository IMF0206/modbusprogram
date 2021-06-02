#ifndef MODBUS2MQTT_H_INCLUDED_
#define MODBUS2MQTT_H_INCLUDED_

#include "modbus.h"
#include <string>
#include <map>

class mqtt_json;
class mqtt_plat;
class db_helper;

class modbus2mqtt
{
public:
    modbus2mqtt();
    ~modbus2mqtt();
    std::string getmqttstr(std::string portid, std::map<std::string, float>datamap);

private:
    mqtt_json* m_mqttjson;
    db_helper* m_dbhelper;

};

#endif