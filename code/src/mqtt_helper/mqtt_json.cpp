#include "mqtt_json.h"
#include "cJSON.h"
#include <time.h>
#include <algorithm>
#include <string.h>
#include <map>

long g_mid_num = 1000000001;

extern int64_t hexToDec(unsigned char *source, int len);
extern float hex2float(unsigned char*p,float* result);

std::map<std::string, std::string> modelmap{std::make_pair("3", "lali"), std::make_pair("4", "qinjiao")};

long get_stamp_time()
{
    time_t tmpcal_ptr;
	struct tm *tmp_ptr = NULL;
	
	time(&tmpcal_ptr);
	printf("tmpcal_ptr=%d\n", tmpcal_ptr);
    return tmpcal_ptr;
}

std::string get_current_time()
{
    time_t tmpcal_ptr;
	struct tm *tmp_ptr = NULL;
	
	time(&tmpcal_ptr);
	printf("tmpcal_ptr=%d\n", tmpcal_ptr);
	
	tmp_ptr = gmtime(&tmpcal_ptr);
	printf("after gmtime, the time is:%02d:%02d:%02d\n", tmp_ptr->tm_hour, tmp_ptr->tm_min, tmp_ptr->tm_sec);
	
	tmp_ptr = localtime(&tmpcal_ptr);
	printf ("after localtime, the time is:%04d.%02d.%02d ", (1900+tmp_ptr->tm_year), (1+tmp_ptr->tm_mon), tmp_ptr->tm_mday);
	printf("%02d:%02d:%02d\n", tmp_ptr->tm_hour, tmp_ptr->tm_min, tmp_ptr->tm_sec);
    char current_time[48] = {0};
    sprintf(current_time, "%04d-%02d-%02d %02d:%02d:%02d", (1900+tmp_ptr->tm_year), (1+tmp_ptr->tm_mon), tmp_ptr->tm_mday,
     tmp_ptr->tm_hour, tmp_ptr->tm_min, tmp_ptr->tm_sec);
	return std::string(current_time);
}

std::string get_current_time4plat()
{
    time_t tmpcal_ptr;
	struct tm *tmp_ptr = NULL;
	
	time(&tmpcal_ptr);
	printf("tmpcal_ptr=%d\n", tmpcal_ptr);
	
	tmp_ptr = gmtime(&tmpcal_ptr);
	printf("after gmtime, the time is:%02d:%02d:%02d\n", tmp_ptr->tm_hour, tmp_ptr->tm_min, tmp_ptr->tm_sec);
	
	tmp_ptr = localtime(&tmpcal_ptr);
	printf ("after localtime, the time is:%04d.%02d.%02d ", (1900+tmp_ptr->tm_year), (1+tmp_ptr->tm_mon), tmp_ptr->tm_mday);
	printf("%02d:%02d:%02d\n", tmp_ptr->tm_hour, tmp_ptr->tm_min, tmp_ptr->tm_sec);
    char current_time[48] = {0};
    sprintf(current_time, "%04d%02d%02dT%02d%02d%02dZ", (1900+tmp_ptr->tm_year), (1+tmp_ptr->tm_mon), tmp_ptr->tm_mday,
     tmp_ptr->tm_hour, tmp_ptr->tm_min, tmp_ptr->tm_sec);
	return std::string(current_time);
}

mqtt_json::mqtt_json()
{
    m_jsonstr = "";
    m_dbhelper = new db_helper(DB_FILE_PATH);
}

mqtt_json::~mqtt_json()
{
    if (m_dbhelper)
    {
        delete m_dbhelper;
        m_dbhelper = NULL;
    }

}

std::string mqtt_json::get_jsonstr()
{
    return m_jsonstr;
}

void mqtt_json::create_json_add_device()
{
    cJSON *root=NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "mid", g_mid_num);
    cJSON_AddStringToObject(root, "type", "CMD_TOPO_ADD");
    cJSON_AddNumberToObject(root, "timestamp", get_stamp_time());
    cJSON_AddNumberToObject(root, "expire", -1);
    std::string sqlstr = "select status from node;";
    db_helper dbhelper(DB_FILE_PATH);
    dbhelper.sql_exec_with_return(sqlstr);
    if (dbhelper.getsqlresult().size() < 1)
    {
        printf("iot.db does not have devices;");
        return;
    }
    cJSON* nodearr = cJSON_CreateArray();
    std::vector<std::string> statusvec = dbhelper.getsqlresult();
    sqlstr = "select nodeid from node;";
    dbhelper.sql_exec_with_return(sqlstr);
    std::vector<std::string> nodeidvec = dbhelper.getsqlresult();
    sqlstr = "select name from node;";
    dbhelper.sql_exec_with_return(sqlstr);
    std::vector<std::string> namevec = dbhelper.getsqlresult();
    for (int i = 0; i < statusvec.size(); i++)
    {
        int status = std::stoi(statusvec[i]);
        printf("%d\n", status);
        if ((status & 2) > 0)
        {
            printf("已通过认证，pass\n");
            continue;
        }
        cJSON* nodeitem = cJSON_CreateObject();
        cJSON_AddStringToObject(nodeitem, "nodeId", nodeidvec[i].c_str());
        cJSON_AddStringToObject(nodeitem, "name", namevec[i].c_str());
        cJSON_AddStringToObject(nodeitem, "description", "test");
        cJSON_AddStringToObject(nodeitem, "mfgInfo", "NARI");
        cJSON_AddStringToObject(nodeitem, "nodeModel", "NARI");
        cJSON_AddItemToArray(nodearr, nodeitem);
    }
    cJSON *param = cJSON_CreateObject();
    cJSON_AddItemToObject(param, "nodeInfos", nodearr);
    cJSON_AddItemToObject(root, "param", param);
    m_jsonstr = cJSON_Print(root);
    
    printf("jsonstr is : %s\n", m_jsonstr.c_str());
    cJSON_Delete(root);
    return;
}

void mqtt_json::create_json_updatestatus_device(int status)
{
    cJSON *root=NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "mid", g_mid_num);
    cJSON_AddStringToObject(root, "type", "CMD_TOPO_UPDATE");
    cJSON_AddNumberToObject(root, "timestamp", get_stamp_time());
    cJSON_AddNumberToObject(root, "expire", -1);
    std::string sqlstr = "select status from node;";
    db_helper dbhelper(DB_FILE_PATH);
    dbhelper.sql_exec_with_return(sqlstr);
    if (dbhelper.getsqlresult().size() < 1)
    {
        printf("iot.db does not have devices;");
        return;
    }
    cJSON* nodearr = cJSON_CreateArray();
    std::vector<std::string> statusvec = dbhelper.getsqlresult();
    sqlstr = "select nodeid from node;";
    dbhelper.sql_exec_with_return(sqlstr);
    std::vector<std::string> nodeidvec = dbhelper.getsqlresult();
    for (int i = 0; i < statusvec.size(); i++)
    {
        int status = std::stoi(statusvec[i]);
        status = status&1;
        cJSON* nodeitem = cJSON_CreateObject();
        cJSON_AddStringToObject(nodeitem, "nodeId", nodeidvec[i].c_str());
        if (status == 1)
        {
            cJSON_AddStringToObject(nodeitem, "status", "ONLINE");
        }
        else
        {
            cJSON_AddStringToObject(nodeitem, "status", "OFFLINE");
        }

        cJSON_AddItemToArray(nodearr, nodeitem);
    }
    // {
    //     cJSON* nodeitem = cJSON_CreateObject();
    //     cJSON_AddStringToObject(nodeitem, "nodeId", "1100001000170014");
    //     if (status == 1)
    //     {
    //         cJSON_AddStringToObject(nodeitem, "status", "ONLINE");
    //     }
    //     else
    //     {
    //         cJSON_AddStringToObject(nodeitem, "status", "OFFLINE");
    //     }

    //     cJSON_AddItemToArray(nodearr, nodeitem);
    // }
    
    cJSON* param = cJSON_CreateObject();
    cJSON_AddItemToObject(param, "nodeStatuses", nodearr);
    cJSON_AddItemToObject(root, "param", param);
    m_jsonstr = cJSON_Print(root);
    
    printf("jsonstr is : %s\n", m_jsonstr.c_str());
    cJSON_Delete(root);
    return;
}

void mqtt_json::create_json_updatestatus_ipc(int status)
{
    cJSON *root=NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "mid", g_mid_num);
    cJSON_AddStringToObject(root, "type", "CMD_TOPO_UPDATE");
    cJSON_AddNumberToObject(root, "timestamp", get_stamp_time());
    cJSON_AddNumberToObject(root, "expire", -1);
    std::string sqlstr = "select devstatus from statusinfo;";
    db_helper dbhelper("/opt/nvr/nvr.db");
    dbhelper.sql_exec_with_return(sqlstr);
    if (dbhelper.getsqlresult().size() < 1)
    {
        printf("nvr.db does not have ipc;");
        return;
    }
    cJSON* nodearr = cJSON_CreateArray();
    std::vector<std::string> statusvec = dbhelper.getsqlresult();
    sqlstr = "select code from channel;";
    dbhelper.sql_exec_with_return(sqlstr);
    std::vector<std::string> nodeidvec = dbhelper.getsqlresult();
    for (int i = 0; i < statusvec.size(); i++)
    {
        int status = std::stoi(statusvec[i]);
        status = status&1;
        cJSON* nodeitem = cJSON_CreateObject();
        cJSON_AddStringToObject(nodeitem, "nodeId", nodeidvec[i].c_str());
        if (status == 1)
        {
            cJSON_AddStringToObject(nodeitem, "status", "ONLINE");
        }
        else
        {
            cJSON_AddStringToObject(nodeitem, "status", "OFFLINE");
        }

        cJSON_AddItemToArray(nodearr, nodeitem);
    }
    // {
    //     cJSON* nodeitem = cJSON_CreateObject();
    //     cJSON_AddStringToObject(nodeitem, "nodeId", "1100001000170014");
    //     if (status == 1)
    //     {
    //         cJSON_AddStringToObject(nodeitem, "status", "ONLINE");
    //     }
    //     else
    //     {
    //         cJSON_AddStringToObject(nodeitem, "status", "OFFLINE");
    //     }

    //     cJSON_AddItemToArray(nodearr, nodeitem);
    // }
    
    cJSON* param = cJSON_CreateObject();
    cJSON_AddItemToObject(param, "nodeStatuses", nodearr);
    cJSON_AddItemToObject(root, "param", param);
    m_jsonstr = cJSON_Print(root);
    
    printf("jsonstr is : %s\n", m_jsonstr.c_str());
    cJSON_Delete(root);
    return;
}

void mqtt_json::create_json_data_upload(frame_info* Frame_info)
{
    cJSON *root=NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "mid", g_mid_num);
    cJSON_AddStringToObject(root, "type", "CMD_REPORTDATA");
    cJSON_AddNumberToObject(root, "timestamp", get_stamp_time());
    cJSON_AddNumberToObject(root, "expire", -1);
    // std::string sqlstr = "select deviceid from node where nodeid=" + std::to_string(hexToDec(Frame_info->node_info, 4));
    // db_helper dbhelper(DB_FILE_PATH);
    // dbhelper.sql_exec_with_return(sqlstr);
    // if (dbhelper.getsqlresult().size() < 1)
    // {
    //     printf("iot.db does not have such nodeid.\n");
    //     return;
    // }
    // cJSON_AddStringToObject(root, "deviceId", dbhelper.getsqlresult()[0].c_str());
    // std::string devicestr = dbhelper.getsqlresult()[0];

    // 使用平台侧的deviceId
    // cJSON_AddStringToObject(root, "deviceId", "1100001000170014");
    std::string sqlstr = "select name from node where deviceid= '1100001000170014'";
    db_helper dbhelper(DB_FILE_PATH);
    // dbhelper.sql_exec_with_return(sqlstr);
    // if (dbhelper.getsqlresult().size() < 1)
    // {
    //     printf("iot.db does not have such deviceid.\n");
    // }
    // std::string namestr = dbhelper.getsqlresult()[0];
    std::string namestr = "";
    cJSON *param = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();
    float tension = 0, angleX = 0, angleY = 0, voltage = 0;
    int64_t battery = 0;
    unsigned char tensionarr[4] = {0};
    unsigned char anglexarr[4] = {0};
    unsigned char angleyarr[4] = {0};
    unsigned char voltagearr[4] = {0};
    switch(Frame_info->cmd)
    {
        case 5:
            cJSON_AddStringToObject(param, "cmd", "pull_alert");
            cJSON_AddStringToObject(param, "deviceId", "1100001000170014");
            cJSON_AddStringToObject(root, "deviceId", "1100001000170014");
            sqlstr = "select name from node where nodeid= '1100001000170014'";
            dbhelper.sql_exec_with_return(sqlstr);
            if (dbhelper.getsqlresult().size() < 1)
            {
                printf("iot.db does not have such deviceid.\n");
            }
            namestr = dbhelper.getsqlresult()[0];
            tensionarr[0] = Frame_info->frame_data[3];
            tensionarr[1] = Frame_info->frame_data[2];
            tensionarr[2] = Frame_info->frame_data[1];
            tensionarr[3] = Frame_info->frame_data[0];
            hex2float(tensionarr, &tension);
            cJSON_AddStringToObject(data, "tension", std::to_string(tension).c_str());
            break;
        case 6:
            cJSON_AddStringToObject(param, "cmd", "inclination_alert");
            cJSON_AddStringToObject(param, "deviceId", "4143972352");
            cJSON_AddStringToObject(root, "deviceId", "4143972352");
            sqlstr = "select name from node where nodeid= '4143972352'";
            dbhelper.sql_exec_with_return(sqlstr);
            if (dbhelper.getsqlresult().size() < 1)
            {
                printf("iot.db does not have such deviceid.\n");
            }
            namestr = dbhelper.getsqlresult()[0];
            anglexarr[0] = Frame_info->frame_data[3];
            anglexarr[1] = Frame_info->frame_data[2];
            anglexarr[2] = Frame_info->frame_data[1];
            anglexarr[3] = Frame_info->frame_data[0];
            angleyarr[0] = Frame_info->frame_data[7];
            angleyarr[1] = Frame_info->frame_data[6];
            angleyarr[2] = Frame_info->frame_data[5];
            angleyarr[3] = Frame_info->frame_data[4];
            hex2float(anglexarr, &angleX);
            hex2float(angleyarr, &angleY);
            cJSON_AddStringToObject(data, "angleX", std::to_string(angleX).c_str());
            cJSON_AddStringToObject(data, "angleY", std::to_string(angleY).c_str());
            break;
        // case 9:
        //     cJSON_AddStringToObject(param, "cmd", "inclination_alert");
        //     cJSON_AddStringToObject(param, "deviceId", "1100001000170014");
        //     sqlstr = "select name from node where nodeid= '1100001000170014'";
        //     dbhelper.sql_exec_with_return(sqlstr);
        //     if (dbhelper.getsqlresult().size() < 1)
        //     {
        //         printf("iot.db does not have such deviceid.\n");
        //     }
        //     namestr = dbhelper.getsqlresult()[0];
        //     voltagearr[0] = Frame_info->frame_data[3];
        //     voltagearr[1] = Frame_info->frame_data[2];
        //     voltagearr[2] = Frame_info->frame_data[1];
        //     voltagearr[3] = Frame_info->frame_data[0];
        //     hex2float(voltagearr, &voltage);
        //     battery = hexToDec(Frame_info->frame_data+4, 1);
        //     cJSON_AddStringToObject(data, "voltage", std::to_string(voltage).c_str());
        //     cJSON_AddStringToObject(data, "battery", std::to_string(battery).c_str());
        default:
            printf("unknown cmd number.\n");
    }
    cJSON_AddStringToObject(data, "collectTime", get_current_time().c_str());
    cJSON_AddStringToObject(data, "devicename", namestr.c_str());
    // cJSON_AddStringToObject(data, "devicename", "app111");
    cJSON_AddItemToObject(param, "data", data);
    cJSON_AddItemToObject(root, "param", param);
    m_jsonstr = cJSON_Print(root);
    
    printf("jsonstr is : %s\n", m_jsonstr.c_str());
    // cJSON_Delete(data);
    // cJSON_Delete(param);
    cJSON_Delete(root);
    return;
}

void mqtt_json::create_json_event_upload(frame_info* Frame_info)
{
    cJSON *root=NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "mid", g_mid_num);
    cJSON_AddStringToObject(root, "type", "CMD_REPORTEVENT");
    cJSON_AddNumberToObject(root, "timestamp", get_stamp_time());
    cJSON_AddNumberToObject(root, "expire", -1);
    // std::string sqlstr = "select deviceid from node where nodeid=" + std::to_string(hexToDec(Frame_info->node_info, 4));
    // db_helper dbhelper(DB_FILE_PATH);
    // dbhelper.sql_exec_with_return(sqlstr);
    // if (dbhelper.getsqlresult().size() < 1)
    // {
    //     printf("iot.db does not have such nodeid.\n");
    //     return;
    // }
    // cJSON_AddStringToObject(root, "deviceId", dbhelper.getsqlresult()[0].c_str());
    // std::string devicestr = dbhelper.getsqlresult()[0];

    // 使用平台侧的deviceId
    std::string sqlstr = "select name from node where nodeid='1100001000170014'";
    db_helper dbhelper(DB_FILE_PATH);
    // dbhelper.sql_exec_with_return(sqlstr);
    // if (dbhelper.getsqlresult().size() < 1)
    // {
    //     printf("iot.db does not have such nodeid.\n");
    // }
    // std::string namestr = dbhelper.getsqlresult()[0];
    std::string namestr = "";
    cJSON *param = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();
    float tension = 0, angleX = 0, angleY = 0;
    unsigned char tensionarr[4] = {0};
    unsigned char anglexarr[4] = {0};
    unsigned char angleyarr[4] = {0};
    // std::string almDescription = "";
    switch(Frame_info->cmd)
    {
        case 1:
            cJSON_AddStringToObject(param, "event", "tension_alarm");
            cJSON_AddStringToObject(param, "deviceId", "1100001000170014");
            cJSON_AddStringToObject(root, "deviceId", "1100001000170014");
            sqlstr = "select name from node where nodeid= '1100001000170014'";
            dbhelper.sql_exec_with_return(sqlstr);
            if (dbhelper.getsqlresult().size() < 1)
            {
                printf("iot.db does not have such deviceid.\n");
            }
            namestr = dbhelper.getsqlresult()[0];
            tensionarr[0] = Frame_info->frame_data[3];
            tensionarr[1] = Frame_info->frame_data[2];
            tensionarr[2] = Frame_info->frame_data[1];
            tensionarr[3] = Frame_info->frame_data[0];
            hex2float(tensionarr, &tension);
            cJSON_AddStringToObject(data, "tension", std::to_string(tension).c_str());
            cJSON_AddStringToObject(data, "almDescription", "拉力设备告警");
            break;
        case 2:
            cJSON_AddStringToObject(param, "event", "angle_alarm");
            cJSON_AddStringToObject(param, "deviceId", "4143972352");
            cJSON_AddStringToObject(root, "deviceId", "4143972352");
            sqlstr = "select name from node where nodeid= '4143972352'";
            dbhelper.sql_exec_with_return(sqlstr);
            if (dbhelper.getsqlresult().size() < 1)
            {
                printf("iot.db does not have such deviceid.\n");
            }
            namestr = dbhelper.getsqlresult()[0];
            anglexarr[0] = Frame_info->frame_data[3];
            anglexarr[1] = Frame_info->frame_data[2];
            anglexarr[2] = Frame_info->frame_data[1];
            anglexarr[3] = Frame_info->frame_data[0];
            angleyarr[0] = Frame_info->frame_data[7];
            angleyarr[1] = Frame_info->frame_data[6];
            angleyarr[2] = Frame_info->frame_data[5];
            angleyarr[3] = Frame_info->frame_data[4];
            hex2float(anglexarr, &angleX);
            hex2float(angleyarr, &angleY);
            cJSON_AddStringToObject(data, "almXValue", std::to_string(angleX).c_str());
            cJSON_AddStringToObject(data, "almYValue", std::to_string(angleY).c_str());
            cJSON_AddStringToObject(data, "almDescription", "角度设备告警");
            break;

        default:
            printf("unknown cmd number.\n");
            return;
    }
    cJSON_AddStringToObject(data, "almTime", get_current_time().c_str());
    cJSON_AddStringToObject(data, "devicename", namestr.c_str());
    // cJSON_AddStringToObject(data, "devicename", "app111");
    cJSON_AddItemToObject(param, "data", data);
    cJSON_AddItemToObject(root, "param", param);
    this->m_jsonstr = cJSON_Print(root);
    printf("jsonstr is : %s\n", m_jsonstr.c_str());
    // cJSON_Delete(data);
    // cJSON_Delete(param);
    cJSON_Delete(root);
    return;
}

void mqtt_json::create_json_sysinfo_upload(float usercpu, float syscpu, float mem)
{
    cJSON *root=NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "mid", g_mid_num);
    cJSON_AddStringToObject(root, "type", "CMD_REPORTDATA");
    cJSON_AddNumberToObject(root, "timestamp", get_stamp_time());
    cJSON_AddNumberToObject(root, "expire", -1);

    cJSON *param = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();

    cJSON_AddStringToObject(param, "cmd", "sys_info");
    cJSON_AddStringToObject(param, "deviceId", "1100001000170006");
    cJSON_AddStringToObject(root, "deviceId", "1100001000170006");

    cJSON_AddNumberToObject(data, "user_cpu", usercpu);
    std::string syscpustr = std::to_string(syscpu);
    cJSON_AddStringToObject(data, "sys_cpu", syscpustr.c_str());
    std::string memstr = std::to_string(mem/1000)+"M";
    cJSON_AddStringToObject(data, "mem_free", memstr.c_str());

    cJSON_AddStringToObject(data, "collectTime", get_current_time().c_str());
    cJSON_AddStringToObject(data, "devicename", "NARI_WULIAN");

    cJSON_AddItemToObject(param, "data", data);
    cJSON_AddItemToObject(root, "param", param);
    m_jsonstr = cJSON_Print(root);
    
    printf("jsonstr is : %s\n", m_jsonstr.c_str());

    cJSON_Delete(root);
    return;
}

void mqtt_json::parse_json_device_response(char* msg)
{
    cJSON* cjson = cJSON_Parse(msg);
    if (!cjson)
    {
        printf("cjson is NULL, return;\n");
        return;
    }
    char* typestr = cJSON_GetObjectItem(cjson, "type")->valuestring;
    if (strcmp(typestr, "CMD_TOPO_UPDATE") == 0)
    {
        cJSON* param = cJSON_GetObjectItem(cjson, "param");
        cJSON* resultarr = cJSON_GetObjectItem(param, "result");
        int num = cJSON_GetArraySize(resultarr);
        for (int i = 0; i < num; i++)
        {
            cJSON* resultitem = cJSON_GetArrayItem(resultarr, i);
            cJSON* statusitem = cJSON_GetObjectItem(resultitem, "statusDesc");
            cJSON* nodeiditem = cJSON_GetObjectItem(resultitem, "deviceId");
            printf("node %s status from response is %s\n", nodeiditem->valuestring, statusitem->valuestring);
        }
    }
    if (strcmp(typestr, "CMD_TOPO_ADD") == 0)
    {
        cJSON* param = cJSON_GetObjectItem(cjson, "param");
        cJSON* resultarr = cJSON_GetObjectItem(param, "result");
        int num = cJSON_GetArraySize(resultarr);
        for (int i = 0; i < num; i++)
        {
            cJSON* resultitem = cJSON_GetArrayItem(resultarr, i);
            cJSON* statusitem = cJSON_GetObjectItem(resultitem, "statusDesc");
            cJSON* nodeiditem = cJSON_GetObjectItem(resultitem, "deviceId");
            printf("node %s status from response is %s\n", nodeiditem->valuestring, statusitem->valuestring);
        }
    }
}

device_resp* mqtt_json::get_deviceresp()
{
    return NULL;
}

std::string mqtt_json::get_json4plat()
{
    return m_json4plat;
}

void mqtt_json::create_json_plat_adddev()
{
    cJSON *root=NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "mid", PLAT_ADD_DEV);
    std::string sqlstr = "select status from node;";
    db_helper dbhelper(DB_FILE_PATH);
    dbhelper.sql_exec_with_return(sqlstr);
    if (dbhelper.getsqlresult().size() < 1)
    {
        printf("iot.db does not have devices;");
        return;
    }
    cJSON* nodearr = cJSON_CreateArray();
    std::vector<std::string> statusvec = dbhelper.getsqlresult();
    sqlstr = "select nodeid from node;";
    dbhelper.sql_exec_with_return(sqlstr);
    std::vector<std::string> nodeidvec = dbhelper.getsqlresult();
    sqlstr = "select name from node;";
    dbhelper.sql_exec_with_return(sqlstr);
    std::vector<std::string> namevec = dbhelper.getsqlresult();
    sqlstr = "select type from node;";
    dbhelper.sql_exec_with_return(sqlstr);
    std::vector<std::string> typevec = dbhelper.getsqlresult();
    for (int i = 0; i < statusvec.size(); i++)
    {
        int status = std::stoi(statusvec[i]);
        printf("%d\n", status);
        if ((status & 2) > 0)
        {
            printf("已通过认证，pass\n");
            continue;
        }
        if (modelmap.find(typevec[i]) == modelmap.end())
        {
            printf("no model found in db\n");
            continue;
        }
        cJSON* nodeitem = cJSON_CreateObject();
        cJSON_AddStringToObject(nodeitem, "manufacturerId", "NARI_n");
        cJSON_AddStringToObject(nodeitem, "name", namevec[i].c_str());
        cJSON_AddStringToObject(nodeitem, "description", "test");
        cJSON_AddStringToObject(nodeitem, "model", modelmap[typevec[i]].c_str());
        cJSON_AddStringToObject(nodeitem, "nodeId", nodeidvec[i].c_str());
        cJSON_AddItemToArray(nodearr, nodeitem);
    }
    cJSON_AddItemToObject(root, "deviceInfos", nodearr);
    m_jsonstr = cJSON_Print(root);
    
    m_json4plat = cJSON_Print(root);
    printf("jsonstr is : %s\n", m_json4plat.c_str());
    cJSON_Delete(root);
    return;
}

void mqtt_json::create_json_plat_update(int status)
{
    cJSON *root=NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "mid", PLAT_UPDATE_DEV);
    std::string sqlstr = "select status from node;";
    db_helper dbhelper(DB_FILE_PATH);
    dbhelper.sql_exec_with_return(sqlstr);
    if (dbhelper.getsqlresult().size() < 1)
    {
        printf("iot.db does not have devices;");
        return;
    }
    cJSON* nodearr = cJSON_CreateArray();
    std::vector<std::string> statusvec = dbhelper.getsqlresult();
    sqlstr = "select deviceid from node;";
    dbhelper.sql_exec_with_return(sqlstr);
    std::vector<std::string> deviceidvec = dbhelper.getsqlresult();
    for (int i = 0; i < statusvec.size(); i++)
    {
        int status = std::stoi(statusvec[i]);
        status = status&1;
        cJSON* deviceitem = cJSON_CreateObject();
        cJSON_AddStringToObject(deviceitem, "deviceId", deviceidvec[i].c_str());
        if (status == 1)
        {
            cJSON_AddStringToObject(deviceitem, "status", "ONLINE");
        }
        else
        {
            cJSON_AddStringToObject(deviceitem, "status", "OFFLINE");
        }

        cJSON_AddItemToArray(nodearr, deviceitem);
    }
    
    cJSON* param = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "deviceStatuses", nodearr);
    
    m_json4plat = cJSON_Print(root);
    printf("jsonstr is : %s\n", m_json4plat.c_str());
    cJSON_Delete(root);
    return;
}

void mqtt_json::create_json_plat_query()
{
    cJSON *root=NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "mid", PLAT_QUERY_GATE);
    std::string sqlstr = "select status from node;";
    db_helper dbhelper(DB_FILE_PATH);
    dbhelper.sql_exec_with_return(sqlstr);
    if (dbhelper.getsqlresult().size() < 1)
    {
        printf("iot.db does not have devices;");
        return;
    }
    cJSON* nodearr = cJSON_CreateArray();
    std::vector<std::string> statusvec = dbhelper.getsqlresult();
    sqlstr = "select deviceid from node;";
    dbhelper.sql_exec_with_return(sqlstr);
    std::vector<std::string> deviceidvec = dbhelper.getsqlresult();
    for (int i = 0; i < statusvec.size(); i++)
    {
        int status = std::stoi(statusvec[i]);
        status = status&1;
        cJSON* deviceitem = cJSON_CreateObject();
        cJSON_AddStringToObject(deviceitem, "deviceId", deviceidvec[i].c_str());
        if (status == 1)
        {
            cJSON_AddStringToObject(deviceitem, "status", "ONLINE");
        }
        else
        {
            cJSON_AddStringToObject(deviceitem, "status", "OFFLINE");
        }

        cJSON_AddItemToArray(nodearr, deviceitem);
    }
    
    cJSON* param = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "deviceStatuses", nodearr);
    m_json4plat = cJSON_Print(root);
    printf("jsonstr is : %s\n", m_json4plat.c_str());
    cJSON_Delete(root);
    return;
}

void mqtt_json::create_json_plat_command_resp()
{
    cJSON *root=NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "mid", PLAT_COMMAND);
    
    cJSON* body = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "errcode", 0);
    cJSON_AddStringToObject(root, "msgType", "deviceRsp");
    cJSON* originParam = cJSON_CreateObject();
    // TODO
    /**
     * {
     *    "body": {
     *             "orginParameters": {
     *                       "temperature": 123
     *              },
     *              "state": "ok"
     *    },
     *    "errcode": 0,
     *    "mid": 54132,
     *    "msgType": "deviceRsp"
     * }
     **/
    cJSON_AddNumberToObject(originParam, "test", 123);
    cJSON_AddItemToObject(body, "orginParameters", originParam);
    cJSON_AddStringToObject(body, "state", "ok");
    cJSON_AddItemToObject(root, "body", body);
    m_json4plat = cJSON_Print(root);
    printf("jsonstr is : %s\n", m_json4plat.c_str());
    cJSON_Delete(root);
    return;
}

void mqtt_json::create_json_plat_date(frame_info* Frame_info)
{
    /**
     * {
     *  "devices": [{
     *         "deviceId": "D68NZxB4",
     *         "services": [{
     *                "data": {
     *                       "key": "value"
     *                },
     *                "eventTime": "20191023T173625Z",
     *                "serviceId": "serviceName"
     *         }]
     *  }]
     * }
     */
    cJSON *root=NULL;
    root = cJSON_CreateObject();
    std::string namestr = "";
    cJSON *devices = cJSON_CreateArray();
    cJSON *devicesitem = cJSON_CreateObject();
    cJSON *services = cJSON_CreateArray();
    cJSON *servicesitem = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();
    float tension = 0, angleX = 0, angleY = 0, voltage = 0;
    int64_t battery = 0;
    unsigned char tensionarr[4] = {0};
    unsigned char anglexarr[4] = {0};
    unsigned char angleyarr[4] = {0};
    unsigned char voltagearr[4] = {0};
    std::string sqlstr;
    switch(Frame_info->cmd)
    {
        case 5:
            // 查找在线的拉力传感器，status 3表示已注册且在线
            sqlstr = "select deviceid from node where type=4 and status=3";
            m_dbhelper->sql_exec_with_return(sqlstr);
            if (m_dbhelper->getsqlresult().size() < 1)
            {
                printf("iot.db does not have such deviceid.\n");
            }
            namestr = m_dbhelper->getsqlresult()[0];
            tensionarr[0] = Frame_info->frame_data[3];
            tensionarr[1] = Frame_info->frame_data[2];
            tensionarr[2] = Frame_info->frame_data[1];
            tensionarr[3] = Frame_info->frame_data[0];
            hex2float(tensionarr, &tension);
            cJSON_AddStringToObject(data, "tension", std::to_string(tension).c_str());
            cJSON_AddItemToObject(servicesitem, "data", data);
            cJSON_AddStringToObject(servicesitem, "eventTime", get_current_time4plat().c_str());
            // TODO 不知道填啥
            cJSON_AddStringToObject(servicesitem, "serviceId", "serviceName");

            cJSON_AddItemToArray(services, servicesitem);
            cJSON_AddStringToObject(devicesitem, "deviceId", namestr.c_str());
            cJSON_AddItemToObject(devicesitem, "services", services);
            cJSON_AddItemToArray(devices, devicesitem);
            cJSON_AddItemToObject(root, "devices", devices);
            break;
        case 6:
            // 查找在线的倾角传感器，status 3表示已注册且在线
            sqlstr = "select deviceid from node where type=3 and status=3";
            m_dbhelper->sql_exec_with_return(sqlstr);
            if (m_dbhelper->getsqlresult().size() < 1)
            {
                printf("iot.db does not have such deviceid.\n");
            }
            namestr = m_dbhelper->getsqlresult()[0];
            anglexarr[0] = Frame_info->frame_data[3];
            anglexarr[1] = Frame_info->frame_data[2];
            anglexarr[2] = Frame_info->frame_data[1];
            anglexarr[3] = Frame_info->frame_data[0];
            angleyarr[0] = Frame_info->frame_data[7];
            angleyarr[1] = Frame_info->frame_data[6];
            angleyarr[2] = Frame_info->frame_data[5];
            angleyarr[3] = Frame_info->frame_data[4];
            hex2float(anglexarr, &angleX);
            hex2float(angleyarr, &angleY);
            cJSON_AddStringToObject(data, "angleX", std::to_string(angleX).c_str());
            cJSON_AddStringToObject(data, "angleY", std::to_string(angleY).c_str());
            cJSON_AddItemToObject(servicesitem, "data", data);
            cJSON_AddStringToObject(servicesitem, "eventTime", get_current_time4plat().c_str());
            // TODO 不知道填啥
            cJSON_AddStringToObject(servicesitem, "serviceId", "serviceName");

            cJSON_AddItemToArray(services, servicesitem);
            cJSON_AddStringToObject(devicesitem, "deviceId", namestr.c_str());
            cJSON_AddItemToObject(devicesitem, "services", services);
            cJSON_AddItemToArray(devices, devicesitem);
            cJSON_AddItemToObject(root, "devices", devices);
            break;
        default:
            printf("unknown cmd number.\n");
    }
    m_json4plat = cJSON_Print(root);
    
    printf("jsonstr is : %s\n", m_json4plat.c_str());
    cJSON_Delete(root);
    return;
}