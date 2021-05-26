#include "mqtt_plat.h"
#include <time.h>
#include "algo_hmac.h"
#include <malloc.h>

#define QOS         1

using namespace std;

mqtt_plat::mqtt_plat()
{
    m_logined = false;
    m_json = new mqtt_json();
    m_dbhelper = new db_helper(DB_FILE_PATH);
    m_myclient = new mqtt_client();
    m_passwd = NULL;
}

mqtt_plat::~mqtt_plat()
{
    m_logined = false;
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
    if (m_myclient)
    {
        delete m_myclient;
        m_myclient = NULL;
    }
    if (m_passwd)
    {
        free(m_passwd);
        m_passwd = NULL;
    }
}

std::string mqtt_plat::get_timestamp()
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
    sprintf(current_time, "%04d%02d%02d%02d", (1900+tmp_ptr->tm_year), (1+tmp_ptr->tm_mon), tmp_ptr->tm_mday,
     tmp_ptr->tm_hour);
	return std::string(current_time);
}

int mqtt_plat::mqtt_platconnect()
{
    // 查询数据库平台表数据
    m_dbhelper->sql_exec_multicol_return("select * from platinfo;");
    // 获取平台地址信息
    string plataddr = m_dbhelper->getsqlresult()[1];
    string platmqttport = m_dbhelper->getsqlresult()[2];
    // 获取鉴权类型
    string authtype = m_dbhelper->getsqlresult()[3];
    // 获取签名类型
    string signtype = m_dbhelper->getsqlresult()[4];
    // 获取secret值
    string secret = m_dbhelper->getsqlresult()[5];
    // 获取gatewayId
    m_gatewayId = m_dbhelper->getsqlresult()[6];
    // 获取device
    m_dbhelper->sql_exec_with_return("select deviceid from edgedev;");
    string deviceid = m_dbhelper->getsqlresult()[0];
    string timestamp = get_timestamp();
    // 拼接登录的clientid
    string clientidstr = deviceid + authtype + signtype + timestamp;
    // 获取加密后的值
	unsigned int passwdlen = 0;
    HmacEncode("sha256", timestamp.c_str(), timestamp.length(), secret.c_str(), secret.length(), m_passwd, passwdlen);

    m_myclient->mqtt_client_open("tcp://" + plataddr + ":" + platmqttport, clientidstr, deviceid, m_passwd);

}

int mqtt_plat::mqtt_platdisconnect()
{
    m_myclient->mqtt_client_disconn();
}

int mqtt_plat::mqtt_platadddev()
{
    string topic = "/v1/devices/" + m_gatewayId + "/topo/add";
    m_json->create_json_plat_adddev();
    string addstr = m_json->get_json4plat();
    m_myclient->mqtt_client_publish((char*)topic.c_str(), QOS, (char*)addstr.c_str(), addstr.size());
    return 0;
}

int mqtt_plat::mqtt_platupdatedev(int status)
{
    string topic = "/v1/devices/" + m_gatewayId + "/topo/update";
    m_json->create_json_plat_update(status);
    string updatestr = m_json->get_json4plat();
    m_myclient->mqtt_client_publish((char*)topic.c_str(), QOS, (char*)updatestr.c_str(), updatestr.size());
    return 0;
}

int mqtt_plat::mqtt_platreqdev()
{
    string topic = "/v1/devices/" + m_gatewayId + "/topo/query";
    m_json->create_json_plat_query();
    string querystr = m_json->get_json4plat();
    m_myclient->mqtt_client_publish((char*)topic.c_str(), QOS, (char*)querystr.c_str(), querystr.size());
    return 0;
}

int mqtt_plat::mqtt_platcommandresp()
{
    string topic = "/v1/devices/" + m_gatewayId + "/topo/commandResponse";
    m_json->create_json_plat_command_resp();
    string commandstr = m_json->get_json4plat();
    m_myclient->mqtt_client_publish((char*)topic.c_str(), QOS, (char*)commandstr.c_str(), commandstr.size());
    return 0;
}

int mqtt_plat::mqtt_platdatasend(frame_info* Frame_info)
{
    string topic = "/v1/devices/" + m_gatewayId + "/topo/datas";
    m_json->create_json_plat_date(Frame_info);
    string datastr = m_json->get_json4plat();
    m_myclient->mqtt_client_publish((char*)topic.c_str(), QOS, (char*)datastr.c_str(), datastr.size());
    return 0;
}

int mqtt_plat::mqtt_plat_sub_init()
{
    m_myclient->mqtt_client_sub_thread();
}