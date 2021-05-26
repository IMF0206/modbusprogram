#ifndef MQTT_PLAT_PARSE_H_INCLUDED_
#define MQTT_PLAT_PARSE_H_INCLUDED_

#include "mqtt_client.h"
#include "sqlite_helper.h"
#include <map>

class mqtt_plat_parse
{
public:
    mqtt_plat_parse();
    virtual ~mqtt_plat_parse();
    int mqtt_plat_parse_add_rsp(std::string jsonstr);
    int mqtt_plat_parse_update_rsp(std::string jsonstr);
    int mqtt_plat_parse_del(std::string jsonstr);
    int mqtt_plat_parse_query_rsp(std::string jsonstr);
    int mqtt_plat_parse_command(std::string jsonstr);

private:
    std::string m_json_str();
    mqtt_json* m_json;
    db_helper* m_dbhelper;
    std::string m_command_str;
    std::map<std::string, int> mappara;
    std::string m_comm_deviceid;
    std::string m_comm_serviceid;
};

#endif