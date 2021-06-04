#include "modbus_handler.h"
#include <string>
#include <pthread.h>
#include "mqtt_pub.h"
#include "mqtt_plat.h"

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
    while(1)
    {
        string sqlcmd = "select comtype from port where protocol=0;";
        m_dbhelper->sql_exec_multicol_return(sqlcmd);
        vector<string> comtypevec = m_dbhelper->getsqlresult();
        printf("size of comtype is:%d \n", comtypevec.size());
        sqlcmd = "select name from port where protocol=0;";
        m_dbhelper->sql_exec_multicol_return(sqlcmd);
        // 为modbus-RTU时需要知道name代表的串口号
        vector<string> namevec = m_dbhelper->getsqlresult();
        sqlcmd = "select ipaddr from port where protocol=0;";
        m_dbhelper->sql_exec_multicol_return(sqlcmd);
        vector<string> ipaddrvec = m_dbhelper->getsqlresult();
        sqlcmd = "select port from port where protocol=0;";
        m_dbhelper->sql_exec_multicol_return(sqlcmd);
        vector<string> portvec = m_dbhelper->getsqlresult();
        sqlcmd = "select id from port where protocol=0;";
        m_dbhelper->sql_exec_multicol_return(sqlcmd);
        vector<string> idvec = m_dbhelper->getsqlresult();

        if (comtypevec.size() == m_mb_vec.size())
        {
            printf("============continue:%d\n", __LINE__);
            sleep(10);
            continue;
        }
        // for (int i = 0; i < comtypevec.size(); i++)
        for (int i = 0; i < 1; i++)
        {
            m_mb_vec.push_back(NULL);
            printf("========%d=========%s=========\n", m_mb_vec.size(), comtypevec[i].c_str());
            m_bConnected.push_back(false);

            // 用的串口，默认是RTU
            // if (comtypevec[i].compare("0") == 0)
            // {
            //     sqlcmd = "select baudrate from port where id=%s" + idvec[i] + ";";
            //     m_dbhelper->sql_exec_with_return(sqlcmd);
            //     BackendParams *ibackend = createRtuBackend(stoi(m_dbhelper->getsqlresult()[0]), 8, 1, 'E');
            //     m_backendvec.push_back(ibackend);
            // }
            // else if (comtypevec[i].compare("1") == 0)
            // {
                printf("=============%d, i:%d\n", __LINE__, i);
                printf("=================%s\n", ipaddrvec[i].c_str());
                printf("================%s\n", portvec[i].c_str());
                BackendParams *ibackend = createTcpBackend(ipaddrvec[i].c_str(), stoi(portvec[i]));
                m_backendvec.push_back(ibackend);
            // }

            if (NULL != m_backendvec[i])
            {
                if (Rtu == m_backendvec[i]->type)
                {
                    RtuBackend *rtuP = (RtuBackend*)m_backendvec[i];
                    strcpy(rtuP->devName, namevec[i].c_str());
                    // modbus_t *ctx = m_backendvec[i]->createCtxt(m_backendvec[i]);
                }
                else if (Tcp == m_backendvec[i]->type) {
                    printf("Tcp=====%d\n", __LINE__);
                    TcpBackend *tcpP = (TcpBackend*)m_backendvec[i];
                    strcpy(tcpP->ip, ipaddrvec[i].c_str());
                }
            }
        }
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
        string sqlcmd = "select param2 from port where protocol=0;";
        m_dbhelper->sql_exec_multicol_return(sqlcmd);
        vector<string> param2vec = m_dbhelper->getsqlresult();
        for (int i = 0; i < m_backendvec.size(); i++)
        {
            if (m_bConnected[i] == true)
            {
                printf("============__LINE__:%d================\n", __LINE__);
                continue;
            }
            modbus_t *ctx = m_backendvec[i]->createCtxt(m_backendvec[i]);
            if (!param2vec[i].empty())
            {
                // modbus_set_slave(ctx, stoi(param2vec[i]));
                modbus_set_slave(ctx, 1);
            }
            else
            {
                modbus_set_slave(ctx, 1);
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
                printf("file:%s, line:%d, addr : %p\n", __FILE__, __LINE__, ctx);
                m_mb_vec[i] = ctx;
                m_mqttpub->mqtt_pub_login_msg();
                m_mqttplat->mqtt_platadddev();
                m_mqttplat->mqtt_platupdatedev(1);
            }
        }
        sleep(5);
    }
    
}


int modbus_handler::modbus_data_process()
{
    int ret, startAddr = 0;
    string sqlcmd = "select param1 from port where protocol=0;";
    m_dbhelper->sql_exec_multicol_return(sqlcmd);
    for (int i = 0; i < m_dbhelper->getsqlresult().size(); i++)
    {
        m_fTypevec.push_back((FuncType)stoi(m_dbhelper->getsqlresult()[i]));
    }
    // param2获取读取的地址
    sqlcmd = "select param2 from port where protocol=0;";
    m_dbhelper->sql_exec_multicol_return(sqlcmd);
    vector<string> param2vec = m_dbhelper->getsqlresult();
    // 先考虑一个
    // TODO
    if (m_fTypevec.empty())
    {
        printf("error, m_fTypevc is empty\n");
        return -1;
    }
    FuncType fType = (FuncType)m_fTypevec[0];
    printf("fType:[%d]\n", fType);
    if (m_wdatetypevec.empty())
    {
        m_wdatetypevec.push_back(DataInt);
    }
    switch (fType) {
    case(ReadCoils):
        m_wdatetypevec[0] = Data8Array;
        break;
    case(ReadDiscreteInput):
        m_wdatetypevec[0] = DataInt;
        break;
    case(ReadHoldingRegisters):
    case(ReadInputRegisters):
        m_wdatetypevec[0] = Data16Array;
        break;
    case(WriteSingleCoil):
    case(WriteSingleRegister):
        m_wdatetypevec[0] = DataInt;
        break;
    case(WriteMultipleCoils):
        m_wdatetypevec[0] = Data8Array;
        break;
    case(WriteMultipleRegisters):
        m_wdatetypevec[0] = Data16Array;
        break;
    default:
        printf("No correct function type chosen");
        // exit(EXIT_FAILURE);
    }

    switch (m_wdatetypevec[0]) {
    case (DataInt):
        //no need to alloc anything
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
            m_datavec[0].data8 = (uint8_t*)malloc(99 * sizeof(uint8_t));
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
            m_datavec[0].data16 = (uint16_t*)malloc(99 * sizeof(uint16_t));
        }
        break;
    default:
        printf("Data alloc error!\n");
        // exit(EXIT_FAILURE);
    }
    printf("===========line:%d==================%d\n", __LINE__, m_mb_vec.size());
    modbus_t *ctx = m_mb_vec[0];
    printf("file:%s, line:%d, addr : %p\n", __FILE__, __LINE__, ctx);
    // int readWriteNo = stoi(param2vec[0]);
    int readWriteNo = 2;
    bool isWriteFunction = false;
    while(1)
    {
        m_data_map.clear();
        if (m_bConnected[0] && ctx != NULL)
        {
            switch (m_fTypevec[0]) {
            case(ReadCoils):
                printf("ReadCoils\n");
                ret = modbus_read_bits(ctx, startAddr, readWriteNo, m_datavec[0].data8);
                break;
            case(ReadDiscreteInput):
                printf("ReadDiscreteInput: not implemented yet!\n");
                m_wdatetypevec[0] = DataInt;
                break;
            case(ReadHoldingRegisters):
                printf("ReadHoldingRegisters\n");
                // ret = modbus_read_registers(ctx, startAddr, readWriteNo, m_datavec[0].data16);
                ret = modbus_read_holdingdata(ctx, 0, startAddr + 1, m_datavec[0].data16);
                break;
            case(ReadInputRegisters):
                printf("ReadInputRegisters\n");
                ret = modbus_read_input_registers(ctx, startAddr, readWriteNo, m_datavec[0].data16);
                break;
            case(WriteSingleCoil):
                isWriteFunction = true;
                ret = modbus_write_bit(ctx, startAddr, m_datavec[0].dataInt);
                break;
            case(WriteSingleRegister):
                isWriteFunction = true;
                ret = modbus_write_register(ctx, startAddr, m_datavec[0].dataInt);
                break;
            case(WriteMultipleCoils):
                isWriteFunction = true;
                ret = modbus_write_bits(ctx, startAddr, readWriteNo, m_datavec[0].data8);
                break;
            case(WriteMultipleRegisters):
                isWriteFunction = true;
                ret = modbus_write_registers(ctx, startAddr, readWriteNo, m_datavec[0].data16);
                break;
            default:
                printf("No correct function type chosen");
            }
            if (ret < 0)
            {
                m_bConnected[0] = false;
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
            modbus_t *ctx = m_mb_vec[0];
        }
        sqlcmd = "select id from port where protocol=0;";
        m_dbhelper->sql_exec_multicol_return(sqlcmd);
        if (m_data_map.empty())
        {
            printf("m_data_map is empty\n");
            sleep(5);
            continue;
        }
        string jsonstr = m_mdb2mqtt.getmqttstr(m_dbhelper->getsqlresult()[0], m_data_map);
        printf("mqtt json is : %s\n", jsonstr.c_str());
        m_mqttpub->mqtt_send(jsonstr, DATA);
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

int modbus_handler::modbus_read_holdingdata(modbus_t* ctx, int sqlresid, int startaddr, uint16_t *dest)
{
    printf("%s===================%d========modbus_read_holdingdata\n", __FILE__, __LINE__);
    string sql = "select id from port where protocol = 0;";
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
    printf("========line:%d=====%s=====", __LINE__, sql.c_str());
    m_dbhelper->sql_exec_with_return(sql);
    if (m_dbhelper->getsqlresult().empty())
    {
        printf("no propertyid found in node table\n");
        return -1;
    }
    std::vector<std::string> propertyidvec = m_dbhelper->getsqlresult();
    sql = "select param1 from property where propertyid=" + propertyidvec[sqlresid] + ";";
    m_dbhelper->sql_exec_with_return(sql);
    std::vector<std::string> offsetvec = m_dbhelper->getsqlresult();
    sql = "select name from property where propertyid=" + propertyidvec[sqlresid] + ";";
    m_dbhelper->sql_exec_with_return(sql);
    std::vector<std::string> namevec = m_dbhelper->getsqlresult();
    printf("==========line:%d================%s\n", __LINE__, sql.c_str());
    int ret = -1;
    for (int i = 0; i < m_dbhelper->getsqlresult().size(); i++)
    {
        printf("offsetvec[%d] is : %s\n", i, offsetvec[i].c_str());
        // ret = modbus_read_input_registers(ctx, stoi(offsetvec[i]), 2, dest);
        ret = modbus_read_input_registers(ctx, 0, 2, dest);
        float result = modbus_get_float(dest);
        m_data_map[namevec[i]] = result;
        const char Format8[] = "0x%02x ";
        const char Format16[] = "0x%04x ";
        const char *format = ((Data8Array == m_wdatetypevec[0]) ? Format8 : Format16);
        for (int j = 0; j < 2; ++j) {
            printf(format, (Data8Array == m_wdatetypevec[0]) ? m_datavec[0].data8[j] : m_datavec[0].data16[j]);
        }
        printf("\n");
        printf("map name is : %s, result is : %f, dest is : [0x%04x ]\n", namevec[i].c_str(), result, dest);
        startaddr =+ 2;
    }
    // string sql = "select param2 from port where protocol=0";
    // m_dbhelper->sql_exec_multicol_return(sql);
    // if (m_dbhelper->getsqlresult().size() <= sqlresid)
    // {
    //     printf("sqlresid is over result size\n");
    //     return -1;
    // }
    // if (m_dbhelper->getsqlresult()[sqlresid].empty())
    // {
    //     printf("param2 is empty\n");
    //     return 0;
    // }
    // sql = "select offset from property where name='" + m_dbhelper->getsqlresult()[sqlresid] + "';";
    // m_dbhelper->sql_exec_with_return(sql);
    // if (m_dbhelper->getsqlresult().empty())
    // {
    //     printf("error:no offset found in the db\n");
    //     return -1;
    // }
    // int offset = stoi(m_dbhelper->getsqlresult()[0]);
    // int ret = modbus_read_input_registers(ctx, startaddr, offset, dest);
    // if (ret != offset)
    // {
    //     printf("read number not right\n");
    //     return -1;
    // }
    // float result = modbus_get_float(dest);
    // m_data_map[m_dbhelper->getsqlresult()[sqlresid]] = result;
    // startaddr += offset;

    // sql = "select param3 from port where protocol=0";
    // m_dbhelper->sql_exec_multicol_return(sql);
    // if (m_dbhelper->getsqlresult().size() <= sqlresid)
    // {
    //     printf("sqlresid is over result size\n");
    //     return startaddr;
    // }
    // if (m_dbhelper->getsqlresult()[sqlresid].empty())
    // {
    //     printf("param3 is empty\n");
    //     return 0;
    // }
    // sql = "select offset from property where name='" + m_dbhelper->getsqlresult()[sqlresid] + "';";
    // m_dbhelper->sql_exec_with_return(sql);
    // offset = stoi(m_dbhelper->getsqlresult()[0]);
    // ret = modbus_read_input_registers(ctx, startaddr, offset, dest);
    // if (ret != offset)
    // {
    //     printf("read number not right\n");
    //     return -1;
    // }
    // result = modbus_get_float(dest);
    // m_data_map[m_dbhelper->getsqlresult()[sqlresid]] = result;
    // startaddr += offset;

    // sql = "select param4 from port where protocol=0";
    // m_dbhelper->sql_exec_multicol_return(sql);
    // if (m_dbhelper->getsqlresult().size() <= sqlresid)
    // {
    //     printf("sqlresid is over result size\n");
    //     return startaddr;
    // }
    // if (m_dbhelper->getsqlresult()[sqlresid].empty())
    // {
    //     printf("param4 is empty\n");
    //     return 0;
    // }
    // sql = "select offset from property where name='" + m_dbhelper->getsqlresult()[sqlresid] + "';";
    // m_dbhelper->sql_exec_with_return(sql);
    // offset = stoi(m_dbhelper->getsqlresult()[0]);
    // ret = modbus_read_input_registers(ctx, startaddr, offset, dest);
    // if (ret != offset)
    // {
    //     printf("read number not right\n");
    //     return -1;
    // }
    // result = modbus_get_float(dest);
    // m_data_map[m_dbhelper->getsqlresult()[sqlresid]] = result;
    // startaddr += offset;

    return startaddr;
}