#include "modbus_handler.h"
#include <string>

using namespace std;

modbus_handler::modbus_handler()
{
    m_dbhelper = new db_helper("/opt/nvr/iot.db");

}

modbus_handler::~modbus_handler()
{
    if (m_dbhelper != NULL)
    {
        delete m_dbhelper;
        m_dbhelper = NULL;
    }

}

int modbus_handler::modbus_handler_init()
{
    string sqlcmd = "select comtype from port where protocol=0;";
    m_dbhelper->sql_exec_multicol_return(sqlcmd);
    vector<string> comtypevec = m_dbhelper->getsqlresult();
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

    for (int i = 0; i < comtypevec.size(); i++)
    {
        // 用的串口，默认是RTU
        if (comtypevec[i].compare("0") == 0)
        {
            sqlcmd = "select baudrate from port where id=%s" + idvec[i] + ";";
            m_dbhelper->sql_exec_with_return(sqlcmd);
            BackendParams *ibackend = createRtuBackend(stoi(m_dbhelper->getsqlresult()[0]), 8, 1, 'E');
            m_backendvec.push_back(ibackend);
        }
        else if (comtypevec[i].compare("1") == 1)
        {
            BackendParams *ibackend = createTcpBackend(ipaddrvec[i].c_str(), stoi(portvec[i]));
            m_backendvec.push_back(ibackend);
        }

        if (NULL != m_backendvec[i])
        {
            if (Rtu == m_backendvec[i]->type)
            {
                RtuBackend *rtuP = (RtuBackend*)m_backendvec[i];
                strcpy(rtuP->devName, namevec[i].c_str());
                modbus_t *ctx = m_backendvec[i]->createCtxt(m_backendvec[i]);
            }
            else if (Tcp == m_backendvec[i]->type) {
                TcpBackend *tcpP = (TcpBackend*)m_backendvec[i];
                strcpy(tcpP->ip, ipaddrvec[i].c_str());
            }
        }
    }

}

int modbus_handler::modbus_handler_connect()
{
    int ret, startAddr = 0;
    string sqlcmd = "select param2 from port where protocol=0;";
    m_dbhelper->sql_exec_multicol_return(sqlcmd);
    vector<string> param2vec = m_dbhelper->getsqlresult();
    for (int i = 0; i < m_backendvec.size(); i++)
    {
        if (m_bConnected[i] == false)
        {
            continue;
        }
        modbus_t *ctx = m_backendvec[i]->createCtxt(m_backendvec[i]);
        if (modbus_connect(ctx) == -1)
        {
            fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
            modbus_free(ctx);
            return -1;
        }
        else
        {
            m_bConnected[i] = true;
        }
    }
}


int modbus_handler::modbus_data_process()
{
    int ret, startAddr = 0;
    string sqlcmd = "select param1 from port where protocol=0;";
    m_dbhelper->sql_exec_multicol_return(sqlcmd);
    for (int i = 0; i < m_dbhelper->getsqlresult().size(); i++)
    {
        m_fTypevec.push_back(stoi(m_dbhelper->getsqlresult()[i]));
    }
    string sqlcmd = "select param2 from port where protocol=0;";
    m_dbhelper->sql_exec_multicol_return(sqlcmd);
    vector<string> param2vec = m_dbhelper->getsqlresult();
    // 先考虑一个
    // TODO

    FuncType fType = (FuncType)m_fTypevec[0];
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
        m_datavec[0].data8 = (uint8_t*)malloc(99 * sizeof(uint8_t));
        break;
    case (Data16Array):
        m_datavec[0].data16 = (uint16_t*)malloc(99 * sizeof(uint16_t));
        break;
    default:
        printf("Data alloc error!\n");
        // exit(EXIT_FAILURE);
    }
    modbus_t *ctx = m_backendvec[0]->createCtxt(m_backendvec[0]);
    int readWriteNo = stoi(param2vec[i]);
    while(m_bConnected[0])
    {
        switch (m_fTypevec[0]) {
        case(ReadCoils):
            ret = modbus_read_bits(ctx, startAddr, readWriteNo, m_datavec[0].data8);
            break;
        case(ReadDiscreteInput):
            printf("ReadDiscreteInput: not implemented yet!\n");
            m_wdatetypevec[0] = DataInt;
            break;
        case(ReadHoldingRegisters):
            ret = modbus_read_registers(ctx, startAddr, readWriteNo, m_datavec[0].data16);
            break;
        case(ReadInputRegisters):
            ret = modbus_read_input_registers(ctx, startAddr, readWriteNo, m_datavec[0].data16);
            break;
        case(WriteSingleCoil):
            ret = modbus_write_bit(ctx, startAddr, m_datavec[0].dataInt);
            break;
        case(WriteSingleRegister):
            ret = modbus_write_register(ctx, startAddr, m_datavec[0].dataInt);
            break;
        case(WriteMultipleCoils):
            ret = modbus_write_bits(ctx, startAddr, readWriteNo, m_datavec[0].data8);
            break;
        case(WriteMultipleRegisters):
            ret = modbus_write_registers(ctx, startAddr, readWriteNo, m_datavec[0].data16);
            break;
        default:
            printf("No correct function type chosen");
        }
        sleep(5);
        if (ret < 0)
        {
            m_bConnected[0] = false;
        }
    }

    

}