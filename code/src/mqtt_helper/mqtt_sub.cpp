#include "mqtt_sub.h"
#include <pthread.h>
#include <string.h>
#include "mqtt_json.h"
#include "sqlite_helper.h"
#include <string>
#include "MQTTClient.h"
#include <unistd.h>

#define ADDRESS     "tcp://192.168.7.31:1883"
#define CLIENTID    "ExampleClientSub"
#define TOPIC       "/v1/#"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

struct mqtt_sub_info
{
    std::string topic;
    std::string response;
};


int sub_cmd(std::string cmd, std::string result)
{
    char buffer[4096];
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    return -1;
    while(!feof(pipe)) {
        if(fgets(buffer, 4096, pipe)){
            result = buffer;
        }
    }
    pclose(pipe);
    return 0;
}

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    printf("Message arrived\n");
    printf("topic: %s\n", topicName);
    printf("message: %d.*s\n", message->payloadlen, (char*)message->payload);
    mqtt_json mqttJson;
    mqttJson.parse_json_device_response((char*)message->payload);
    db_helper db(DB_FILE_PATH);
    device_resp* deviceResp = mqttJson.get_deviceresp();
    std::string ucSql = "";
    if (deviceResp->type.compare("CMD_TOPO_ADD") == 0)
    {
        for (int i = 0; i < deviceResp->result_num; i++)
        {
            ucSql = "update node set deviceid = '" + deviceResp->results[i].nodeId + "' where nodeid = '"+ deviceResp->results[i].nodeId +"'";
            if (db.sql_exec_with_return(ucSql.c_str()))
            {
                ucSql.clear();
                ucSql = "insert into node (nodeId, deviceId) values (" + deviceResp->results[i].nodeId + ", " + deviceResp->results[i].nodeId + ");";
            }
        }
    }
    else if (deviceResp->type.compare("CMD_TOPO_DEL") == 0)
    {
        for (int i = 0; i < deviceResp->result_num; i++)
        {
            ucSql = "delete from node where deviceid = '" + deviceResp->results[i].nodeId + "'";
            db.sql_exec_with_return(ucSql.c_str());
        }
    }
    else if (deviceResp->type.compare("CMD_TOPO_UPDATE") == 0)
    {
        for (int i = 0; i < deviceResp->result_num; i++)
        {
            ucSql = "update node set status = '" + std::to_string(deviceResp->results[i].statuscode) + "' where deviceid = " + deviceResp->results[i].nodeId + "'";
            db.sql_exec_with_return(ucSql.c_str());
        }
    }
    else
    {
        printf("Nothing to do\n");
    }
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

void* mqtt_sub_thread(void* arg)
{
    printf("==========\n");
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    if ((rc = MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to create client, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto exit;
    }

    if ((rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto destroy_exit;
    }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto destroy_exit;
    }

    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    if ((rc = MQTTClient_subscribe(client, TOPIC, QOS)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to subscribe, return code %d\n", rc);
    	rc = EXIT_FAILURE;
    }
    else
    {
    	int ch;
        while(1)
        {
            sleep(5);
        }
    	// do
    	// {
        // 	ch = getchar();
    	// } while (ch!='Q' && ch != 'q');

        if ((rc = MQTTClient_unsubscribe(client, TOPIC)) != MQTTCLIENT_SUCCESS)
        {
            printf("Failed to unsubscribe, return code %d\n", rc);
            rc = EXIT_FAILURE;
        }
    }

    if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to disconnect, return code %d\n", rc);
    	rc = EXIT_FAILURE;
    }
destroy_exit:
    MQTTClient_destroy(&client);
exit:
    return NULL;
}

int mqtt_sub::mqtt_init()
{
    pthread_t tid;
    mqtt_sub_thread(NULL);
    pthread_create(&tid, NULL, mqtt_sub_thread, (void*)"mqtt_sub_thread");
    return 0;
}

