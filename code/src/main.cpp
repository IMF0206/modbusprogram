#define _BSD_SOURCE
#include<netinet/in.h>  
#include<sys/socket.h>  
#include<unistd.h>  
#include <signal.h>
  
#include<stdio.h>
#include <stdlib.h>
#include<string.h>  

#include "sqlite_helper.h"

#include <arpa/inet.h>

#include "mqtt_pub.h"
#include "mqtt_sub.h"

#include "mqtt_plat_parse.h"
#include "mqtt_plat.h"
// #include "http_client.h"

#include <memory.h>
#include "modbus_handler.h"

// 安全测试用，正常为9999
#define SOCKET_PORT 19999
#define WUGUAN_IP "192.168.7.31"
#define WUGUAN_PORT 23106

// int STATUS = 0;
// unsigned char ack_reply[] = {0x68, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x59, 0x16};

// unsigned char login_msg[] = {0x68, 0x01, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x01, 0x00, 0x11, 0x8E, 0x70, 0x16};  
// void listener_cb(evconnlistener *listener, evutil_socket_t fd,  
//                  struct sockaddr *sock, int socklen, void *arg);  
  
// void socket_read_cb(struct bufferevent *bev, void *arg);  
// void socket_event_cb(struct bufferevent *bev, short events, void *arg);

int64_t hexToDec(unsigned char *source, int len)
{
    int64_t sum = 0;
    int64_t t = 1;
    int i;
 
    for(i = 0; i < len; i++)
    {
        printf("%d\n", source[i]);
        sum += t*source[i];
        t *= 16*16;
        printf("t : %lld, sum : %lld\n", t, sum);
    }
 
    return sum;
}

long long int getS(int e,int m)
{
    long long int s=e;
    for (int i=1;i<m;i++)
    {
        s*=e;
    }
    return s;
}

// 16进制转float
float hex2float(unsigned char*p,float* result)
{
    long long int a=0x00000000;

    a=a|p[0];
    a=(a<<8)|p[1];
    a=(a<<8)|p[2];
    a=(a<<8)|p[3];

    //获得符号位，1表示负数，0表示正数
    int s=(a>>31)&0xFF;
    int e=(a>>23)&0x0FF;
    //获得指数
    e=e-127;
    //获得底数，0x800000是添加隐藏位
    long long int m=a&0x7FFFFF|0x800000;
    long long int c=0;
    float v = 0.0f, y = 1.0f;
    //向右移动
    if (e>=0)
    {
        //获得整数的二进制
        c=(m>>(23-e))&0xFFFFFFF;
        long int b=0;
        for (int i=0;i<23-e;i++)
        {
            b=(b<<1)|0x01;
        }
        //获得小数的二进制
        b=b&m;
        int j=0;
        for (int i=23-e-1;i>=0;i--)
        {
            j++;
            y=(double)(((b>>i)&0x01)*getS(2,j));
            if (y>0.0)
            {
                v+=1.0/y;
            }
        }
        v=c+v;
        if (s>0)
        {
            v=-v;
        }
    }
    else
    {
        //向左移动
        e=-e;
        c=m;
        int j=0;
        for (int i=23+e-1;i>=0;i--)
        {
            j++;
            y=(float)(((c>>i)&0x01)*getS(2,j));
            if (y>0.0)
            {
                v+=1.0/y;
            }
        }
        if (s>0)
        {
            v=-v;
        }
    }

    *result = v;
    return v;
}

// void parse_frame(unsigned char *frame, int len, frame_info* frameinfo)
// {
//     frameinfo->head_info = frame[0];
//     frameinfo->address_info[3] = frame[1];
//     frameinfo->address_info[2] = frame[2];
//     frameinfo->address_info[1] = frame[3];
//     frameinfo->address_info[0] = frame[4];
//     frameinfo->frame_num = frame[5];
//     frameinfo->frame_des = frame[6];
//     frameinfo->frame_type = frame[7];
//     unsigned char datanum[2] = {frame[8], frame[9]};
//     frameinfo->data_num = hexToDec(datanum, 2);
//     int dateindex = 10;
//     // 4位为节点地址数据长度
//     if (frameinfo->data_num < 4)
//     {
//         frameinfo->cmd = frame[dateindex];
//         dateindex++;
//     }
//     else
//     {
//         frameinfo->node_info[0] = frame[dateindex + 3];
//         frameinfo->node_info[1] = frame[dateindex + 2];
//         frameinfo->node_info[2] = frame[dateindex + 1];
//         frameinfo->node_info[3] = frame[dateindex];
//         // 命令码
//         frameinfo->cmd = frame[dateindex + 4];
//         dateindex += 5;
//     }
    
//     frameinfo->frame_data = (unsigned char*)malloc(frameinfo->data_num - dateindex + 10);
//     int loopnum = frameinfo->data_num - dateindex + 10;
//     for (int i = 0; i < loopnum; i++)
//     {
//         frameinfo->frame_data[i] = frame[dateindex];
//         dateindex++;
//     }
// }

// 校验
unsigned short CRC16_plc(unsigned char bufInOut[], unsigned short sizeIn)
{
    unsigned short crc =0xffff;
    unsigned short i,j;
    unsigned char byte1, crcbit, databit;
    for (i = 0; i < sizeIn; i++)
    { 
        byte1 = bufInOut[i];
        for (j = 0; j < 8; j++)
        { 
            crcbit = crc & 0x8000 ? 1 : 0;
            databit = byte1 & 0x80 ? 1 : 0;
            crc = crc << 1;
            if (crcbit != databit)
            { 
                crc = crc ^ 0x1021; //CCITT CRC-16 standard : X^16 + X^12 + X^5 + 1 = 2^16 + 2^12 + 2^5 + 1 = 0x11021
            }
            byte1 = byte1 << 1;
        }
    }
    return crc;
}

//一个新客户端连接上服务器了  
//当此函数被调用时，libevent已经帮我们accept了这个客户端。该客户端的
//文件描述符为fd
// void listener_cb(evconnlistener *listener, evutil_socket_t fd,  
//                  struct sockaddr *sock, int socklen, void *arg)  
// {
// 	//读取连接信息

//     struct sockaddr_in * adds=(struct sockaddr_in *)sock;
//     char* ips = NULL;
//     ips = inet_ntoa(adds->sin_addr);
//     printf("%s\n", ips);

//     printf("accept a client %d\n", fd);
//     // 发送传感器上线消息
//     mqtt_pub mqttpub;
//     mqttpub.mqtt_pub_login_msg();
//     mqtt_plat mqttplat;
//     mqttplat.mqtt_platadddev();
//     mqttplat.mqtt_platupdatedev(1);
//     sleep(1);
//     mqttpub.mqtt_pub_status_update(1);
  
//     struct event_base *base = (struct event_base*)arg;
  
//     //为这个客户端分配一个bufferevent  
//     struct bufferevent *bev =  bufferevent_socket_new(base, fd,  
//                                                BEV_OPT_CLOSE_ON_FREE);  
  
//     bufferevent_setcb(bev, socket_read_cb, NULL, socket_event_cb, NULL);
//     bufferevent_enable(bev, EV_READ | EV_PERSIST);
// }

// void* event_thread(void* arg)
// {
//     //evthread_use_pthreads();//enable threads
//     char test[26] = {0x34, 0x30, 0x30, 0x32, 0x2E, 0x35, 0x39, 0x37, 0x33, 0x31, 0x2C, 0x4E, 0x2C, 0x31, 0x31, 0x36, 0x32, 0x31, 0x2E, 0x30, 0x36, 0x32, 0x31, 0x38, 0x2C, 0x45};
//     for (int i = 0; i < 26; i++)
//     {
//         printf("%c", test[i]);
//     }
//     printf("\n");
  
//     struct sockaddr_in sin;
//     memset(&sin, 0, sizeof(struct sockaddr_in));  
//     sin.sin_family = AF_INET;  
//     sin.sin_port = htons(SOCKET_PORT);
  
//     struct event_base *base = event_base_new();
//     struct evconnlistener *listener  
//             = evconnlistener_new_bind(base, listener_cb, (void*)base,  
//                                       LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,  
//                                       10, (struct sockaddr*)&sin,  
//                                       sizeof(struct sockaddr_in));  
//     event_base_dispatch(base);
    
//     evconnlistener_free(listener);
//     event_base_free(base);
// }

std::string getValueBySystemCommand(std::string strCommand){
    //printf("-----%s------\n",strCommand.c_str());
    std::string strData = "";
    FILE * fp = NULL;
    char buffer[128];
    fp=popen(strCommand.c_str(),"r");
    if (fp) {
        while(fgets(buffer,sizeof(buffer),fp)){
            strData.append(buffer);
        }
        pclose(fp);
    }
    //printf("-----%s------ end\n",strCommand.c_str());
    return strData;
}


// int runSystemCommand(std::string strCommand){
// //    printf("-----%s------\n",strCommand.c_str());
//     return system(strCommand.c_str());
// }

std::string trim(std::string str)
{
    if (str.empty())
    {
        return str;
    }
    str.erase(0,str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);
    return str;
}

std::vector<std::string> split(std::string str, std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;
    int size = str.size();
    
    for (int i = 0; i < size; i++) {
        pos = str.find(pattern, i);
        if (pos < size) {
            std::string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    
    return result;
}

float getcpugap(){
    std::string strRes = getValueBySystemCommand("top -b -n 1 |grep Cpu | cut -d \",\" -f 1 | cut -d \":\" -f 2");
   
    strRes = trim(strRes);
    printf("--%s--  strRes.size : %lu \n", strRes.c_str(),strRes.length());
    std::vector<std::string> vecT = split(strRes," ");
    if (vecT.size() == 2){
        printf("%s\n", vecT.at(0).c_str());
        return atof(vecT.at(0).c_str());
    }
    return 0;
}

float getusercpugap(){
    std::string strRes = getValueBySystemCommand("top -b -n 1 |grep Cpu | cut -d \",\" -f 1 | cut -d \":\" -f 2");
   
    strRes = trim(strRes);
    printf("--%s--  strRes.size : %lu \n", strRes.c_str(),strRes.length());
    std::vector<std::string> vecT = split(strRes," ");
    if (vecT.size() == 2){
        printf("%s\n", vecT.at(0).c_str());
        return atof(vecT.at(0).c_str());
    }
    return 0;
}

float getsyscpugap(){
    std::string strRes = getValueBySystemCommand("top -b -n 1 |grep Cpu | cut -d \",\" -f 2 | cut -d \":\" -f 2");
   
    strRes = trim(strRes);
    printf("--%s--  strRes.size : %lu \n", strRes.c_str(),strRes.length());
    std::vector<std::string> vecT = split(strRes," ");
    if (vecT.size() == 2){
        printf("%s\n", vecT.at(0).c_str());
        return atof(vecT.at(0).c_str());
    }
    return 0;
}

float getmemgap(){
    std::string strRes = getValueBySystemCommand("top -b -n 1 |grep \"KiB Mem\" | cut -d \",\" -f 2 | cut -d \":\" -f 2");
   
    strRes = trim(strRes);
    printf("--%s--  strRes.size : %lu \n", strRes.c_str(),strRes.length());
    std::vector<std::string> vecT = split(strRes," ");
    if (vecT.size() == 2){
        printf("%s\n", vecT.at(0).c_str());
        return atof(vecT.at(0).c_str());
    }
    return 0;
}

// void* sysstatus_pthread(void* arg)
// {
//     printf("start \n");
//     while (1)
//     {
//         signal(SIGCHLD, SIG_IGN);
//         signal(SIGPIPE, SIG_IGN);
//         signal(SIGILL, SIG_IGN);

//         // std::string pstr = "%Cpu(s):  0.4 us,  0.9 sy,  0.0 ni, 98.7 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st";
//         float cpuusergap = getusercpugap();
//         float cpusysgap = getsyscpugap();
//         float memgap = getmemgap();
//         printf("%f, %f, %f\n", cpuusergap, cpusysgap, memgap);
//         mqtt_pub mqttpub;
//         mqttpub.mqtt_pub_sysinfo_upload_msg(cpuusergap, cpusysgap, memgap);
//         sleep(120);
//     }
// }

int main()  
{
    mqtt_plat mqttplat;
    mqttplat.mqtt_plat_sub_init();
    modbus_handler *modbushandler = new modbus_handler();
    modbushandler->modbus_handler_init();
    printf("start\n");
    modbushandler->modbus_start();
    // mqttSub.mqtt_init();
    while(1)
    {
        sleep(10);
    }
    return 0;
}
  
  
// void socket_read_cb(struct bufferevent *bev, void *arg)  
// {
//     printf("socket_read_cb\n");
//     unsigned char msg[4096];  
  
//     size_t len = bufferevent_read(bev, msg, sizeof(msg)-1 );
//     printf("server read the data size : %d\n", len);
//     msg[len] = '\0';
//     for(int i = 0; i < len; i++) 
//     {
//         printf("%02X ", msg[i]);            
//     }
//     printf("\n");

//     printf("==========\n");
//     printf("server read the data %s\n", msg);

//     frame_info Frame_info;
//     parse_frame(msg, len - 1, &Frame_info);
//     mqtt_pub mqttPub;
//     db_helper dbHelper(DB_FILE_PATH);
//     mqtt_plat mqttplat;
//     std::string sql = "";
//     unsigned char bufInOut[10] = {msg[10], msg[11], msg[12], msg[13], 0x01, 0x00, 0x00, 0x00, 0x00, msg[14]};
//     unsigned char bufAckOut[14] = {0};
//     bufAckOut[0] = 0x68;
//     unsigned short crc = 0;
//     float tension = 0, angleX = 0, angleY = 0, anglexheld = 0, angleyheld = 0;
//     unsigned char tensionarr[4] = {0};
//     unsigned char anglexarr[4] = {0};
//     unsigned char angleyarr[4] = {0};
//     switch(Frame_info.cmd)
//     {
//         case 0:
//             printf("设备类型\n");
//             // ACK帧
//             bufferevent_write(bev, ack_reply, sizeof(ack_reply));
//             mqttPub.mqtt_pub_get_dev_msg();
//             mqttplat.mqtt_platupdatedev(1);
//             break;
//         case 1:
//             printf("拉力告警\n");
//             // ACK帧
//             bufferevent_write(bev, ack_reply, sizeof(ack_reply));
//             // 确认帧
//             crc = CRC16_plc(bufInOut, sizeof(bufInOut));
//             for (int i = 0; i < 6; i++)
//             {
//                 bufAckOut[1 + i] = bufInOut[i];
//             }
//             bufAckOut[7] = 0x07;
//             bufAckOut[8] = 0x01;
//             bufAckOut[9] = 0x00;
//             bufAckOut[10] = Frame_info.cmd;
//             bufAckOut[11] = crc&0xFF;
//             bufAckOut[12] = (crc>>8)&0xFF;
//             bufAckOut[13] = 0x16;
//             bufferevent_write(bev, bufAckOut, sizeof(bufAckOut));
//             mqttPub.mqtt_pub_event_upload_msg(&Frame_info);
//             break;
//         case 2:
//             printf("角度告警\n");
//             // ACK帧
//             bufferevent_write(bev, ack_reply, sizeof(ack_reply));
//             // 确认帧
//             crc = CRC16_plc(bufInOut, sizeof(bufInOut));
//             for (int i = 0; i < 6; i++)
//             {
//                 bufAckOut[1 + i] = bufInOut[i];
//             }
//             bufAckOut[7] = 0x07;
//             bufAckOut[8] = 0x01;
//             bufAckOut[9] = 0x00;
//             bufAckOut[10] = Frame_info.cmd;
//             bufAckOut[11] = crc&0xFF;
//             bufAckOut[12] = (crc>>8)&0xFF;
//             bufAckOut[13] = 0x16;
//             bufferevent_write(bev, bufAckOut, sizeof(bufAckOut));
//             mqttPub.mqtt_pub_event_upload_msg(&Frame_info);
//             break;
//         case 3:
//             printf("上报间隔\n");
//             break;
//         case 4:
//             printf("声光报警\n");
//             break;
//         case 5:
//             printf("拉力数据\n");
//             bufferevent_write(bev, ack_reply, sizeof(ack_reply));
//             mqttPub.mqtt_pub_date_upload_msg(&Frame_info);
//             tensionarr[0] = Frame_info.frame_data[3];
//             tensionarr[1] = Frame_info.frame_data[2];
//             tensionarr[2] = Frame_info.frame_data[1];
//             tensionarr[3] = Frame_info.frame_data[0];
//             hex2float(tensionarr, &tension);
//             sql = "select threshold from property where name = 'tension' and nodeid = '1100001000170014'";
//             if (dbHelper.sql_exec_with_return(sql.c_str()));
//             {
//                 printf("get node count error!\n");
//                 break;
//             }
//             if (std::stof(dbHelper.getsqlresult()[0]) < tension)
//             {
//                 Frame_info.cmd = 1;
//                 mqttPub.mqtt_pub_event_upload_msg(&Frame_info);
//             }
//             mqttplat.mqtt_platdatasend(&Frame_info);
//             break;
//         case 6:
//             printf("角度数据\n");
//             // ACK帧
//             bufferevent_write(bev, ack_reply, sizeof(ack_reply));
//             mqttPub.mqtt_pub_date_upload_msg(&Frame_info);
//             anglexarr[0] = Frame_info.frame_data[3];
//             anglexarr[1] = Frame_info.frame_data[2];
//             anglexarr[2] = Frame_info.frame_data[1];
//             anglexarr[3] = Frame_info.frame_data[0];
//             angleyarr[0] = Frame_info.frame_data[7];
//             angleyarr[1] = Frame_info.frame_data[6];
//             angleyarr[2] = Frame_info.frame_data[5];
//             angleyarr[3] = Frame_info.frame_data[4];
//             hex2float(anglexarr, &angleX);
//             hex2float(angleyarr, &angleY);
//             sql = "select threshold from property where name = 'angleX' and nodeid = '1100001000170014'";
//             if (dbHelper.sql_exec_with_return(sql.c_str()));
//             {
//                 printf("get node count error!\n");
//                 break;
//             }
//             anglexheld = std::stof(dbHelper.getsqlresult()[0]);
//             sql = "select threshold from property where name = 'angleY' and nodeid = '1100001000170014'";
//             if (dbHelper.sql_exec_with_return(sql.c_str()));
//             {
//                 printf("get node count error!\n");
//                 break;
//             }
//             angleyheld = std::stof(dbHelper.getsqlresult()[0]);
//             if (angleX > anglexheld || angleY > angleyheld)
//             {
//                 Frame_info.cmd = 2;
//                 mqttPub.mqtt_pub_event_upload_msg(&Frame_info);
//             }
//             mqttplat.mqtt_platdatasend(&Frame_info);
//             break;
//         case 7:
//             printf("声光报警自动启动\n");
//             break;
//         case 8:
//             printf("电池阈值\n");
//             break;
//         case 9:
//         {
//             printf("当前电池电压值+电量\n");
//             // ACK帧
//             bufferevent_write(bev, ack_reply, sizeof(ack_reply));
//             mqttPub.mqtt_pub_date_upload_msg(&Frame_info);
//         }

//             break;
//         case 10:
//             printf("角度数据\n");
//             break;
//         case 11:
//             printf("设备地址\n");
//             break;
//         case 12:
//             printf("运营商APN\n");
//             break;
//         case 13:
//             printf("运营商用户名和密码\n");
//             break;
//         case 14:
//             printf("远程服务器 IP和端口号（4G）\n");
//             break;
//         case 15:
//             printf("本地服务器IP和端口号(lwip)\n");
//             break;
//         case 16:
//             printf("调试服务器IP和端口号(未启用)\n");
//             break;
//         case 17:
//         {
//             printf("登入帧\n");
//             // ACK帧
//             bufferevent_write(bev, ack_reply, sizeof(ack_reply));
//             sql = "SELECT count(*) from node;";
//             if (dbHelper.sql_exec_with_return(sql.c_str()));
//             {
//                 printf("get node count error!\n");
//                 return;
//             }
//             std::vector<std::string> res = dbHelper.getsqlresult();
//             int nodenum = std::stoi(res[0]);
//             if (nodenum <= 0)
//             {
//                 printf("error, 数据库未配置\n");
//                 return;
//             }
//             sql.clear();
//             sql = "SELECT deviceid from node;";
//             if (dbHelper.sql_exec_with_return(sql.c_str()))
//             {
//                 printf("get deviceid error!\n");
                
//                 return;
//             }
//             // 数据目前为假，TODO
//             mqttPub.mqtt_pub_login_msg();
//             mqttplat.mqtt_platadddev();
//             break;
//         }
            
//         case 18:
//             printf("心跳帧\n");
//             // ACK帧
//             bufferevent_write(bev, ack_reply, sizeof(ack_reply));
//             break;
//         case 19:
//             printf("心跳间隔\n");
//             break;
//         case 20:
//             printf("本机IP\n");
//             break;
//         case 21:
//             printf("本机子网掩码\n");
//             break;
//         case 22:
//             printf("本机默认网关\n");
//             break;
//         case 45:
//         {
//             printf("定位信息\n");
//             bufferevent_write(bev, ack_reply, sizeof(ack_reply));
//             mqttPub.mqtt_pub_date_upload_msg(&Frame_info);
//             break;
//         }
//         case 100:
//             printf("倾角传感器零点\n");
//             break;
//         case 101:
//             printf("倾角传感器零点调教确认\n");
//             break;
//         default:
//             printf("其他消息，未定义");
//     }

//     return;
//     // if (strcmp(login_msg, msg) == 0)
//     // {
//     //     printf("==========\n");
//     //     printf("server read the data %s\n", msg);  
  
//     //     bufferevent_write(bev, ack_reply, sizeof(ack_reply));
//     //     return;
//     // }
//     // printf("server read the data %s\n", msg);  
  
//     // char reply[] = "I has read your data";  
//     // bufferevent_write(bev, reply, sizeof(reply));
// }


// void socket_event_cb(struct bufferevent *bev, short events, void *arg)  
// {  
//     if (events & BEV_EVENT_EOF)  
//         printf("connection closed\n");  
//     else if (events & BEV_EVENT_ERROR)  
//         printf("some other error\n");
  
//     //这将自动close套接字和free读写缓冲区  
//     bufferevent_free(bev);
//     // 更新端设备状态为离线
//     // TODO
//     mqtt_pub mqttpub;
//     mqttpub.mqtt_pub_status_update(0);
// }