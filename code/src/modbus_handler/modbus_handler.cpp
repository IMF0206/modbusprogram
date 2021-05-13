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

int modbus_handler::modbus_init()
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
    }
}

int modbus_handler::modbus_data_process()
{
    string sqlcmd = "select param1 from port where protocol=0;";
    m_dbhelper->sql_exec_multicol_return(sqlcmd);
    for (int i = 0; i < m_dbhelper->getsqlresult().size(); i++)
    {
        m_fTypevec.push_back(stoi(m_dbhelper->getsqlresult()[i]));
    }
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

}