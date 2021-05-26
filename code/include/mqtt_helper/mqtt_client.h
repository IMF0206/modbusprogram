#ifndef MQTT_CLIENT_H_INCLUDED_
#define MQTT_CLIENT_H_INCLUDED_

#include "mqtt_json.h"
#include "sqlite_helper.h"
#include "MQTTClient.h"

class mqtt_client
{
public:
    mqtt_client();
    virtual ~mqtt_client();
    int mqtt_init();

    int mqtt_client_open(std::string addr, std::string clientid, std::string username, unsigned char* passwd);

    int mqtt_client_publish(char *top, int qos, char *msg, int len);
    int mqtt_client_subscribe(char *top, int qos);
    int mqtt_client_disconn();

    int mqtt_client_sub_thread();

private:
    mqtt_json* m_json;
    db_helper* m_dbhelper;
    MQTTClient m_client;
};

#endif