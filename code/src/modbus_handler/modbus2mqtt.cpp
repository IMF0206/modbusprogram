#include "modbus2mqtt.h"
#include "mqtt_json.h"
#include "sqlite_helper.h"

using namespace std;

modbus2mqtt::modbus2mqtt()
{
    m_mqttjson = new mqtt_json();
    m_dbhelper = new db_helper("./iot.db");
}

modbus2mqtt::~modbus2mqtt()
{
    if (m_mqttjson != NULL)
    {
        delete m_mqttjson;
        m_mqttjson = NULL;
    }
    if (m_dbhelper != NULL)
    {
        delete m_dbhelper;
        m_dbhelper = NULL;
    }
}

std::string modbus2mqtt::getmqttstr(string portid, map<string, float>datamap)
{
    string sql = "select deviceid from node where portid=" + portid + ";";
    m_dbhelper->sql_exec_with_return(sql);
    if (m_dbhelper->getsqlresult().empty())
    {
        printf("%s\n", sql);
        printf("No deviceid get in the table\n");
        return "{}";
    }
    m_mqttjson->create_json_data_upload_map(m_dbhelper->getsqlresult()[0], datamap);
    return m_mqttjson->get_jsonstr();
}