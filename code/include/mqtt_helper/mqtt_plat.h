#ifndef MQTT_PLAT_H_INCLUDED_
#define MQTT_PLAT_H_INCLUDED_

#include "mqtt_client.h"
#include "sqlite_helper.h"
class mqtt_plat
{
public:
    mqtt_plat();
    virtual ~mqtt_plat();
    int mqtt_init();
    int mqtt_platconnect();
    int mqtt_platdisconnect();
    // 边设备->平台
    int mqtt_platadddev();
    int mqtt_platupdatedev(int status);
    int mqtt_platreqdev();
    int mqtt_platcommandresp();
    int mqtt_platdatasend(frame_info* Frame_info);
    // 平台->边设备
    int mqtt_plat_sub_init();
    // int mqtt_platadddevresp();
    // int mqtt_platupdatedevresp();
    // int mqtt_platdeldev();
    // int mqtt_platreqdevresp();
    // int mqtt_platcommand();
    

private:
    bool m_logined;
    mqtt_json* m_json;
    db_helper* m_dbhelper;
    mqtt_client* m_myclient;
    std::string get_timestamp();
    unsigned char * m_passwd;
    std::string m_gatewayId;
};

#endif