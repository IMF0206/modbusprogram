#include "mqtt_plat_parse.h"
#include "cJSON.h"

using namespace std;

mqtt_plat_parse::mqtt_plat_parse()
{
    m_json = new mqtt_json();
    m_dbhelper = new db_helper(DB_FILE_PATH);
}

mqtt_plat_parse::~mqtt_plat_parse()
{
    if (m_json)
    {
        delete m_json;
        m_dbhelper = NULL;
    }
    if (m_dbhelper)
    {
        delete m_dbhelper;
        m_dbhelper = NULL;
    }
}

int mqtt_plat_parse::mqtt_plat_parse_add_rsp(std::string jsonstr)
{
    cJSON * pJson = cJSON_Parse(jsonstr.c_str());
    if (!pJson)
    {
        printf("line : %d, parse failed\n", __LINE__);
        return -1;
    }
    cJSON * midJson = cJSON_GetObjectItem(pJson, "mid");
    if (!midJson)
    {
        printf("line : %d, no mid item\n", __LINE__);
        return -1;
    }
    cJSON * statusJson = cJSON_GetObjectItem(pJson, "statusCode");
    if (!statusJson)
    {
        printf("line : %d, no statuscode item\n", __LINE__);
    }
    if (statusJson->valueint != 0)
    {
        printf("line : %d, 平台侧响应错误码\n");
        // TODO
    }
    cJSON * dataarr = cJSON_GetObjectItem(pJson, "data");
    if (!cJSON_IsArray(dataarr))
    {
        printf("line : %d, dataarr no array\n", __LINE__);
        return -1;
    }
    int size = cJSON_GetArraySize(dataarr);
    for (int i = 0; i < size; i++)
    {
        cJSON * deviceitem = cJSON_GetArrayItem(dataarr, i);
        cJSON * deviceinfo = cJSON_GetObjectItem(deviceitem, "deviceInfo");
        cJSON * nodejson = cJSON_GetObjectItem(deviceinfo, "nodeId");
        cJSON * devicejson = cJSON_GetObjectItem(deviceinfo, "deviceId");
        string nodestr = nodejson->valuestring;
        string devicestr = devicejson->valuestring;
        char sql[128] = {0};
        sprintf(sql, "update node set deviceid = %s where nodeid = %s\n", devicestr.c_str(), devicestr.c_str());
        m_dbhelper->sql_exec_with_return(sql);
        cJSON * statuscode = cJSON_GetObjectItem(deviceitem, "statusCode");
        int code = statuscode->valueint;
        printf("code is %d\n", code);
    }
}

int mqtt_plat_parse::mqtt_plat_parse_update_rsp(std::string jsonstr)
{
    cJSON * pJson = cJSON_Parse(jsonstr.c_str());
    if (!pJson)
    {
        printf("line : %d, parse failed\n", __LINE__);
        return -1;
    }
    cJSON * midJson = cJSON_GetObjectItem(pJson, "mid");
    if (!midJson)
    {
        printf("line : %d, no mid item\n", __LINE__);
        return -1;
    }
    cJSON * statusJson = cJSON_GetObjectItem(pJson, "statusCode");
    if (!statusJson)
    {
        printf("line : %d, no statuscode item\n", __LINE__);
    }
    if (statusJson->valueint != 0)
    {
        printf("line : %d, 平台侧响应错误码\n");
        // TODO
    }

    cJSON * dataarr = cJSON_GetObjectItem(pJson, "data");
    if (!cJSON_IsArray(dataarr))
    {
        printf("line : %d, dataarr no array\n", __LINE__);
        return -1;
    }
    int size = cJSON_GetArraySize(dataarr);
    for (int i = 0; i < size; i++)
    {
        cJSON * dataitem = cJSON_GetArrayItem(dataarr, i);
        cJSON * devicejson = cJSON_GetObjectItem(dataitem, "deviceId");
        cJSON * statuscode = cJSON_GetObjectItem(dataitem, "statusCode");
        int code = statuscode->valueint;
        printf("code is %d\n", code);
    }
}

int mqtt_plat_parse::mqtt_plat_parse_del(std::string jsonstr)
{
    cJSON * pJson = cJSON_Parse(jsonstr.c_str());
    if (!pJson)
    {
        printf("line : %d, parse failed\n", __LINE__);
        return -1;
    }
    cJSON * midJson = cJSON_GetObjectItem(pJson, "mid");
    if (!midJson)
    {
        printf("line : %d, no mid item\n", __LINE__);
        return -1;
    }
    cJSON * statusJson = cJSON_GetObjectItem(pJson, "statusCode");
    if (!statusJson)
    {
        printf("line : %d, no statuscode item\n", __LINE__);
    }
    if (statusJson->valueint != 0)
    {
        printf("line : %d, 平台侧响应错误码\n");
        // TODO
    }

    cJSON * statusdesc = cJSON_GetObjectItem(pJson, "statusDesc");
    if (!statusdesc)
    {
        printf("line : %d, no statusDesc item\n", __LINE__);
    }
    printf("statusdesc : %s\n", statusdesc->valuestring);

    cJSON * dataarr = cJSON_GetObjectItem(pJson, "data");
    if (!cJSON_IsArray(dataarr))
    {
        printf("line : %d, dataarr no array\n", __LINE__);
        return -1;
    }
    int size = cJSON_GetArraySize(dataarr);
    for (int i = 0; i < size; i++)
    {
        cJSON * dataitem = cJSON_GetArrayItem(dataarr, i);
        cJSON * devicejson = cJSON_GetObjectItem(dataitem, "deviceId");
        cJSON * nodeidjson = cJSON_GetObjectItem(dataitem, "nodeId");
        cJSON * namejson = cJSON_GetObjectItem(dataitem, "name");
        printf("devicejson is %s, nodeidjson is : %s, namejson is : %s\n", devicejson->valuestring
        , nodeidjson->valuestring, namejson->valuestring);
    }
}

int mqtt_plat_parse::mqtt_plat_parse_query_rsp(std::string jsonstr)
{
    // TODO
}

int mqtt_plat_parse::mqtt_plat_parse_command(std::string jsonstr)
{
    cJSON * pJson = cJSON_Parse(jsonstr.c_str());
    if (!pJson)
    {
        printf("line : %d, parse failed\n", __LINE__);
        return -1;
    }
    cJSON * midJson = cJSON_GetObjectItem(pJson, "mid");
    if (!midJson)
    {
        printf("line : %d, no mid item\n", __LINE__);
        return -1;
    }
    cJSON * cmdjson = cJSON_GetObjectItem(pJson, "cmd");
    if (!cmdjson)
    {
        printf("line : %d, no cmdjson item\n", __LINE__);
        return -1;
    }
    m_command_str = cmdjson->valuestring;

    cJSON * parajson = cJSON_GetObjectItem(pJson, "paras");
    if (!parajson)
    {
        printf("line : %d, no parajson item\n", __LINE__);
        return -1;
    }
    //TODO 解析parajson
    // *************************************
    cJSON * devinfojson = cJSON_GetObjectItem(pJson, "deviceId");
    m_comm_deviceid = devinfojson->valuestring;
    cJSON * servicejson = cJSON_GetObjectItem(pJson, "serviceId");
    m_comm_serviceid = servicejson->valuestring;

}