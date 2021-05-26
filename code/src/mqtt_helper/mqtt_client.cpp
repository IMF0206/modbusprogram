#include "mqtt_client.h"
#include "mqtt_json.h"
#include <algorithm>
#include "sqlite_helper.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "mqtt_plat_parse.h"

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

#define TOPIC_ADDRSP     "addResponse"
#define TOPIC_UPDATERSP  "updateResponse"
#define TOPIC_DELETE     "delete"
#define TOPIC_QUERYRSP   "queryResponse"
#define TOPIC_COMMAND    "command"

using namespace std;

mqtt_client::mqtt_client()
{
    m_json = new mqtt_json();
    m_dbhelper = new db_helper(DB_FILE_PATH);
}

mqtt_client::~mqtt_client()
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

extern int ssl_error_cb(const char *str, size_t len, void *u);

extern volatile MQTTClient_deliveryToken deliveredtoken;

extern void delivered(void *context, MQTTClient_deliveryToken dt);

int msgarrvd_plat(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    printf("Message arrived\n");
    printf("topic: %s\n", topicName);
    printf("message: %d.*s\n", message->payloadlen, (char*)message->payload);
    string topicstr = topicName;
    
    mqtt_json mqttJson;
    mqttJson.parse_json_device_response((char*)message->payload);
    db_helper db(DB_FILE_PATH);
    device_resp* deviceResp = mqttJson.get_deviceresp();
    std::string ucSql = "";
    mqtt_plat_parse plat_parse;
    if (topicstr.find(TOPIC_ADDRSP) != string::npos)
    {
        plat_parse.mqtt_plat_parse_add_rsp((char*)message->payload);
        
    }
    else if (topicstr.find(TOPIC_COMMAND) != string::npos)
    {
        plat_parse.mqtt_plat_parse_command((char*)message->payload);
    }
    else if (topicstr.find(TOPIC_DELETE) != string::npos)
    {
        plat_parse.mqtt_plat_parse_del((char*)message->payload);
    }
    else if (topicstr.find(TOPIC_QUERYRSP) != string::npos)
    {
        plat_parse.mqtt_plat_parse_query_rsp((char*)message->payload);
    }
    else if (topicstr.find(TOPIC_UPDATERSP) != string::npos)
    {
        plat_parse.mqtt_plat_parse_update_rsp((char*)message->payload);
    }
    else
    {
        printf("Nothing to do\n");
    }
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

extern void connlost(void *context, char *cause);

void* mqtt_client_sub_loop(void* arg)
{
    printf("==========\n");
    mqtt_client* client = (mqtt_client*)arg;
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    client->mqtt_client_subscribe("/v1/#", QOS);
    while (1)
    {
        sleep(5);
    }
}

int mqtt_client::mqtt_client_open(std::string addr, std::string clientid, std::string username, unsigned char* passwd)
{
    int rc;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
   
    // MQTTClient_create(&m_client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_create(&m_client, addr.c_str(), clientid.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_setCallbacks(m_client, NULL, connlost, msgarrvd_plat, delivered);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = username.c_str();
    conn_opts.password = (const char*)passwd;
    conn_opts.ssl->ssl_error_cb = ssl_error_cb;
    if ((rc = MQTTClient_connect(m_client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        return -1;
    }
    printf("Success to connect!\n");
    return 0;
}

int mqtt_client::mqtt_client_publish(char *top, int qos, char *msg, int len)
{
    MQTTClient_deliveryToken token;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;

    if (m_client == NULL)
        return -1;

    pubmsg.payload = msg;
    pubmsg.payloadlen = len;
    pubmsg.qos = qos;
    pubmsg.retained = 0;
    deliveredtoken = 0;
    MQTTClient_publishMessage(m_client, top, &pubmsg, &token);
    printf("Waiting for publication of %s\n"
            "on topic %s for client\n",
            msg, top);
    while (deliveredtoken != token);
    return 0;
}

int mqtt_client::mqtt_client_subscribe(char *top, int qos)
{
    if (m_client == NULL)
	return -1;

    MQTTClient_subscribe(m_client, top, qos);
    return 0;
}

int mqtt_client::mqtt_client_disconn()
{
    MQTTClient_disconnect(m_client, 10000);
    MQTTClient_destroy(&m_client);
}

int mqtt_client::mqtt_client_sub_thread()
{
    pthread_t tid;
    pthread_create(&tid, NULL, mqtt_client_sub_loop, (void*)this);
}
