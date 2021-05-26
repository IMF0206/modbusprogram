#include "mqtt_pub.h"
#include "mqtt_json.h"
#include <algorithm>
#include "sqlite_helper.h"
#include "MQTTClient.h"

// #define ADDRESS     "ssl://172.16.20.40:8883"
#define ADDRESS     "tcp://172.16.20.40:1883"
// #define ADDRESS     "ssl://172.16.20.40:8883"
#define CLIENTID    "ExampleClientPub"
#define TOPIC       "topictests/aaa/bbb"
#define PAYLOAD     "Hello man, can you see me?"
#define QOS         1
#define TIMEOUT     10000L
#define EVENT 1
#define DATA 2


mqtt_pub::mqtt_pub()
{
    m_json = new mqtt_json();
    m_dbhelper = new db_helper(DB_FILE_PATH);
}

mqtt_pub::~mqtt_pub()
{
    if (m_json)
    {
        delete m_json;
        m_json = NULL;
    }
    if (m_dbhelper)
    {
        delete m_dbhelper;
        m_dbhelper = NULL;
    }
}

int ssl_error_cb(const char *str, size_t len, void *u)
{
    printf("%s\n", str);
}

int mqtt_pub::mqtt_send(std::string jsonstr, int type)
{
    m_dbhelper->sql_exec_with_return("select deviceid from edgedev;");
    // std::string cmd = "mosquitto_pub -t /v1/" + m_dbhelper->getsqlresult()[0] + "/topo/request -m " + jsonstr;
    // printf("cmd:%s\n", cmd.c_str());
    // system(cmd.c_str());
    std::string topicstr = "";
    std::string deviceidstr = m_dbhelper->getsqlresult()[0];
    if (deviceidstr.empty())
    {
        deviceidstr = "app1";
    }
    if (type == 0)
    {
        topicstr = "/v1/" + deviceidstr + "/topo/request";
        // topicstr = "/v1/app1/topo/request";
    }
    else if (type == DATA)
    {
        topicstr = "/v1/" + deviceidstr + "/service/data";
        // topicstr = "/v1/app1/service/data";
    }
    else if (type == EVENT)
    {
        topicstr = "/v1/" + deviceidstr + "/service/event";
        // topicstr = "/v1/app1/service/event";
    }
    
    MQTTClient client;
    int rc;

    m_dbhelper->sql_exec_with_return("select ipaddr from edgedev;");
    std::string deviceip = m_dbhelper->getsqlresult()[0];
    m_dbhelper->sql_exec_with_return("select mqport from edgedev;");
    std::string mqportstr = m_dbhelper->getsqlresult()[0];
    // 判断是否需要进行加密
    m_dbhelper->sql_exec_with_return("select secmode from edgedev;");
    std::string secmode = m_dbhelper->getsqlresult()[0];
    char mqttaddr[128] = {0};
    if (secmode.compare("1") == 0)
    {
        if (deviceip.size() == 0)
        {
            deviceip = "192.168.68.1";
        }
        if (mqportstr.size() == 0)
        {
            mqportstr = "1883";
        }
        sprintf(mqttaddr, "ssl://%s:%s", deviceip.c_str(), mqportstr.c_str());
    }
    else
    {
        if (deviceip.size() == 0)
        {
            deviceip = "192.168.68.1";
        }
        if (mqportstr.size() == 0)
        {
            mqportstr = "8883";
        }
        sprintf(mqttaddr, "tcp://%s:%s", deviceip.c_str(), mqportstr.c_str());
    }
    
    
    printf("ADDRESS is : %s\n", mqttaddr);
    if ((rc = MQTTClient_create(&client, mqttaddr, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
         printf("%s, %d:Failed to create client, return code %d\n", __FILE__, __LINE__, rc);
         exit(EXIT_FAILURE);
    }

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
    conn_opts.username = "admin";
    conn_opts.password = "admin";
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    // 配置TLS证书
    if (secmode.compare("1") == 0)
    {
        conn_opts.ssl = &ssl_opts;
        conn_opts.struct_version = 1;
        conn_opts.ssl->trustStore = "/home/sysadm/testserver/ca_0901/ca.crt";
        conn_opts.ssl->CApath = "/home/sysadm/testserver/ca_0901/";
        // conn_opts.ssl->privateKey = "/home/openest/certs/client/client.key";
        // conn_opts.ssl->keyStore =   "/home/openest/certs/client/client.crt";
        conn_opts.ssl->ssl_error_cb = ssl_error_cb;
        conn_opts.ssl->enableServerCertAuth = 1;
        conn_opts.ssl->sslVersion = 2;
        conn_opts.ssl->verify = 1;
    }
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        return -1;
        exit(EXIT_FAILURE);
    }
    std::replace(jsonstr.begin(), jsonstr.end(), '\t', ' ');
    pubmsg.payload = (void*)(jsonstr.c_str());
    pubmsg.payloadlen = jsonstr.size();
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    if ((rc = MQTTClient_publishMessage(client, topicstr.c_str(), &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
    {
         printf("Failed to publish message, return code %d\n", rc);
         exit(EXIT_FAILURE);
    }

    printf("Waiting for up to %d seconds for publication of %s\n"
            "on topic %s for client with ClientID: %s\n",
            (int)(TIMEOUT/1000), pubmsg.payload, TOPIC, CLIENTID);
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message with delivery token %d delivered\n", token);

    if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
    	printf("Failed to disconnect, return code %d\n", rc);
    MQTTClient_destroy(&client);
    return rc;
}

int mqtt_pub::mqtt_pub_login_msg()
{
    printf("%s, %d\n", __FILE__, __LINE__);
    m_json->create_json_add_device();
    std::string jsonstr = m_json->get_jsonstr();
    std::replace(jsonstr.begin(), jsonstr.end(), '\n', ' ');
    mqtt_send(jsonstr, 0);    
    return 0;
}

int mqtt_pub::mqtt_pub_plat_access_msg()
{
    printf("%s, %d\n", __FILE__, __LINE__);
    m_json->create_json_add_device();
    std::string jsonstr = m_json->get_jsonstr();
    std::replace(jsonstr.begin(), jsonstr.end(), '\n', ' ');
    mqtt_send(jsonstr, 0);
    return 0;
}

int mqtt_pub::mqtt_pub_get_dev_msg()
{
    printf("%s, %d\n", __FILE__, __LINE__);
    m_json->create_json_updatestatus_device(1);
    std::string jsonstr = m_json->get_jsonstr();
    std::replace(jsonstr.begin(), jsonstr.end(), '\n', ' ');
    mqtt_send(jsonstr, 0);
    return 0;
}

int mqtt_pub::mqtt_pub_date_upload_msg(frame_info* Frame_info)
{
    printf("%s, %d\n", __FILE__, __LINE__);
    m_json->create_json_data_upload(Frame_info);
    std::string jsonstr = m_json->get_jsonstr();
    std::replace(jsonstr.begin(), jsonstr.end(), '\n', ' ');
    mqtt_send(jsonstr, DATA);
    return 0;
}

int mqtt_pub::mqtt_pub_event_upload_msg(frame_info* Frame_info)
{
    printf("%s, %d\n", __FILE__, __LINE__);
    m_json->create_json_event_upload(Frame_info);
    std::string jsonstr = m_json->get_jsonstr();
    std::replace(jsonstr.begin(), jsonstr.end(), '\n', ' ');
    mqtt_send(jsonstr, EVENT);
    return 0;
}

int mqtt_pub::mqtt_pub_status_update(int status)
{
    printf("%s, %d\n", __FILE__, __LINE__);
    m_json->create_json_updatestatus_device(status);
    std::string jsonstr = m_json->get_jsonstr();
    std::replace(jsonstr.begin(), jsonstr.end(), '\n', ' ');
    mqtt_send(jsonstr, EVENT);
    return 0;
}

int mqtt_pub::mqtt_pub_status_ipc(int status)
{
    printf("%s, %d\n", __FILE__, __LINE__);
    m_json->create_json_updatestatus_ipc(status);
    std::string jsonstr = m_json->get_jsonstr();
    std::replace(jsonstr.begin(), jsonstr.end(), '\n', ' ');
    mqtt_send(jsonstr, EVENT);
    return 0;
}

int mqtt_pub::mqtt_pub_sysinfo_upload_msg(float usercpu, float syscpu, float mem)
{
    printf("%s, %d\n", __FILE__, __LINE__);
    m_json->create_json_sysinfo_upload(usercpu, syscpu, mem);
    std::string jsonstr = m_json->get_jsonstr();
    std::replace(jsonstr.begin(), jsonstr.end(), '\n', ' ');
    mqtt_send(jsonstr, DATA);
    return 0;
}
