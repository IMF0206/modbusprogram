#ifndef MQTT_JSON_H_INCLUDED_
#define MQTT_JSON_H_INCLUDED_

#include "sqlite_helper.h"

// 定义对应的mid值
#define PLAT_ADD_DEV 7
#define PLAT_UPDATE_DEV 9
#define PLAT_QUERY_GATE 2
#define PLAT_COMMAND 54132

typedef struct FRAMEINFO
{
    unsigned char head_info;
    unsigned char address_info[4];
    unsigned char frame_num;
    unsigned char frame_des;
    unsigned char frame_type;
    int64_t data_num;
    unsigned char cmd;
    unsigned char node_info[4];
    unsigned char* frame_data;
}frame_info;

typedef struct DEVICERESULT
{
    std::string nodeId;
    int statuscode;
}device_result;

typedef struct DEVICERESP
{
    std::string type;
    int result_num;
    std::vector<device_result> results;
}device_resp;


class mqtt_json
{
public:
    mqtt_json();
    ~mqtt_json();
    
    std::string get_jsonstr();
    void create_json_add_device();
    void create_json_updatestatus_device(int status);
    void create_json_updatestatus_ipc(int status);
    void create_json_data_upload(frame_info* Frame_info);
    void create_json_sysinfo_upload(float usercpu, float syscpu, float mem);
    void create_json_event_upload(frame_info* Frame_info);
    void parse_json_device_response(char* msg);
    device_resp* get_deviceresp();

    // 创建平台侧对接的json
    std::string get_json4plat();
    // 边设备->平台
    void create_json_plat_adddev();
    void create_json_plat_update(int status);
    void create_json_plat_query();
    void create_json_plat_command_resp();
    void create_json_plat_date(frame_info* Frame_info);
    // 平台->边设备
    

private:
    std::string m_jsonstr;
    std::string m_json4plat;
    db_helper* m_dbhelper;
    device_resp* m_deviceresp;
    
};

#endif