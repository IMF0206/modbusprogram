#include "modbus_handler.h"
#include <string>
#include <pthread.h>
#include "mqtt_pub.h"
#include "mqtt_plat.h"
#include "commonfunc.h"

#define ADDRESS     "tcp://172.16.20.40:1883"
#define CLIENTID    "ExampleClientPub"
#define TOPIC       "topictests/aaa/bbb"
#define PAYLOAD     "Hello man, can you see me?"
#define QOS         1
#define TIMEOUT     10000L
#define EVENT 1
#define DATA 2

using namespace std;

modbus_handler::modbus_handler()
{
    m_dbhelper = new db_helper("./iot.db");
    m_mqttpub = new mqtt_pub();
    m_mqttplat = new mqtt_plat();
    m_checknum = 0;
}

modbus_handler::~modbus_handler()
{
    if (m_dbhelper != NULL)
    {
        delete m_dbhelper;
        m_dbhelper = NULL;
    }
    if (m_mqttpub != NULL)
    {
        delete m_mqttpub;
        m_mqttpub = NULL;
    }
    if (m_mqttplat != NULL)
    {
        delete m_mqttplat;
        m_mqttplat = NULL;
    }

}

int modbus_handler::modbus_handler_init()
{
    std::string sqltype = "select id from protocol where type=1";
    m_dbhelper->sql_exec_with_return(sqltype);
    m_protocol_type = m_dbhelper->getsqlresult()[0];
    while(1)
    {
        // string sqlcmd = "select comtype from port where protocol=0;";
        // m_dbhelper->sql_exec_multicol_return(sqlcmd);
        // vector<string> comtypevec = m_dbhelper->getsqlresult();
        // printf("size of comtype is:%d \n", comtypevec.size());
        // sqlcmd = "select name from port where protocol=0;";
        // m_dbhelper->sql_exec_multicol_return(sqlcmd);
        // // 为modbus-RTU时需要知道name代表的串口号
        // vector<string> namevec = m_dbhelper->getsqlresult();
        // sqlcmd = "select ipaddr from port where protocol=0;";
        // m_dbhelper->sql_exec_multicol_return(sqlcmd);
        // vector<string> ipaddrvec = m_dbhelper->getsqlresult();
        // sqlcmd = "select port from port where protocol=0;";
        // m_dbhelper->sql_exec_multicol_return(sqlcmd);
        // vector<string> portvec = m_dbhelper->getsqlresult();
        string sqlcmd = "select id from port where protocol=" + m_protocol_type + ";";
        m_dbhelper->sql_exec_multicol_return(sqlcmd);
        vector<string> idvec = m_dbhelper->getsqlresult();
        int checknum = 0;
        for (auto it : idvec)
        {
            sqlcmd = "select id from node where portid = " + it + ";";
            printf("==========%d ================ %s\n", __LINE__, sqlcmd.c_str());
            m_dbhelper->sql_exec_multicol_return(sqlcmd);
            vector<string> nodeidvec = m_dbhelper->getsqlresult();
            int i = 0;
            for (auto itnode : nodeidvec)
            {
                i++;
                sqlcmd = "select param1 from node where id = " + itnode + ";";
                printf("==========%d ================ %s\n", __LINE__, sqlcmd.c_str());
                m_dbhelper->sql_exec_with_return(sqlcmd);
                if (m_dbhelper->getsqlresult().empty())
                {
                    if (isNum(itnode))
                        m_nodeid_portaddr[stoi(itnode)] = "1";
                }
                else
                {
                    if (isNum(itnode))
                        m_nodeid_portaddr[stoi(itnode)] = m_dbhelper->getsqlresult()[0];
                }
                printf("==========93============%s\n", itnode.c_str());
                if (isNum(itnode))
                    checknum += stoi(itnode);
            }
        }
        printf("%s======%d==========checknum:[%d], m_checknum:[%d]\n", __FILE__, __LINE__, checknum, m_checknum);
        if (checknum == m_checknum)
        {
            printf("============continue:%d\n", __LINE__);
            sleep(10);
            continue;
        }
        int index = 0;
        printf("=======__LINE__:%d=====================%d=================\n", __LINE__, m_nodeid_portaddr.size());
        for (auto i = m_nodeid_portaddr.begin(); i != m_nodeid_portaddr.end(); i++)
        {
            m_mb_vec.push_back(NULL);
            sqlcmd = "select portid from node where id = " + std::to_string(i->first) + ";";
            m_dbhelper->sql_exec_with_return(sqlcmd);
            string idstr = m_dbhelper->getsqlresult()[0];
            printf("========%s=========%s=========\n", sqlcmd.c_str(), m_dbhelper->getsqlresult()[0].c_str());
            m_bConnected.push_back(false);
            sqlcmd = "select comtype from port where id = " + m_dbhelper->getsqlresult()[0] + ";";
            printf("=========%s================\n", sqlcmd.c_str());
            m_dbhelper->sql_exec_with_return(sqlcmd);
            string comtype = m_dbhelper->getsqlresult()[0];
            sqlcmd = "select parity from port where id = " + m_dbhelper->getsqlresult()[0] + ";";
            m_dbhelper->sql_exec_with_return(sqlcmd);
            string parity = "";
            if (m_dbhelper->getsqlresult().empty())
            {
                parity = "none";
            }
            else
            {
                parity = m_dbhelper->getsqlresult()[0];
            }
            string ipaddrstr = "";
            // 用的串口，默认是RTU
            if (comtype.compare("1") == 0)
            {
                sqlcmd = "select baudrate from port where id=" + idstr + ";";
                m_dbhelper->sql_exec_with_return(sqlcmd);
                BackendParams *ibackend = NULL;
                if (parity.compare("odd") == 0)
                {
                    ibackend = createRtuBackend(stoi(m_dbhelper->getsqlresult()[0]), 8, 1, 'E');
                }
                else if (parity.compare("even") == 0)
                {
                    ibackend = createRtuBackend(stoi(m_dbhelper->getsqlresult()[0]), 8, 1, 'O');
                }
                else
                {
                    ibackend = createRtuBackend(stoi(m_dbhelper->getsqlresult()[0]), 8, 1, 'N');
                }
                m_backendvec.push_back(ibackend);
            }
            else if (comtype.compare("2") == 0)
            {
                sqlcmd = "select ipaddr from port where id=" + idstr + ";";
                m_dbhelper->sql_exec_with_return(sqlcmd);
                printf("===%s===\n", sqlcmd.c_str());
                ipaddrstr = m_dbhelper->getsqlresult()[0];
                sqlcmd = "select port from port where id=" + idstr + ";";
                printf("===%s===\n", sqlcmd.c_str());
                m_dbhelper->sql_exec_with_return(sqlcmd);
                string portstr = m_dbhelper->getsqlresult()[0];
                BackendParams *ibackend = createTcpBackend(ipaddrstr.c_str(), stoi(portstr));
                m_backendvec.push_back(ibackend);
            }

            printf("===============m_backendvec size:[%d], index is : %d\n\n", m_backendvec.size(), index);
            if (NULL != m_backendvec[index])
            {
                if (Rtu == m_backendvec[index]->type)
                {
                    sqlcmd = "select name from port where id=" + idstr + ";";
                    m_dbhelper->sql_exec_with_return(sqlcmd);
                    printf("sqlcmd is %s\n", sqlcmd.c_str());
                    string namestr = m_dbhelper->getsqlresult()[0];
                    RtuBackend *rtuP = (RtuBackend*)m_backendvec[index];
                    strcpy(rtuP->devName, namestr.c_str());
                    // modbus_t *ctx = m_backendvec[i]->createCtxt(m_backendvec[i]);
                }
                else if (Tcp == m_backendvec[index]->type) {
                    printf("Tcp=====%d, ipaddr : [%s]\n", __LINE__, ipaddrstr.c_str());
                    TcpBackend *tcpP = (TcpBackend*)m_backendvec[index];
                    strcpy(tcpP->ip, ipaddrstr.c_str());
                }
            }
            index++;
        }
        m_checknum = checknum;
        sleep(10);
    }

}

int modbus_handler::modbus_handler_connect()
{
    // int ret, startAddr = 0;
    // string sqlcmd = "select param2 from port where protocol=0;";
    // m_dbhelper->sql_exec_multicol_return(sqlcmd);
    // vector<string> param2vec = m_dbhelper->getsqlresult();
    while(1)
    {
        printf("m_backendvec size is [%d]\n", m_backendvec.size());
        if (m_nodeid_portaddr.size() != m_backendvec.size())
        {
            printf("something error, m_backendvec != m_nodeid_portaddr size, check\n");
            sleep(5);
            continue;
        }
        auto it = m_nodeid_portaddr.begin();
        for (int i = 0; i < m_backendvec.size(); i++)
        {
            printf("m_nodeid_portaddr first : %d, second : %s\n", it->first, it->second.c_str());
            if (m_bConnected[i] == true)
            {
                printf("============__LINE__:%d================\n", __LINE__);
                continue;
            }
            modbus_t *ctx = m_backendvec[i]->createCtxt(m_backendvec[i]);
            if (it->second.empty() || isNum(it->second))
            {
                // modbus_set_slave(ctx, stoi(param2vec[i]));
                modbus_set_slave(ctx, 1);
            }
            else
            {
                modbus_set_slave(ctx, stoi(it->second));
            }

            if (modbus_connect(ctx) == -1)
            {
                fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
                modbus_free(ctx);
                // return -1;
            }
            else
            {
                m_bConnected[i] = true;
                printf("i:[%d], file:%s, line:%d, addr : %p\n", i, __FILE__, __LINE__, ctx);
                m_mb_vec[i] = ctx;
                m_mqttpub->mqtt_pub_login_msg();
                m_mqttplat->mqtt_platadddev();
                m_mqttplat->mqtt_platupdatedev(1);
            }
            it++;
        }

        if (m_mb_vec.size() != m_nodeid_portaddr.size())
        {
            printf("something error, m_backendvec != m_nodeid_portaddr size, check\n");
            sleep(5);
            continue;
        }
        auto iter = m_nodeid_portaddr.begin();
        for (int i = 0; i < m_mb_vec.size(); i++)
        {
            string sqlcmd = "select portid from node where id = " + std::to_string(iter->first) + ";";
            m_dbhelper->sql_exec_with_return(sqlcmd);
            string idstr = m_dbhelper->getsqlresult()[0];
            printf("m_nodeid_portaddr first : %d, idstr : %s\n", iter->first, idstr.c_str());
            // param1获取功能码
            sqlcmd = "select param1 from port where id=" + idstr + ";";
            printf("=========%d=============%s\n", __LINE__, sqlcmd.c_str());
            m_dbhelper->sql_exec_with_return(sqlcmd);
            string param1 = "";
            if (!m_dbhelper->getsqlresult().empty())
            {
                param1 = m_dbhelper->getsqlresult()[0];
            }
            if (m_fTypevec.size() < i + 1)
            {
                printf("param1 is [%s]\n", param1.c_str());
                if (isNum(param1))
                {
                    m_fTypevec.push_back((FuncType)stoi(param1));
                }
                else
                {
                    m_fTypevec.push_back(FuncNone);
                }

            }
            else
            {
                if (isNum(param1))
                {
                    m_fTypevec[i] = (FuncType)stoi(param1);
                }
                else
                {
                    m_fTypevec[i] = FuncNone;
                }
                
            }
            FuncType fType = (FuncType)m_fTypevec[0];
            printf("fType:[%d]\n", fType);
            if (m_wdatetypevec.empty())
            {
                m_wdatetypevec.push_back(DataInt);
            }
            switch (fType) {
            case(ReadCoils):
                m_wdatetypevec[i] = Data8Array;
                break;
            case(ReadDiscreteInput):
                m_wdatetypevec[i] = DataInt;
                break;
            case(ReadHoldingRegisters):
            case(ReadInputRegisters):
                m_wdatetypevec[i] = Data16Array;
                break;
            case(WriteSingleCoil):
            case(WriteSingleRegister):
                m_wdatetypevec[i] = DataInt;
                break;
            case(WriteMultipleCoils):
                m_wdatetypevec[i] = Data8Array;
                break;
            case(WriteMultipleRegisters):
                m_wdatetypevec[i] = Data16Array;
                break;
            default:
                m_wdatetypevec[i] = DataError;
                printf("No correct function type chosen");
                // exit(EXIT_FAILURE);
            }

            switch (m_wdatetypevec[i]) {
            case (DataInt):
                //no need to alloc anything
                // 此处分配空间是为了占位，与数据处理做匹配
                if (m_datavec.empty())
                {
                    Data mydata;
                    mydata.data8 = (uint8_t*)malloc(1 * sizeof(uint8_t));
                    // m_datavec[0].data8 = (uint8_t*)malloc(99 * sizeof(uint8_t));
                    m_datavec.push_back(mydata);
                }
                else
                {
                    m_datavec[i].data8 = (uint8_t*)malloc(1 * sizeof(uint8_t));
                }
                break;
            case (Data8Array):
                if (m_datavec.empty())
                {
                    Data mydata;
                    mydata.data8 = (uint8_t*)malloc(99 * sizeof(uint8_t));
                    // m_datavec[0].data8 = (uint8_t*)malloc(99 * sizeof(uint8_t));
                    m_datavec.push_back(mydata);
                }
                else
                {
                    m_datavec[i].data8 = (uint8_t*)malloc(99 * sizeof(uint8_t));
                }
                break;
            case (Data16Array):
                if (m_datavec.empty())
                {
                    Data mydata;
                    mydata.data16 = (uint16_t*)malloc(99 * sizeof(uint16_t));
                    m_datavec.push_back(mydata);
                }
                else
                {
                    m_datavec[i].data16 = (uint16_t*)malloc(99 * sizeof(uint16_t));
                }
                break;
            case (DataError):
                // 此处分配空间是为了占位，与数据处理做匹配
                if (m_datavec.empty())
                {
                    Data mydata;
                    mydata.data8 = (uint8_t*)malloc(1 * sizeof(uint8_t));
                    // m_datavec[0].data8 = (uint8_t*)malloc(99 * sizeof(uint8_t));
                    m_datavec.push_back(mydata);
                }
                else
                {
                    m_datavec[i].data8 = (uint8_t*)malloc(1 * sizeof(uint8_t));
                }
            default:
                printf("Data alloc error!\n");
                // exit(EXIT_FAILURE);
            }
            iter++;
        }
        sleep(30);
    }
    
}


int modbus_handler::modbus_data_process()
{

    // int ret, startAddr = 0;
    // // param1获取功能码
    // string sqlcmd = "select param1 from port where protocol=0;";
    // m_dbhelper->sql_exec_multicol_return(sqlcmd);
    // for (int i = 0; i < m_dbhelper->getsqlresult().size(); i++)
    // {
    //     m_fTypevec.push_back((FuncType)stoi(m_dbhelper->getsqlresult()[i]));
    // }
    // // param2获取读取的地址
    // sqlcmd = "select param2 from port where protocol=0;";
    // m_dbhelper->sql_exec_multicol_return(sqlcmd);
    // vector<string> param2vec = m_dbhelper->getsqlresult();
    // // 先考虑一个
    // // TODO
    // if (m_fTypevec.empty())
    // {
    //     printf("error, m_fTypevc is empty\n");
    //     return -1;
    // }
    // FuncType fType = (FuncType)m_fTypevec[0];
    // printf("fType:[%d]\n", fType);
    // if (m_wdatetypevec.empty())
    // {
    //     m_wdatetypevec.push_back(DataInt);
    // }
    // switch (fType) {
    // case(ReadCoils):
    //     m_wdatetypevec[0] = Data8Array;
    //     break;
    // case(ReadDiscreteInput):
    //     m_wdatetypevec[0] = DataInt;
    //     break;
    // case(ReadHoldingRegisters):
    // case(ReadInputRegisters):
    //     m_wdatetypevec[0] = Data16Array;
    //     break;
    // case(WriteSingleCoil):
    // case(WriteSingleRegister):
    //     m_wdatetypevec[0] = DataInt;
    //     break;
    // case(WriteMultipleCoils):
    //     m_wdatetypevec[0] = Data8Array;
    //     break;
    // case(WriteMultipleRegisters):
    //     m_wdatetypevec[0] = Data16Array;
    //     break;
    // default:
    //     printf("No correct function type chosen");
    //     // exit(EXIT_FAILURE);
    // }

    // switch (m_wdatetypevec[0]) {
    // case (DataInt):
    //     //no need to alloc anything
    //     break;
    // case (Data8Array):
    //     if (m_datavec.empty())
    //     {
    //         Data mydata;
    //         mydata.data8 = (uint8_t*)malloc(99 * sizeof(uint8_t));
    //         // m_datavec[0].data8 = (uint8_t*)malloc(99 * sizeof(uint8_t));
    //         m_datavec.push_back(mydata);
    //     }
    //     else
    //     {
    //         m_datavec[0].data8 = (uint8_t*)malloc(99 * sizeof(uint8_t));
    //     }
    //     break;
    // case (Data16Array):
    //     if (m_datavec.empty())
    //     {
    //         Data mydata;
    //         mydata.data16 = (uint16_t*)malloc(99 * sizeof(uint16_t));
    //         m_datavec.push_back(mydata);
    //     }
    //     else
    //     {
    //         m_datavec[0].data16 = (uint16_t*)malloc(99 * sizeof(uint16_t));
    //     }
    //     break;
    // default:
    //     printf("Data alloc error!\n");
    //     // exit(EXIT_FAILURE);
    // }
    // printf("===========line:%d==================%d\n", __LINE__, m_mb_vec.size());
    int ret, startAddr = 0;
    while(1)
    {
        printf("modbus_data_process loop ============ \n");
        if (m_mb_vec.empty())
        {
            printf("empty===========line:%d==================%d\n", __LINE__, m_mb_vec.size());
            sleep(3);
            continue;
        }
        // int readWriteNo = stoi(param2vec[0]);
        int readWriteNo = 2;
        bool isWriteFunction = false;
        auto iter = m_nodeid_portaddr.begin();
        for (int i = 0; i < m_mb_vec.size(); i++)
        {
            m_data_map.clear();
            modbus_t *ctx = m_mb_vec[i];

            if (m_bConnected[i] && ctx != NULL)
            {
                printf("modbus_data_process loop\n");
                switch (m_fTypevec[i]) {
                case(ReadCoils):
                    printf("ReadCoils\n");
                    ret = modbus_read_bits(ctx, startAddr, readWriteNo, m_datavec[i].data8);
                    break;
                case(ReadDiscreteInput):
                    printf("ReadDiscreteInput: not implemented yet!\n");
                    m_wdatetypevec[i] = DataInt;
                    break;
                case(ReadHoldingRegisters):
                    printf("ReadHoldingRegisters\n");
                    printf("ctx addr = [%p], startAddr = [%d], readWriteNo = [%d]\n", ctx, startAddr, readWriteNo);
                    ret = modbus_read_holdingdata(ctx, iter->first, startAddr + 1, m_datavec[i].data16);
                    break;
                case(ReadInputRegisters):
                    printf("ReadInputRegisters\n");
                    ret = modbus_read_input_registers(ctx, startAddr, readWriteNo, m_datavec[i].data16);
                    break;
                case(WriteSingleCoil):
                    isWriteFunction = true;
                    ret = modbus_write_bit(ctx, startAddr, m_datavec[i].dataInt);
                    break;
                case(WriteSingleRegister):
                    isWriteFunction = true;
                    ret = modbus_write_register(ctx, startAddr, m_datavec[i].dataInt);
                    break;
                case(WriteMultipleCoils):
                    isWriteFunction = true;
                    ret = modbus_write_bits(ctx, startAddr, readWriteNo, m_datavec[i].data8);
                    break;
                case(WriteMultipleRegisters):
                    isWriteFunction = true;
                    ret = modbus_write_registers(ctx, startAddr, readWriteNo, m_datavec[i].data16);
                    break;
                default:
                    printf("No correct function type chosen");
                }
                if (ret < 0)
                {
                    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! read failed \n");
                    m_bConnected[i] = false;
                }
                printf("--------%d   %d\n", ret, readWriteNo);
                if (ret == readWriteNo) {//success
                    printf("pengjialing\n");
                    if (isWriteFunction)
                        printf("SUCCESS: written %d elements!\n", readWriteNo);
                    else {
                        printf("SUCCESS: read %d of elements:\n\tData: ", readWriteNo);
                        int i = 0;
                        if (DataInt == m_wdatetypevec[0]) {
                            printf("0x%04x\n", m_datavec[0].dataInt);
                        }
                        else {
                            const char Format8[] = "0x%02x ";
                            const char Format16[] = "0x%04x ";
                            const char *format = ((Data8Array == m_wdatetypevec[0]) ? Format8 : Format16);
                            for (; i < readWriteNo; ++i) {
                                printf(format, (Data8Array == m_wdatetypevec[0]) ? m_datavec[0].data8[i] : m_datavec[0].data16[i]);
                            }
                            printf("\n");
                        }
                    }
                }
                printf("test\n");
            }
            else
            {
                iter++;
                continue;
            }
            string sqlcmd = "select deviceid from node where id=" + std::to_string(iter->first) + ";";
            printf("=========%d=======%s=======\n", __LINE__, sqlcmd.c_str());
            m_dbhelper->sql_exec_with_return(sqlcmd);
            if (m_data_map.empty())
            {
                printf("m_data_map is empty\n");
                iter++;
                sleep(5);
                continue;
            }
            string jsonstr = m_mdb2mqtt.getmqttstr(m_dbhelper->getsqlresult()[0], m_data_map);
            printf("mqtt json is : %s\n", jsonstr.c_str());
            m_mqttpub->mqtt_send(jsonstr, DATA);
            iter++;
            
        }
        sleep(5);
    }
    
}

static void* myinit(void *handler)
{
    modbus_handler* myhandler = (modbus_handler*)handler;
    myhandler->modbus_handler_init();
}

static void* myconnect(void *handler)
{
    modbus_handler* myhandler = (modbus_handler*)handler;
    myhandler->modbus_handler_connect();
}

static void* myprocess(void *handler)
{
    modbus_handler* myhandler = (modbus_handler*)handler;
    myhandler->modbus_data_process();
}

int modbus_handler::modbus_start()
{
    pthread_t initpid, pid1, pid2;
    int ret = pthread_create(&initpid, NULL, myinit, this);
    ret = pthread_create(&pid1, NULL, myconnect, this);
    if (ret < 0)
    {
        printf("Create connect pthread error\n");
        return -1;
    }
    sleep(1);
    ret = pthread_create(&pid2, NULL, myprocess, this);
    if (ret < 0)
    {
        printf("Create process pthread error\n");
        return -1;
    }
    while(1)
    {
        sleep(10);
    }
}

int modbus_handler::modbus_read_holdingdata(modbus_t* ctx, int nodeid, int startaddr, uint16_t *dest)
{
    printf("%s===========nodeid: %d========%d========modbus_read_holdingdata\n", __FILE__, nodeid, __LINE__);
    string sql = "select id from port where protocol=" + m_protocol_type + ";";
    m_dbhelper->sql_exec_with_return(sql);
    if (m_dbhelper->getsqlresult().size() <= sqlresid)
    {
        printf("sqlresid is over result size\n");
        return -1;
    }
    if (m_dbhelper->getsqlresult()[sqlresid].empty())
    {
        printf("port id is empty\n");
        return -1;
    }

    sql = "select id from node where portid=" + m_dbhelper->getsqlresult()[sqlresid] + ";";
    printf("%s\n", sql.c_str());
    m_dbhelper->sql_exec_with_return(sql);
    if (m_dbhelper->getsqlresult().empty())
    {
        printf("port id not found in node table\n");
        return -1;
    }
    sql = "select propertyid from nodeproperty where nodeid=" + m_dbhelper->getsqlresult()[sqlresid] + ";";
    printf("%s\n", sql.c_str());
    m_dbhelper->sql_exec_with_return(sql);
    if (m_dbhelper->getsqlresult().empty())
    {
        printf("no propertyid found in node table\n");
        return -1;
    }
    std::vector<std::string> propertyidvec = m_dbhelper->getsqlresult();
    sql = "select param1 from property where propertyid=" + propertyidvec[sqlresid] + ";";
    printf("%s\n", sql.c_str());
    m_dbhelper->sql_exec_with_return(sql);
    std::vector<std::string> offsetvec = m_dbhelper->getsqlresult();
    sql = "select name from property where propertyid=" + propertyidvec[sqlresid] + ";";
    printf("%s\n", sql.c_str());
    m_dbhelper->sql_exec_with_return(sql);
    std::vector<std::string> namevec = m_dbhelper->getsqlresult();
    printf("==========line:%d================%s, namevec.size is [%d], m_wdatetypevec[i]: %d\n", __LINE__, sql.c_str(), namevec.size(), m_wdatetypevec[sqlresid]);
    int ret = -1;
    for (int i = 0; i < namevec.size(); i++)
    {
        printf("offsetvec[%d] is : %s\n", i, offsetvec[i].c_str());
        // ret = modbus_read_registers(ctx, stoi(offsetvec[i]), 2, dest);
        ret = modbus_read_registers(ctx, 0, 2, dest);
        // uint16_t test[2] = {0x0020, 0xF147};
        // float testresult = modbus_get_float(test);
        // printf("testresult : [%f]\n", testresult);
        // testresult = modbus_get_float_abcd(test);
        // printf("testresult : [%f]\n", testresult);
        // testresult = modbus_get_float_dcba(test);
        // printf("testresult : [%f]\n", testresult);
        // testresult = modbus_get_float_badc(test);
        // printf("testresult : [%f]\n", testresult);
        // testresult = modbus_get_float_cdab(test);
        // printf("testresult : [%f]\n", testresult);
        float result = modbus_get_float_abcd(dest);
        
        m_data_map[namevec[i]] = result;
        const char Format8[] = "0x%02x ";
        const char Format16[] = "0x%04x ";
        const char *format = ((Data8Array == m_wdatetypevec[0]) ? Format8 : Format16);
        for (int j = 0; j < 2; ++j) {
            printf(format, (Data8Array == m_wdatetypevec[0]) ? m_datavec[0].data8[j] : m_datavec[0].data16[j]);

        }
        printf("\n");
        printf("map name is : %s, result is : %f, dest is : [0x%04x ]\n", namevec[i].c_str(), result, m_datavec[0].data16);
        startaddr =+ 2;
    }

    return startaddr;
}