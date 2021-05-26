#ifndef MQTT_PUB_H_INCLUDED_
#define MQTT_PUB_H_INCLUDED_

#include "mqtt_json.h"
#include "sqlite_helper.h"

class mqtt_pub
{
public:
    mqtt_pub();
    virtual ~mqtt_pub();
    // int mqtt_init();

    int mqtt_send(std::string jsonstr, int type);
    int mqtt_pub_login_msg();
    int mqtt_pub_status_update(int status);
    int mqtt_pub_status_ipc(int status);
    int mqtt_pub_sysinfo_upload_msg(float usercpu, float syscpu, float mem);
    int mqtt_pub_get_dev_msg();
    int mqtt_pub_date_upload_msg(frame_info* Frame_info);
    int mqtt_pub_event_upload_msg(frame_info* Frame_info);
    int mqtt_pub_plat_access_msg();
private:
    mqtt_json* m_json;
    db_helper* m_dbhelper;
};

#endif